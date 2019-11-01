#include "common.h"
#include <xpwn/libxpwn.h>
#include <xpwn/plist.h>
#include <xpwn/outputstate.h>
#include <xpwn/img3.h>
#include <stdio.h>
#include <openssl/asn1.h>
#include <openssl/x509.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "validate_ca.h"

#ifdef WIN32
#include <windows.h>
#endif

char endianness;

extern uint64_t MaxLoadZipSize;
void flipAppleImg3Header(AppleImg3Header* header);

struct tuple_t {
    int len;
    const unsigned char *value;
};

static struct tuple_t *contarray;
static int clen = 0, cmax = 0;
static struct tuple_t apcert;
static struct tuple_t rsasig;
static struct tuple_t theset;

static void
save_tuple(struct tuple_t *dst, const void *src, int len)
{
	dst->len = len;
	dst->value = src;
}

static int
show_cont(int xclass, const unsigned char *p, long len)
{
	if ((xclass & V_ASN1_CONTEXT_SPECIFIC) == V_ASN1_CONTEXT_SPECIFIC) {
		struct tuple_t *tmp;
		if (clen >= cmax) {
			cmax = cmax ? (cmax * 2) : 1;
			tmp = realloc(contarray, cmax * sizeof(struct tuple_t));
			if (!tmp) return -1;
			contarray = tmp;
		}
		tmp = contarray + clen++;
		save_tuple(tmp, p, len);
	}
	return 0;
}

/*
 * This function was lifted from OpenSSL crypto/asn1/asn1_par.c
 * As a consequence, its respective Copyright and Licence applies.
 */

static int
asn1_parse2(const unsigned char **pp, long length, long offset, int depth)
{
	const unsigned char *p, *ep, *tot, *op;
	long len, hl;
	int j, tag, xclass, r, ret = 0;
	p = *pp;
	tot = p + length;
	op = p - 1;
	while (p < tot && op < p) {
		op = p;
		j = ASN1_get_object(&p, &len, &tag, &xclass, length);
		if (j & 0x80) {
			XLOG(0, "Error in encoding\n");
			goto end;
		}
		hl = p - op;
		length -= hl;
		/* if j == 0x21 it is a constructed indefinite length object */

		if (j & V_ASN1_CONSTRUCTED) {
			ep = p + len;
			if (len > length) {
				XLOG(0, "length is greater than %ld\n", length);
				goto end;
			}
			if (j == 0x21 && len == 0) {
				for (;;) {
					r = asn1_parse2(&p, tot - p, offset + (p - *pp), depth + 1);
					if (r == 0) {
						goto end;
					}
					if (r == 2 || p >= tot) {
						break;
					}
				}
			} else {
				if (depth == 1 && !xclass && tag == V_ASN1_SET) save_tuple(&theset, op, hl + len);
				if (depth == 1 && (xclass & V_ASN1_CONTEXT_SPECIFIC) == V_ASN1_CONTEXT_SPECIFIC) save_tuple(&apcert, p, len);
				while (p < ep) {
					r = asn1_parse2(&p, len, offset + (p - *pp), depth + 1);
					if (r == 0) {
						goto end;
					}
				}
			}
		} else if (xclass != 0) {
			if (show_cont(xclass, op + hl, len)) goto end;
			p += len;
		} else {
			/* DECODE HERE */
			if (depth == 1 && tag == V_ASN1_OCTET_STRING) save_tuple(&rsasig, p, len);
			/* DECODE HERE */
			p += len;
			if (tag == V_ASN1_EOC && xclass == 0) {
				ret = 2;	/* End of sequence */
				goto end;
			}
		}
		length -= len;
	}
	ret = 1;
    end:
	if (!ret) {
		free(contarray);
		contarray = NULL;
	}
	*pp = p;
	return ret;
}

struct component_t {
	char *key;
	char *path;
	char *build;
	DataValue *digest;
	DataValue *partial;
	void *blob;
	int required;
};

