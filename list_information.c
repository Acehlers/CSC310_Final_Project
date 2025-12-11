/**
 ** Team 12: Ella Berry, Anne McCullagh, Andrew Ehlers
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "qfs.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <disk image file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 2;
    }
    //Read the superblock first
    superblock_t sb;
    fread(&sb, sizeof(sb), 1, fp);

    if (sb.fs_type != 0x51) {
        fprintf(stderr, "Not a QFS filesystem\n");
        fclose(fp);
        return 3;
    }
	//print file system
    printf("Block size: %u\n", sb.bytes_per_block);
    printf("Total blocks: %u\n", sb.total_blocks);
    printf("Free blocks: %u\n", sb.available_blocks);
    printf("Total dir entries: %u\n", sb.total_direntries);
    printf("Free dir entries: %u\n", sb.available_direntries);

    direntry_t de;
    for (int i = 0; i < sb.total_direntries; i++) {
        fread(&de, sizeof(de), 1, fp);
        if (de.filename[0] != '\0') {
            printf("%s\t%u\t%u\n",
                   de.filename,
                   de.file_size,
                   de.starting_block);
        }
    }

    fclose(fp);
    return 0;
}
