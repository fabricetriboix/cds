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

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
}

#include <memory>
#include <map>
#include <string>


class MyItem
{
public :
    MyItem(long long v)
    {
        value = v;
    }

    long long value;
};


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
    unsigned long* numbers = (unsigned long*)malloc(size_B);
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

    std::map<std::string, std::shared_ptr<MyItem>> map;

    printf("Inserting %lld items\n", count);
    for (long long i = 0; i < count; i++) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%016lx", numbers[i]);
        map[tmp] = std::make_shared<MyItem>(numbers[i]);
    }

    printf("Removing %lld items\n", count);
    for (long long i = count - 1; i >= 0; i--) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "%016lx", numbers[i]);
        map.erase(tmp);
    }

    if (map.size() != 0) {
        fprintf(stderr, "ERROR: map size should be 0 after all items are "
                "removed (it is currently %lld)\n", (long long)map.size());
        exit(1);
    }
    return 0;
}