static struct component_t *
parseManifest(Dictionary *manifest, int *plen)
{
	int i;
	struct component_t *array = NULL;
	int alen = 0, amax = 0;
	ArrayValue *buildIdentities = (ArrayValue *)getValueByKey(manifest, "BuildIdentities");
	if (!buildIdentities) {
		return NULL;
	}
	for (i = 0; i < buildIdentities->size; i++) {
		Dictionary *dict = (Dictionary *)buildIdentities->values[i];
		if (!dict) continue;
		dict = (Dictionary *)getValueByKey(dict, "Manifest");
		if (!dict) continue;
		for (dict = (Dictionary *)dict->values; dict; dict = (Dictionary*) dict->dValue.next) {
			DataValue *digest = (DataValue *)getValueByKey(dict, "Digest");
			Dictionary *info;
			StringValue *str;
			BoolValue *isfw;
			struct component_t *tmp;
			if (!digest) {
				continue;
			}
			if (alen >= amax) {
				amax = amax ? (amax * 2) : 1;
				tmp = realloc(array, amax * sizeof(struct component_t));
				if (!tmp) goto error;
				array = tmp;
			}
			tmp = array + alen++;
			tmp->key = dict->dValue.key;
			tmp->path = NULL;
			tmp->digest = digest;
			tmp->partial = (DataValue *)getValueByKey(dict, "PartialDigest");
			tmp->build = NULL;
			tmp->blob = NULL;
			tmp->required = FALSE;
			info = (Dictionary *)getValueByKey(dict, "Info");
			if (info) {
				str = (StringValue *)getValueByKey(info, "Path");
				if (str) {
					tmp->path = str->value;
				}
				isfw = (BoolValue *)getValueByKey(info, "IsFirmwarePayload");
				tmp->required = (isfw && isfw->value) || !strcmp(tmp->key, "KernelCache");
			}
			str = (StringValue *)getValueByKey(dict, "BuildString");
			if (str) {
				tmp->build = str->value;
			}
		}
	}
	*plen = alen;
	return array;
error:
	free(array);
	*plen = 0;
	return NULL;
}

uint64_t
getECID(const void *data)
{
	unsigned char temp[8];
	memcpy(temp, data, sizeof(temp));
	FLIPENDIANLE(temp);
	return *(uint64_t *)temp;
}

static void
doPartialSHA1(unsigned char md[SHA_DIGEST_LENGTH], const unsigned char *toHashData, int toHashLength, DataValue *partialDigest)
{
	const unsigned *q = (const unsigned *)partialDigest->value;
	unsigned v31 = q[0]; // XXX ASSERT(v31 == ecid->size == 64)?
	unsigned v32 = q[1];
	SHA_CTX hashctx;
	memset(&hashctx, 0, sizeof(hashctx));
	hashctx.h0 = q[2];
	hashctx.h1 = q[3];
	hashctx.h2 = q[4];
	hashctx.h3 = q[5];
	hashctx.h4 = q[6];
	FLIPENDIAN(hashctx.h0);
	FLIPENDIAN(hashctx.h1);
	FLIPENDIAN(hashctx.h2);
	FLIPENDIAN(hashctx.h3);
	FLIPENDIAN(hashctx.h4);
	FLIPENDIANLE(v32);
	hashctx.Nl = 8 * v32 + 64; // XXX could this 64 be actually v31?
	SHA1_Update(&hashctx, toHashData, toHashLength);
	SHA1_Final(md, &hashctx);
}

static int
extract2Certs(const unsigned char *p, long length, X509 **x1, X509 **x2)
{
	const unsigned char *cert1;
	const unsigned char *cert2;

	long len1, len2;
	int j, tag, xclass;

	cert1 = p;
	j = ASN1_get_object(&p, &len1, &tag, &xclass, length);
	if (j != V_ASN1_CONSTRUCTED) {
		return -1;
	}
	p += len1;
	len1 = p - cert1;
	if (len1 >= length) {
		return -1;
	}
	*x1 = d2i_X509(NULL, &cert1, len1);
	if (!*x1) {
		return -1;
	}
	length -= len1;

	cert2 = p;
	j = ASN1_get_object(&p, &len2, &tag, &xclass, length);
	if (j != V_ASN1_CONSTRUCTED) {
		X509_free(*x1);
		return -1;
	}
	p += len2;
	len2 = p - cert2;
	if (len2 > length) {
		X509_free(*x1);
		return -1;
	}
	*x2 = d2i_X509(NULL, &cert2, len2);
	if (!*x2) {
		X509_free(*x1);
		return -1;
	}

	return 0;
}

static int
cryptoMagic(X509 *x0, X509 *x1, X509 *x2,
	    const unsigned char *toHashData, int toHashLength,
	    /*XXX const*/ unsigned char *rsaSigData, int rsaSigLen,
	    DataValue *partialDigest)
{
	int rv = 0;
	EVP_PKEY *pk = X509_get_pubkey(x2);
	if (pk) {
		if (pk->type == EVP_PKEY_RSA) {
			RSA *rsa = EVP_PKEY_get1_RSA(pk);
			if (rsa) {
				X509_STORE *store = X509_STORE_new();
				if (store) {
					X509_STORE_CTX ctx;
					X509_STORE_add_cert(store, x0);
					X509_STORE_add_cert(store, x1);
					if (X509_STORE_CTX_init(&ctx, store, x2, 0) == 1) {
						X509_STORE_CTX_set_flags(&ctx, X509_V_FLAG_IGNORE_CRITICAL);
						if (X509_verify_cert(&ctx) == 1) {
							unsigned char md[SHA_DIGEST_LENGTH];
							if (partialDigest) {
								// XXX we need to flip ECID back before hashing
								flipAppleImg3Header((AppleImg3Header *)toHashData);
								doPartialSHA1(md, toHashData, toHashLength, partialDigest);
							} else {
								SHA1(toHashData, toHashLength, md);
							}
							rv = RSA_verify(NID_sha1, md, SHA_DIGEST_LENGTH, rsaSigData, rsaSigLen, rsa);
						}
						X509_STORE_CTX_cleanup(&ctx);
					}
					X509_STORE_free(store);
				}
				RSA_free(rsa);
			}
		}
		EVP_PKEY_free(pk);
	}
	return rv ? 0 : -1;
}

