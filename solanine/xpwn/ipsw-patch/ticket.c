#include "common.h"
#include <xpwn/libxpwn.h>
#include <xpwn/plist.h>
#include <xpwn/outputstate.h>
#include <xpwn/img3.h>
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#endif

char endianness;

extern uint64_t MaxLoadZipSize;
void flipAppleImg3Header(AppleImg3Header* header);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
const char *componentName(unsigned int magic) {
	switch (magic) {
	case 'illb': return "LLB";
	case 'krnl': return "KernelCache";
	case 'logo': return "AppleLogo";
	case 'dtre': return "DeviceTree";
	case 'ibot': return "iBoot";
	case 'recm': return "RecoveryMode";
	case 'bat0': return "BatteryLow0";
	case 'bat1': return "BatteryLow1";
	case 'batF': return "BatteryFull";
	case 'chg0': return "BatteryCharging0";
	case 'chg1': return "BatteryCharging1";
	case 'glyC': return "BatteryCharging";
	case 'glyP': return "BatteryPlugin";
	default:
		XLOG(0, "unknown magic %08x\n", magic);
		exit(1);
	}
}

static DataValue *
findPartialDigest(Dictionary *manifest, const char *component) {
	int i;
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
			if (!strcmp(dict->dValue.key, component)) {
				DataValue *partial = (DataValue *)getValueByKey(dict, "PartialDigest");
				if (partial) {
					return partial;
				}
			}
		}
	}
	return NULL;
}

DataValue *makeData(const void *data, int size) {
	DataValue *val = malloc(sizeof(DataValue));
	val->dValue.type = DataType;
	val->len = size;
	val->value = malloc(size);
	memcpy(val->value, data, size);
	return val;
}

DataValue *makeBlob(AppleImg3Header *ecid, AppleImg3Header *cert, const void *data, int size) {
	DataValue *val;
	AppleImg3Header shsh;
	int ecid_size = ecid->size;
	int cert_size = cert->size;
	int total = ecid_size + sizeof(AppleImg3Header) + size + cert_size;
	char *ptr = malloc(total);

	shsh.magic = 'SHSH';
	shsh.size = size + 12;
	shsh.dataSize = size;
	flipAppleImg3Header(&shsh);
	flipAppleImg3Header(ecid);
	flipAppleImg3Header(cert);

	memcpy(ptr, ecid, ecid_size);
	memcpy(ptr + ecid_size, &shsh, sizeof(AppleImg3Header));
	memcpy(ptr + ecid_size + sizeof(AppleImg3Header), data, size);
	memcpy(ptr + ecid_size + sizeof(AppleImg3Header) + size, cert, cert_size);

	flipAppleImg3Header(ecid);
	flipAppleImg3Header(cert);

	val = malloc(sizeof(DataValue));
	val->dValue.type = DataType;
	val->len = total;
	val->value = malloc(total);
	memcpy(val->value, ptr, total);
	return val;
}

