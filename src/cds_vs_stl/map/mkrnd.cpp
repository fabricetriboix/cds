#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <map>

int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: mkrnd COUNT FILE\n");
        exit(2);
    }
    long long count;
    if (sscanf(argv[1], "%lld", &count) != 1) {
        fprintf(stderr, "Invalid COUNT: '%s'\n", argv[1]);
        exit(2);
    }
    if (count <= 0) {
        fprintf(stderr, "Invalid COUNT: %lld\n", count);
        exit(2);
    }

    long long size_B = count * sizeof(unsigned long);
    unsigned long* numbers = (unsigned long*)malloc(size_B);
    if (numbers == NULL) {
        fprintf(stderr, "Failed to allocate %lld bytes\n", size_B);
        exit(1);
    }
    std::map<unsigned long, unsigned long> nmap;

    srandom(time(NULL));
    for (long long i = 0; i < count; i++) {
        for (;;) {
            unsigned long tmp = (unsigned long)random();
            if (nmap.find(tmp) == nmap.end()) {
                nmap[tmp] = tmp;
                numbers[i] = tmp;
                break;
            }
        }
    }

    int fd = creat(argv[2], 0666);
    if (fd < 0) {
        fprintf(stderr, "Failed to create file '%s': %s\n",
                argv[2], strerror(errno));
        exit(1);
    }

    unsigned long* ptr = numbers;
    long long remaining = size_B;
    while (remaining > 0) {
        ssize_t n = write(fd, ptr, remaining);
        if (n < 0) {
            fprintf(stderr, "Failed to write to file '%s': %s\n",
                    argv[2], strerror(errno));
            exit(1);
        }
        if (n == 0) {
            fprintf(stderr, "ERROR: Zero write to file '%s'\n", argv[2]);
            exit(1);
        }
        remaining -= n;
        ptr += n;
    }

    close(fd);
    free(numbers);
    return 0;
}