static const char *
checkBlob(X509 *x0, DataValue *blob, DataValue *partialDigest, uint64_t *savecid)
{
	int rv;
	X509 *x1, *x2;
	uint64_t thisecid;
	unsigned partial0;
	int len = blob->len;
	const unsigned char *ptr = blob->value;

	AppleImg3Header *cert = NULL;
	AppleImg3Header *ecid = NULL;
	AppleImg3Header *shsh = NULL;

	while (len > 0) {
		AppleImg3Header *hdr;
		if (len < sizeof(AppleImg3Header)) {
			return "truncated";
		}
		hdr = (AppleImg3Header *)ptr;
		flipAppleImg3Header(hdr); // XXX we need to flip ECID back before hashing
		switch (hdr->magic) {
			case IMG3_ECID_MAGIC:
				ecid = (AppleImg3Header *)ptr;
				break;
			case IMG3_SHSH_MAGIC:
				shsh = (AppleImg3Header *)ptr;
				break;
			case IMG3_CERT_MAGIC:
				cert = (AppleImg3Header *)ptr;
				break;
			default:
				return "unknown";
		}
		len -= hdr->size;
		ptr += hdr->size;
	}

	if (!ecid || !shsh || !cert) {
		return "incomplete";
	}
	partial0 = *(unsigned *)partialDigest->value;
	FLIPENDIANLE(partial0);
	if (partial0 != 0x40 || partial0 != ecid->size) {
		return "internal"; // XXX see doPartialSHA1()
	}

	thisecid = getECID(ecid + 1);
	if (*savecid == 0) {
		*savecid = thisecid;
	}
	if (*savecid != thisecid) {
		return "mismatch";
	}

	rv = extract2Certs((unsigned char *)(cert + 1), cert->dataSize, &x1, &x2);
	if (rv) {
		return "asn1";
	}

	rv = cryptoMagic(x0, x1, x2, (unsigned char *)ecid, ecid->size, (unsigned char *)(shsh + 1), shsh->dataSize, partialDigest);

	X509_free(x2);
	X509_free(x1);
	return rv ? "crypto" : NULL;
}

#define VERBOSE(format, ...) if (verbose) printf(format, ## __VA_ARGS__)