int main(int argc, char* argv[]) {
	init_libxpwn(&argc, argv);

	OutputState *outputState;
	size_t fileLength;

	AbstractFile *dumpFile;
	char *dumpData;
	Dictionary *signDict;
	DataValue *llb = NULL;
	AppleImg3Header *cert = NULL;
	AppleImg3Header *ecid = NULL;
	uint64_t thisecid = 0;

	AbstractFile *manifestFile;
	Dictionary* manifest;

	char *plist;
	int i;

	int fromzip = FALSE;

	if (argc < 4) {
		XLOG(0, "usage %s file.dump file.shsh BuildManifest.plist [-z]\n", argv[0]);
		return 0;
	}

	for (i = 4; i < argc; i++) {
		if (!strcmp(argv[i], "-z")) {
			fromzip = TRUE;
			continue;
		}
	}

	MaxLoadZipSize = 128 * 1024;
	if (fromzip) {
		outputState = loadZip2(argv[3], TRUE); // XXX ASSERT
		manifestFile = getFileFromOutputState(&outputState, "BuildManifest.plist");
	} else {
		outputState = NULL;
		manifestFile = createAbstractFileFromFile(fopen(argv[3], "rb"));
	}
	if (!manifestFile) {
		XLOG(0, "FATAL: cannot open %s\n", argv[3]);
		exit(1);
	}

	fileLength = manifestFile->getLength(manifestFile);
	plist = malloc(fileLength);
	manifestFile->read(manifestFile, plist, fileLength);
	manifestFile->close(manifestFile);
	manifest = createRoot(plist);
	free(plist);

	if (!manifest) {
		XLOG(0, "FATAL: cannot parse %s\n", argv[3]);
		exit(1);
	}

	releaseOutput(&outputState);

	dumpFile = createAbstractFileFromFile(fopen(argv[1], "rb"));
	if (!dumpFile) {
		XLOG(0, "FATAL: cannot open %s\n", argv[1]);
		exit(1);
	}

	fileLength = dumpFile->getLength(dumpFile);
	dumpData = malloc(fileLength);
	dumpFile->read(dumpFile, dumpData, fileLength);
	dumpFile->close(dumpFile);

	signDict = createRoot("<dict></dict>");

	for (i = 0; i < fileLength - 8; ) {
		unsigned magic = *(unsigned *)(dumpData + i);
		unsigned size = *(unsigned *)(dumpData + i + 4);
		if (magic == 'illb') {
			int len = size;
			const char *ptr = dumpData + i + 8;
			llb = makeData(dumpData + i + 8, size);
			while (len > 0) {
				AppleImg3Header *hdr;
				if (len < sizeof(AppleImg3Header)) {
					break;
				}
				hdr = (AppleImg3Header *)ptr;
				flipAppleImg3Header(hdr);
				switch (hdr->magic) {
					case IMG3_ECID_MAGIC:
						ecid = (AppleImg3Header *)ptr;
						break;
					case IMG3_CERT_MAGIC:
						cert = (AppleImg3Header *)ptr;
						break;
				}
				len -= hdr->size;
				ptr += hdr->size;
			}
			break;
		}
		i += 8 + size;
	}
	if (!llb || !ecid || !cert) {
		XLOG(0, "FATAL: LLB blob corrupted or not found\n");
		exit(1);
	}
	for (i = 0; i < fileLength - 8; ) {
		unsigned magic = *(unsigned *)(dumpData + i);
		unsigned size = *(unsigned *)(dumpData + i + 4);
		if (magic == 'SCAB') {
			DataValue *ticket = makeData(dumpData + i + 8, size);
			addValueToDictionary(signDict, "APTicket", (DictValue *)ticket);
		} else {
			const char *component = componentName(magic);
			DataValue *p = findPartialDigest(manifest, component);
			if (p) {
				Dictionary *dict = createRoot("<dict></dict>");
				DataValue *partialDigest = makeData(p->value, p->len);
				DataValue *blob = llb;
				if (magic != 'illb') {
					blob = makeBlob(ecid, cert, dumpData + i + 8, size);
				}
				addValueToDictionary(dict, "Blob", (DictValue *)blob);
				addValueToDictionary(dict, "PartialDigest", (DictValue *)partialDigest);
				free(dict->dValue.key); // hack
				addValueToDictionary(signDict, component, (DictValue *)dict);
			}
		}
		i += 8 + size;
	}

	plist = getXmlFromRoot(signDict);
	releaseDictionary(signDict);

	dumpFile = createAbstractFileFromFile(fopen(argv[2], "w"));
	dumpFile->write(dumpFile, plist, strlen(plist));
	dumpFile->close(dumpFile);
	free(plist);

	memcpy(&thisecid, ecid + 1, 8);
	FLIPENDIANLE(thisecid);
	printf("saved blob for ECID=%lld\n", thisecid);

	free(dumpData);
	releaseDictionary(manifest);
	return 0;
}
#pragma GCC diagnostic pop
