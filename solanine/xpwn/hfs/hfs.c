#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <hfs/hfsplus.h>
#include <dmg/dmgfile.h>
#include <dmg/filevault.h>
#include <dirent.h>

#include <hfs/hfslib.h>
#include "abstractfile.h"
#include <inttypes.h>

char endianness;


void cmd_ls(Volume* volume, int argc, const char *argv[]) {
	int flags = 0;
	const char *path = "/";
	while (argc > 1) {
		const char *p = argv[1];
		if (*p == '-') {
			int fl = 0;
			while (*++p) switch (*p) {
				case 'l': fl |= 1; break;
				case 'R': fl |= 2; break;
				default: fl = -1;
			}
			if (fl > 0) {
				argc--;
				argv++;
				flags |= fl;
				continue;
			}
		}
		path = argv[1];
		break;
	}
	if (flags)
		hfs_list(volume, path, flags & 2);
	else 
		hfs_ls(volume, path);
}

void cmd_cat(Volume* volume, int argc, const char *argv[]) {
	HFSPlusCatalogRecord* record;
	AbstractFile* stdoutFile;

	record = getRecordFromPath(argv[1], volume, NULL, NULL);

	stdoutFile = createAbstractFileFromFile(stdout);
	
	if(record != NULL) {
		if(record->recordType == kHFSPlusFileRecord)
			writeToFile((HFSPlusCatalogFile*)record, stdoutFile, volume);
		else
			printf("Not a file\n");
	} else {
		printf("No such file or directory\n");
	}
	
	free(record);
	free(stdoutFile);
}

void cmd_extract(Volume* volume, int argc, const char *argv[]) {
	HFSPlusCatalogRecord* record;
	AbstractFile *outFile;
	const char *outName;
	
	if(argc < 3) {
		if (argc == 2) {
			char *p = strrchr(argv[1], '/');
			outName = p ? ++p : argv[1];
			goto do_extract;
		}
		printf("Not enough arguments");
		return;
	}
	outName = argv[2];
do_extract:
	
	outFile = createAbstractFileFromFile(fopen(outName, "wb"));
	
	if(outFile == NULL) {
		printf("cannot create file");
	}
	
	record = getRecordFromPath(argv[1], volume, NULL, NULL);
	
	if(record != NULL) {
		if(record->recordType == kHFSPlusFileRecord)
			writeToFile((HFSPlusCatalogFile*)record, outFile, volume);
		else
			printf("Not a file\n");
	} else {
		printf("No such file or directory\n");
	}
	
	outFile->close(outFile);
	free(record);
}

void cmd_mv(Volume* volume, int argc, const char *argv[]) {
	if(argc > 2) {
		move(argv[1], argv[2], volume);
	} else {
		printf("Not enough arguments");
	}
}

void cmd_symlink(Volume* volume, int argc, const char *argv[]) {
	if(argc > 2) {
		makeSymlink(argv[1], argv[2], volume);
	} else {
		printf("Not enough arguments");
	}
}

void cmd_mkdir(Volume* volume, int argc, const char *argv[]) {
	if(argc > 1) {
		newFolder(argv[1], volume);
	} else {
		printf("Not enough arguments");
	}
}

void cmd_add(Volume* volume, int argc, const char *argv[]) {
	AbstractFile *inFile;
	
	if(argc < 3) {
		printf("Not enough arguments");
		return;
	}
	
	inFile = createAbstractFileFromFile(fopen(argv[1], "rb"));
	
	if(inFile == NULL) {
		printf("file to add not found");
	}

	add_hfs(volume, inFile, argv[2]);
}

void cmd_rm(Volume* volume, int argc, const char *argv[]) {
	if(argc > 1) {
		removeFile(argv[1], volume);
	} else {
		printf("Not enough arguments");
	}
}

void cmd_chmod(Volume* volume, int argc, const char *argv[]) {
	int mode;
	
	if(argc > 2) {
		sscanf(argv[1], "%o", &mode);
		chmodFile(argv[2], mode, volume);
	} else {
		printf("Not enough arguments");
	}
}

void cmd_chown(Volume* volume, int argc, const char *argv[]) {
	uint32_t uid, gid;
	
	if(argc > 2 && sscanf(argv[1], "%u:%u", &uid, &gid) == 2) {
		chownFile(argv[2], uid, gid, volume);
	} else {
		printf("usage: chown uid:gid path\n");
	}
}

void cmd_extractall(Volume* volume, int argc, const char *argv[]) {
	HFSPlusCatalogRecord* record;
	char cwd[1024];
	char* name;
	
	ASSERT(getcwd(cwd, 1024) != NULL, "cannot get current working directory");
	
	if(argc > 1)
		record = getRecordFromPath(argv[1], volume, &name, NULL);
	else
		record = getRecordFromPath("/", volume, &name, NULL);
	
	if(argc > 2) {
		ASSERT(chdir(argv[2]) == 0, "chdir");
	}

	if(record != NULL) {
		if(record->recordType == kHFSPlusFolderRecord)
			extractAllInFolder(((HFSPlusCatalogFolder*)record)->folderID, volume);  
		else
			printf("Not a folder\n");
	} else {
		printf("No such file or directory\n");
	}
	free(record);
	
	ASSERT(chdir(cwd) == 0, "chdir");
}