int main(int argc, char* argv[]) {
	init_libxpwn(&argc, argv);

	OutputState *outputState;
	size_t fileLength;

	AbstractFile *signFile;
	Dictionary *signDict = NULL;
	DataValue *ticket;
	Dictionary *dict;

	AbstractFile *manifestFile;
	Dictionary* manifest;
	struct component_t *array;

	char *plist;
	int i, j, n;
	int rv, error = 0;

	X509 *x0, *y1, *y2;
	uint64_t savecid = 0;
	const unsigned char *p;

	int verbose = FALSE;
	int fromzip = FALSE;

	if (argc < 3) {
		XLOG(0, "usage %s file.shsh BuildManifest.plist [-v] [-z]\n", argv[0]);
		return 0;
	}

	for (i = 3; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) {
			verbose = TRUE;
			continue;
		}

		if (!strcmp(argv[i], "-z")) {
			fromzip = TRUE;
			continue;
		}
	}

	// XXX handle gzip/bplist files
	signFile = createAbstractFileFromFile(fopen(argv[1], "rb"));
	if (!signFile) {
		XLOG(0, "FATAL: cannot open %s\n", argv[1]);
		exit(1);
	}

	fileLength = signFile->getLength(signFile);
	plist = malloc(fileLength);
	signFile->read(signFile, plist, fileLength);
	signFile->close(signFile);
	signDict = createRoot(plist);
	free(plist);

	if (!signDict) {
		XLOG(0, "FATAL: cannot parse %s\n", argv[1]);
		exit(1);
	}

	MaxLoadZipSize = 128 * 1024;
	if (fromzip) {
		outputState = loadZip2(argv[2], TRUE); // XXX ASSERT
		manifestFile = getFileFromOutputState(&outputState, "BuildManifest.plist");
	} else {
		outputState = NULL;
		manifestFile = createAbstractFileFromFile(fopen(argv[2], "rb"));
	}
	if (!manifestFile) {
		XLOG(0, "FATAL: cannot open %s\n", argv[2]);
		exit(1);
	}

	fileLength = manifestFile->getLength(manifestFile);
	plist = malloc(fileLength);
	manifestFile->read(manifestFile, plist, fileLength);
	manifestFile->close(manifestFile);
	manifest = createRoot(plist);
	free(plist);

	if (!manifest) {
		XLOG(0, "FATAL: cannot parse %s\n", argv[2]);
		exit(1);
	}

	releaseOutput(&outputState);

	array = parseManifest(manifest, &n);
	if (!array) {
		XLOG(0, "FATAL: cannot parse manifest\n");
		exit(1);
	}

	OPENSSL_add_all_algorithms_noconf();
	p = cerb;
	x0 = d2i_X509(NULL, &p, cerb_len);
	if (!x0) {
		XLOG(0, "FATAL: cannot load root CA\n");
		exit(1);
	}

	ticket = (DataValue *)getValueByKey(signDict, "APTicket");
	if (ticket) {
		p = ticket->value;
		rv = asn1_parse2(&p, ticket->len, 0, 0);
		if (!rv || !apcert.value || !rsasig.value || !theset.value) {
			XLOG(0, "FATAL: cannot parse ticket\n");
			exit(1);
		}
		if (clen > 0 && contarray->len == 8) {
			savecid = getECID(contarray->value);
		}
		if (!savecid) {
			printf("ERROR: bad, bad ECID\n");
			error = 1;
		}

		rv = extract2Certs(apcert.value, apcert.len, &y1, &y2);
		if (rv == 0) {
			rv = cryptoMagic(x0, y1, y2, theset.value, theset.len, (unsigned char *)rsasig.value, rsasig.len, NULL);
			X509_free(y1);
			X509_free(y2);
		}
		if (rv) {
			printf("ERROR: APTicket failed crypto\n");
			error = 1;
		}
	} else {
		VERBOSE("WARNING: cannot find ticket in %s\n", argv[1]);
	}

	dict = (Dictionary *)signDict->values;
	while (dict) {
		DataValue *blob = NULL;
		DataValue *partialDigest = NULL;
		if (dict->dValue.type == DictionaryType) {
			blob = (DataValue *)getValueByKey(dict, "Blob");
			partialDigest = (DataValue *)getValueByKey(dict, "PartialDigest");
		}
		if (blob && partialDigest) {
			const char *diag = checkBlob(x0, blob, partialDigest, &savecid);
			if (diag) {
				printf("ERROR: Blob for %s is invalid (%s)\n", dict->dValue.key, diag);
				error = 1;
			} else {
				for (i = 0; i < n; i++) {
					struct component_t *centry = array + i;
					if (centry->partial &&
					    partialDigest->len == centry->partial->len &&
					    !memcmp(partialDigest->value, centry->partial->value, partialDigest->len)) {
						array[i].blob = blob;
					}
				}
			}
		}
		dict = (Dictionary *)dict->dValue.next;
	}

	if (!ticket && !savecid) {
		printf("ERROR: bad, bad ECID\n");
		error = 1;
	}

	for (i = 0; i < n; i++) {
		struct component_t *centry = array + i;
		int found = FALSE;
		for (j = 0; j < clen; j++) {
			struct tuple_t *tentry = contarray + j;
			if (tentry->len == centry->digest->len && !memcmp(tentry->value, centry->digest->value, tentry->len)) {
				found = TRUE;
			}
		}
		if (!found) {
			if (centry->blob) {
				VERBOSE("WARNING: no digest for %s (%s), but it has blob\n", centry->key, centry->path);
			} else if (!centry->required) {
				VERBOSE("WARNING: no digest for %s (%s), but it is not critical\n", centry->key, centry->path);
			} else {
				printf("ERROR: no digest for %s (%s) and no blob found\n", centry->key, centry->path);
				error = 1;
			}
		} else {
			VERBOSE("INFO: %s (%s) is signed by APTicket%s\n", centry->key, centry->path, centry->blob ? " and blob" : "");
		}
	}
	free(array);
	free(contarray);

	releaseDictionary(manifest);
	releaseDictionary(signDict);

	if (error) {
		printf("%s is BROKEN\n", argv[1]);
	} else {
		printf("%s seems usable for ECID 0x%016llX\n", argv[1], savecid);
	}

	X509_free(x0);

	EVP_cleanup();
	ERR_remove_state(0);
	CRYPTO_cleanup_all_ex_data();
	return error;
}
