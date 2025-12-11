/**
 ** Team 12: Ella Berry, Anne McCullagh, Andrew Ehlers
*/


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "qfs.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <disk image file> <file to read> <output file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 2;
    }

    superblock_t sb;
    fread(&sb, sizeof(sb), 1, fp);

    direntry_t de;
    int found = 0;

    for (int i = 0; i < sb.total_direntries; i++) {
        fread(&de, sizeof(de), 1, fp);
        if (strcmp(de.filename, argv[2]) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("File not found\n");
        fclose(fp);
        return 0;
    }

    FILE *out = fopen(argv[3], "wb");
    if (!out) {
        perror("fopen");
        fclose(fp);
        return 3;
    }

    uint32_t remaining = de.file_size;
    uint16_t block = de.starting_block;
    int data_size = sb.bytes_per_block - 3;

    uint8_t busy;
    uint16_t next;
    uint8_t *buffer = malloc(data_size);

    long data_start = sizeof(superblock_t) + sizeof(direntry_t) * 255;

    while (remaining > 0) {
        fseek(fp, data_start + block * sb.bytes_per_block, SEEK_SET);
        fread(&busy, 1, 1, fp);
        fread(buffer, 1, data_size, fp);
        fread(&next, sizeof(uint16_t), 1, fp);

        int to_write = remaining < data_size ? remaining : data_size;
        fwrite(buffer, 1, to_write, out);

        remaining -= to_write;
        block = next;
    }

    free(buffer);
    fclose(out);
    fclose(fp);
    return 0;
}