void cmd_rmall(Volume* volume, int argc, const char *argv[]) {
	HFSPlusCatalogRecord* record;
	char* name;
	char initPath[1024];
	int lastCharOfPath;
	
	if(argc > 1) {
		record = getRecordFromPath(argv[1], volume, &name, NULL);
		strcpy(initPath, argv[1]);
		lastCharOfPath = strlen(argv[1]) - 1;
		if(argv[1][lastCharOfPath] != '/') {
			initPath[lastCharOfPath + 1] = '/';
			initPath[lastCharOfPath + 2] = '\0';
		}
	} else {
		record = getRecordFromPath("/", volume, &name, NULL);
		initPath[0] = '/';
		initPath[1] = '\0';	
	}
	
	if(record != NULL) {
		if(record->recordType == kHFSPlusFolderRecord) {
			removeAllInFolder(((HFSPlusCatalogFolder*)record)->folderID, volume, initPath);
		} else {
			printf("Not a folder\n");
		}
	} else {
		printf("No such file or directory\n");
	}
	free(record);
}

void cmd_addall(Volume* volume, int argc, const char *argv[]) {   
	if(argc < 2) {
		printf("Not enough arguments");
		return;
	}

	if(argc > 2) {
		addall_hfs(volume, argv[1], argv[2]);
	} else {
		addall_hfs(volume, argv[1], "/");
	}
}

void cmd_grow(Volume* volume, int argc, const char *argv[]) {
	uint64_t newSize;

	if(argc < 2) {
		printf("Not enough arguments\n");
		return;
	}
	
	newSize = 0;
	sscanf(argv[1], "%" PRId64, &newSize);

	grow_hfs(volume, newSize);

	printf("grew volume: %" PRId64 "\n", newSize);
}

void cmd_untar(Volume* volume, int argc, const char *argv[]) {
	AbstractFile *inFile;
	
	if(argc < 2) {
		printf("Not enough arguments");
		return;
	}
	
	inFile = createAbstractFileFromFile(fopen(argv[1], "rb"));
	
	if(inFile == NULL) {
		printf("file to untar not found");
	}

	hfs_untar(volume, inFile);
}

void cmd_getattr(Volume* volume, int argc, const char *argv[]) {
	HFSPlusCatalogRecord* record;

	if(argc < 3) {
		printf("Not enough arguments");
		return;
	}

	record = getRecordFromPath(argv[1], volume, NULL, NULL);

	if(record != NULL) {
		HFSCatalogNodeID id;
		uint8_t* data;
		size_t size;
		if(record->recordType == kHFSPlusFileRecord)
			id = ((HFSPlusCatalogFile*)record)->fileID;
		else
			id = ((HFSPlusCatalogFolder*)record)->folderID;

		size = getAttribute(volume, id, argv[2], &data);

		if(size > 0) {
			fwrite(data, size, 1, stdout);
			free(data);
		} else {
			printf("No such attribute\n");
		}
	} else {
		printf("No such file or directory\n");
	}
	
	free(record);
}

void TestByteOrder()
{
	short int word = 0x0001;
	char *byte = (char *) &word;
	endianness = byte[0] ? IS_LITTLE_ENDIAN : IS_BIG_ENDIAN;
}


int main(int argc, const char *argv[]) {
	io_func* io;
	Volume* volume;
	AbstractFile* image;
	
	TestByteOrder();
	
	if(argc < 3) {
		printf("usage: %s <image-file> (-k <key>) <ls|cat|mv|symlink|mkdir|add|rm|chmod|chown|extract|extractall|rmall|addall|grow|untar|getattr|debug> <arguments>\n", argv[0]);
		return 0;
	}
	
	image = createAbstractFileFromFile(fopen(argv[1], "r+b"));
	if(argc > 3 && strcmp(argv[2], "-k") == 0) {
		AbstractFile *tmp = createAbstractFileFromFileVault(image, argv[3]);
		if (tmp) {
			image = tmp;
		}
		argc -= 2;
		argv += 2;
	}
	io = openDmgFilePartition(image, -1);
	if(io == NULL) {
		fprintf(stderr, "error: Cannot open image-file.\n");
		return 1;
	}
	
	volume = openVolume(io); 
	if(volume == NULL) {
		fprintf(stderr, "error: Cannot open volume.\n");
		CLOSE(io);
		return 1;
	}
	
	if(argc > 1) {
		if(strcmp(argv[2], "ls") == 0) {
			cmd_ls(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "cat") == 0) {
			cmd_cat(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "mv") == 0) {
			cmd_mv(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "symlink") == 0) {
			cmd_symlink(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "mkdir") == 0) {
			cmd_mkdir(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "add") == 0) {
			cmd_add(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "rm") == 0) {
			cmd_rm(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "chmod") == 0) {
			cmd_chmod(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "chown") == 0) {
			cmd_chown(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "extract") == 0) {
			cmd_extract(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "extractall") == 0) {
			cmd_extractall(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "rmall") == 0) {
			cmd_rmall(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "addall") == 0) {
			cmd_addall(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "grow") == 0) {
			cmd_grow(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "untar") == 0) {
			cmd_untar(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "getattr") == 0) {
			cmd_getattr(volume, argc - 2, argv + 2);
		} else if(strcmp(argv[2], "debug") == 0) {
			if(argc > 3 && strcmp(argv[3], "verbose") == 0) {
				debugBTree(volume->catalogTree, TRUE);
			} else {
				debugBTree(volume->catalogTree, FALSE);
			}
		}
	}
	
	closeVolume(volume);
	CLOSE(io);
	
	return 0;
}
