// by elliot kroo (http://elliot.kroo.net/)
// thanks!

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define UNTAR_BUFFER_SIZE 512
#define UNTAR_DEBUG 0

static long long int untar_numBytes = 0;

typedef struct struct_FileHeader {
  char filename[100];
  char mode[8];
  char owner[8];
  char group[8];
  char filesize[12];
  char lastmodified[12];
  char checksum[8];
  char linkindicator;
  char linkedfile[100];
  char padding[255];
} FileHeader;

int isNull(FileHeader *header) {
  long long int *iheader = (long long int *)header;
  int i;
  for(i=0; i<sizeof(FileHeader) / sizeof(long long int); i++) {
    if(iheader[i] != 0) {
      return 0;
    }
  }
  return 1;
}

void untar_create_link_(char * fullpath, char * linkpath, int filesize, FILE* fp){
  int result = symlink(linkpath, fullpath);
  if(result != 0) {
    fprintf(stderr, "Could not create symlink %s (%d)\n", fullpath, errno);
		perror("having trouble creating symlink");
    return;
  }
}


void untar_create_directory_(char * fullpath, mode_t mode, int filesize, FILE* fp){
  int result = mkdir(fullpath, mode);
  if(result != 0) {
    if(errno != EEXIST)
      fprintf(stderr, "Could not create directory %s (%d)\n", fullpath, result);
		perror("having trouble creating directory");
    return;
  }
}

void untar_create_file_(char * fullpath, mode_t mode, int filesize, FILE*fp){
  FILE *out = fopen(fullpath, "wb+");
  if(!out) {
    fprintf(stderr, "Could not create file %s\n", fullpath);
		perror("having trouble creating file");
  }

  char buffer[UNTAR_BUFFER_SIZE]; // read until we run out of filled blocks
  while (filesize / UNTAR_BUFFER_SIZE != 0) {
    fread(buffer, 1, UNTAR_BUFFER_SIZE, fp);
		untar_numBytes += UNTAR_BUFFER_SIZE;
		
		if(out)
			fwrite(buffer, 1, UNTAR_BUFFER_SIZE, out);
    filesize -= UNTAR_BUFFER_SIZE;
  }
	
	if(filesize > 0) {
		// now read in and write the last block...
		fread(buffer, 1, UNTAR_BUFFER_SIZE, fp);
		untar_numBytes += UNTAR_BUFFER_SIZE;
		
		if(out)
			fwrite(buffer, 1, filesize, out);
	}
	
  if(out) fclose(out);

  chmod(fullpath, mode);
}

void untar(char *filename, char *basepath) {
  FILE *fp = fopen(filename, "r");
  
  int consecutive_zero_fields = 0;
	untar_numBytes = 0;
	
  if(fp) {
    for(;;) {
      FileHeader header;
      fread(&header, 1, sizeof(FileHeader), fp);
			untar_numBytes += sizeof(FileHeader);
      if(feof(fp))  /* if at end of file */
        break;      /* we are done       */
      
      if(isNull(&header)) {
        consecutive_zero_fields++;
        continue;
      }

      if(UNTAR_DEBUG)
        printf("type: %c\t filename: %s\tmode: %s \towner: %s\tgroup: %s\tfilesize: %s\tlastmodified: %s\tchecksum: %s\tlinkedfile: %s\n", 
                header.linkindicator, header.filename, header.mode, header.owner, header.group, header.filesize, header.lastmodified,
                header.checksum, header.linkedfile);

      // build the full path
      char fullpath[strlen(basepath)+strlen(header.filename)+1];
      strcpy(fullpath, basepath);
      strcpy(fullpath+strlen(basepath), header.filename);

			// build the full linkpath
			char linkpath[strlen(basepath)+strlen(header.linkedfile)+1];
			strcpy(linkpath, basepath);
			strcpy(linkpath+strlen(basepath), header.linkedfile);			
			
      mode_t mode = strtoul(header.mode, 0, 8);

      long int filesize = strtoul(header.filesize, 0, 8);

      switch(header.linkindicator) {
        case '0':
          untar_create_file_(fullpath, mode, filesize, fp);
          break;
				case '2':					
					untar_create_link_(fullpath, header.linkedfile, filesize, fp);
					break;
        case '5':
          untar_create_directory_(fullpath, mode, filesize, fp);
          break;
        default:
          fprintf(stderr, "Unsupported file type %c; skipping.\n", header.linkindicator);
      }


    }
  }
	
	fprintf(stderr, "finished!\n");
}
