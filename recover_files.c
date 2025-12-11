/**
 ** Team 12: Ella Berry, Anne McCullagh, Andrew Ehlers
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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

    superblock_t sb;
    fread(&sb, sizeof(sb), 1, fp);

    int data_size = sb.bytes_per_block - 3;
    uint8_t *buffer = malloc(data_size);
    uint8_t *used = calloc(sb.total_blocks, 1);

    long data_start = sizeof(superblock_t) + sizeof(direntry_t) * 255;
    int recovered = 0;

    for (uint16_t b = 0; b < sb.total_blocks; b++) {
        if (used[b]) continue;

        uint8_t busy;
        uint16_t next;

        fseek(fp, data_start + b * sb.bytes_per_block, SEEK_SET);
        fread(&busy, 1, 1, fp);
        fread(buffer, 1, data_size, fp);
        fread(&next, sizeof(uint16_t), 1, fp);

        if (buffer[0] != 0xFF || buffer[1] != 0xD8) continue;

        char name[64];
        sprintf(name, "recovered_file_%d.jpg", recovered++);
        FILE *out = fopen(name, "wb");

        uint16_t cur = b;
        while (cur < sb.total_blocks && !used[cur]) {
            used[cur] = 1;

            fseek(fp, data_start + cur * sb.bytes_per_block, SEEK_SET);
            fread(&busy, 1, 1, fp);
            fread(buffer, 1, data_size, fp);
            fread(&next, sizeof(uint16_t), 1, fp);

            fwrite(buffer, 1, data_size, out);

            for (int i = 0; i < data_size - 1; i++) {
                if (buffer[i] == 0xFF && buffer[i+1] == 0xD9) {
                    fclose(out);
                    goto done;
                }
            }

            cur = next;
        }

        fclose(out);
done:;
    }

    free(buffer);
    free(used);
    fclose(fp);
    return 0;
}
