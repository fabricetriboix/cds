/* Copyright (c) 2016  Fabrice Triboix
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "cdsmap.h"


// A key is a string of 16 characters, add terminating null char and ref counter
#define KEYSIZE_B 18

typedef struct
{
    CdsMapItem item;
    int ref;
    long long value;
} MyItem;

static void addItem(CdsMap* map, long long value)
{
    MyItem* item = CdsMallocZ(sizeof(*item));
    item->ref = 1;
    item->value = value;

    // NB: The last character is used as a reference counter
    char* key = CdsMallocZ(KEYSIZE_B);
    snprintf(key, KEYSIZE_B - 1, "%016lx", (unsigned long)value);
    key[KEYSIZE_B - 1] = 1;

    CDSASSERT(CdsMapInsert(map, key, (CdsMapItem*)item));
}

static void keyUnref(void* lkey)
{
    char* key = (char*)lkey;
    // NB: The last character is used as a reference counter
    key[KEYSIZE_B - 1]--;
    if (key[KEYSIZE_B - 1] <= 0) {
        free(key);
    }
}

static void myItemUnref(CdsMapItem* litem)
{
    MyItem* item = (MyItem*)litem;
    item->ref--;
    if (item->ref <= 0) {
        free(item);
    }
}

static int keyCmp(void* leftKey, void* rightKey, void* cookie)
{
    (void)cookie;
    return strcmp((const char*)leftKey, (const char*)rightKey);
}


int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: ./cdsmapperf COUNT FILE\n");
        exit(2);
    }
    long long count;
    if (sscanf(argv[1], "%lld", &count) != 1) {
        fprintf(stderr, "Invalid COUNT argument: '%s'\n", argv[1]);
        exit(2);
    }
    if (count <= 0) {
        fprintf(stderr, "Invalid COUNT: %lld\n", count);
        exit(2);
    }

    int fd = open(argv[2], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open file '%s'\n", argv[2]);
        exit(1);
    }
    long long size_B = count * sizeof(unsigned long);
    unsigned long* numbers = malloc(size_B);
    if (numbers == NULL) {
        fprintf(stderr, "Failed to allocate %lld bytes\n", size_B);
        exit(1);
    }
    unsigned long* ptr = numbers;
    long long remaining_B = size_B;
    while (remaining_B > 0) {
        ssize_t n = read(fd, ptr, remaining_B);
        if (n < 0) {
            fprintf(stderr, "Failed to read file '%s': %s\n",
                    argv[2], strerror(errno));
            exit(1);
        }
        if (n == 0) {
            fprintf(stderr, "ERROR: Zero read from file '%s'\n", argv[2]);
            exit(1);
        }
        ptr += n;
        remaining_B -= n;
    }
    close(fd);

    CdsMap* map = CdsMapCreate(NULL, 0, keyCmp, NULL, keyUnref, myItemUnref);

    printf("Inserting %lld items\n", count);
    for (long long i = 0; i < count; i++) {
        addItem(map, numbers[i]);
    }

    printf("Removing %lld items\n", count);
    for (long long i = count - 1; i >= 0; i--) {
        char key[KEYSIZE_B];
        snprintf(key, sizeof(key), "%016lx", numbers[i]);
        CDSASSERT(CdsMapRemove(map, key));
    }

    CDSASSERT(CdsMapSize(map) == 0);
    CdsMapDestroy(map);
    return 0;
}
