#include <iostream>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

struct CityTemps {
    char city[100];
    int count, min, max;
    long sum;
};

inline int min(int a, int b) { return a ^ ((b ^ a) & -(b < a)); }
inline int max(int a, int b) { return a ^ ((a ^ b) & -(a < b)); }

char* parse_number(char *s, int *temperature) {
    int mod;
    if (*s == '-') {
        mod = -1;
        s++;
    } else {
        mod = 1;
    }

    if (s[0] == '.') {
        *temperature = (s[1] - '0') * mod;
        return s + 3;
    }

    if (s[1] == '.') {
        *temperature = ((s[0] * 10) + s[2] - ('0' * 11)) * mod;
        return s + 4;
    }

    *temperature = (s[0] * 100 + s[1] * 10 + s[3] - '0' * 111) * mod;
    return s + 5;
}

int hash(const unsigned char *data, int n) {
    unsigned int hash = 0;
    for (int i = 0; i < n; i++) {
        hash = (hash * 31) + data[i];
    }
    return hash;
}

int cmp(const void *a, const void *b) {
    return strcmp(((struct CityTemps *)a)->city, ((struct CityTemps *)b)->city);
}

int main(int argc, char* argv[]) {
    const char* fileName = "measurements.txt";
    if (argc > 1) {
        fileName = argv[1];
    }

    int fd = -1;
    fd = open(fileName, O_RDONLY);
    if (fd < 0) {
        perror("establish fd err");
        exit(EXIT_FAILURE);
    }
    
    struct stat s;
    fstat(fd, &s);
    size_t fileSize = s.st_size;

    struct CityTemps CityTemps[10000];
    int nCityTemps = 0;
    int map[32768];
    memset(map, -1, 32768 * 4);

    void* buf = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd , 0);
    madvise(buf, fileSize, MADV_SEQUENTIAL | MADV_WILLNEED | MADV_HUGEPAGE);

    char* buf_start = (char*)buf;
    char* buf_end = buf_start + fileSize;
    char * pos, *new_buf_start;
    int cityLength = 0, h, c;
    int measurement;

    while (buf_start < buf_end) {
        pos = strchr(buf_start, ';');
        if (pos == NULL) {
            break;
        }

        new_buf_start = parse_number(pos + 1, &measurement);
        cityLength = pos - buf_start;

        h = hash((unsigned char *)buf_start, cityLength) & (32768 - 1);
        while (map[h] != -1 && strncmp(CityTemps[map[h]].city, buf_start, cityLength) != 0) {
            h = (h + 1) & (32768 - 1);
        }
        c = map[h];

        if (c == -1) {
            strncpy(CityTemps[nCityTemps].city, buf_start, pos-buf_start);
            CityTemps[nCityTemps].sum = measurement;
            CityTemps[nCityTemps].max = measurement;
            CityTemps[nCityTemps].min = measurement;
            CityTemps[nCityTemps].count = 1;
            map[h] = nCityTemps;
            nCityTemps++;
        } else {
            CityTemps[c].sum += measurement;
            CityTemps[c].count += 1;
            CityTemps[c].min = min(CityTemps[c].min, measurement);
            CityTemps[c].max = max(CityTemps[c].max, measurement);
        }
        buf_start = new_buf_start;
    }
    qsort(CityTemps, (size_t)nCityTemps, sizeof(*CityTemps), cmp);

    std::cout << std::fixed;
    std::cout << std::setprecision(1);
    std::cout << "{";
    for (int i = 0; i < nCityTemps; i++) {
        std::cout << CityTemps[i].city << "=" << (float)CityTemps[i].min / 10.0 << "/" <<
        (float)CityTemps[i].sum / (float)CityTemps[i].count / 10.0 << "/" <<
        (float)CityTemps[i].max / 10.0 << (i < nCityTemps - 1 ? ", " : "");
    }
    std::cout << "}" << std::endl;

    close(fd);
    munmap(buf, fileSize);
}
