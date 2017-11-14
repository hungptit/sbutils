#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <vector>

#include "celero/Celero.h"

// write N * 512 integers
void fillFile(int N, char *name) {
    FILE *fd = ::fopen(name, "wb");
    if (fd == NULL) {
        std::cerr << "problem"
                  << "\n";
        return;
    }
    int numbers[512];
    for (int k = 0; k < 512; ++k) numbers[k] = k; // whatever
    for (int t = 0; t < N; ++t) {
        int size = 512;
        size_t realsize;
        int err, error;
        if ((realsize = fwrite(&size, sizeof(int), 1, fd)) != 1) {
            err = errno;
            error = ferror(fd);
            if (error) {
                std::cout << "We written only " << realsize << " bytes"
                          << "\n";
                std::cout << "[ERROR] " << strerror(err) << "\n";
                exit(EXIT_FAILURE);
            }
            return;
        }
        if ((realsize = fwrite(&numbers[0], sizeof(int), 512, fd)) != 512) {
            err = errno;
            error = ferror(fd);
            if (error) {
                std::cout << "We written only" << realsize << "bytes"
                          << "\n";
                std::cout << "[ERROR] " << strerror(err) << "\n";
                exit(EXIT_FAILURE);
            }
            return;
        }
    }
    ::fclose(fd);
}

int DoSomeComputation(int *numbers, size_t size) {
    int answer = 0;
    // totally arbitrary
    for (size_t k = 0; k < size; k += 2) {
        answer += numbers[k];
    }
    for (size_t k = 1; k < size; k += 2) {
        answer += numbers[k] * 2;
    }
    return answer;
}

int testfread(char *name, int N) {
    int answer = 0;
    FILE *fd = ::fopen(name, "rb");
    if (fd == NULL) {
        std::cerr << "problem"
                  << "\n";
        return -1;
    }
    std::vector<int> numbers(512);
    for (int t = 0; t < N; ++t) {
        int size = 0;
        if (fread(&size, sizeof(int), 1, fd) != 1) {
            std::cout << "Data can't be read???"
                      << "\n";
            return -1;
        }
        numbers.resize(size);
        if (fread(&numbers[0], sizeof(int), numbers.size(), fd) != numbers.size()) {
            std::cout << "Data can't be read???"
                      << "\n";
            return -1;
        }
        answer += DoSomeComputation(&numbers[0], numbers.size());
    }
    ::fclose(fd);
    return answer;
}

int testwithCpp(char *name, int N) {
    int answer = 0;
    std::ifstream in(name, std::ios::binary);
    std::vector<int> numbers(512);
    for (int t = 0; t < N; ++t) {
        int size = 0;
        in.read(reinterpret_cast<char *>(&size), sizeof(int));
        numbers.resize(size);
        in.read(reinterpret_cast<char *>(&numbers[0]), sizeof(int) * size);
        answer += DoSomeComputation(&numbers[0], numbers.size());
    }
    in.close();
    return answer;
}

int testfreadwithsetbuffer(char *name, int N) {
    int answer = 0;
    FILE *fd = ::fopen(name, "rb");
    setvbuf(fd, NULL, _IOFBF, 1024 * 4); // large buffer
    if (fd == NULL) {
        std::cerr << "problem"
                  << "\n";
        return -1;
    }
    std::vector<int> numbers(512);
    for (int t = 0; t < N; ++t) {
        int size = 0;
        if (fread(&size, sizeof(int), 1, fd) != 1) {
            std::cout << "Data can't be read???"
                      << "\n";
            return -1;
        }
        numbers.resize(size);
        if (fread(&numbers[0], sizeof(int), numbers.size(), fd) != numbers.size()) {
            std::cout << "Data can't be read???"
                      << "\n";
            return -1;
        }
        answer += DoSomeComputation(&numbers[0], numbers.size());
    }
    ::fclose(fd);
    return answer;
}

int testfreadwithlargebuffer(char *name, int N) {
    int answer = 0;
    FILE *fd = ::fopen(name, "rb");
    setvbuf(fd, NULL, _IOFBF, 1024 * 1024 * 32); // large buffer
    if (fd == NULL) {
        std::cerr << "problem"
                  << "\n";
        return -1;
    }
    std::vector<int> numbers(512);
    for (int t = 0; t < N; ++t) {
        int size = 0;
        if (fread(&size, sizeof(int), 1, fd) != 1) {
            std::cout << "Data can't be read???"
                      << "\n";
            return -1;
        }
        numbers.resize(size);
        if (fread(&numbers[0], sizeof(int), numbers.size(), fd) != numbers.size()) {
            std::cout << "Data can't be read???"
                      << "\n";
            return -1;
        }
        answer += DoSomeComputation(&numbers[0], numbers.size());
    }
    ::fclose(fd);
    return answer;
}

int testwmmap(char *name, int N, bool advise, bool shared) {
    int answer = 0;
    int fd = ::open(name, O_RDONLY);
    size_t length = N * (512 + 1) * 4;
    // for Linux:
#ifdef __linux__
    int *addr = reinterpret_cast<int *>(
        mmap(NULL, length, PROT_READ,
             MAP_FILE | (shared ? MAP_SHARED : MAP_PRIVATE) | MAP_POPULATE, fd, 0));
#else
    int *addr = reinterpret_cast<int *>(
        mmap(NULL, length, PROT_READ, MAP_FILE | (shared ? MAP_SHARED : MAP_PRIVATE), fd, 0));
#endif
    int *initaddr = addr;
    if (addr == MAP_FAILED) {
        std::cout << "Data can't be mapped???"
                  << "\n";
        return -1;
    }
    if (advise)
        if (madvise(addr, length, MADV_SEQUENTIAL | MADV_WILLNEED) != 0)
            std::cerr << " Couldn't set hints"
                      << "\n";
    close(fd);
    for (int t = 0; t < N; ++t) {
        int size = *addr++;
        answer += DoSomeComputation(addr, size);
        addr += size;
    }
    munmap(initaddr, length);
    return answer;
}

int testread(char *name, int N) {
    int answer = 0;
    int fd = ::open(name, O_RDONLY);
    if (fd < 0) {
        std::cerr << "problem"
                  << "\n";
        return -1;
    }
    std::vector<int> numbers(512);
    for (int t = 0; t < N; ++t) {
        int size = 0;
        if (read(fd, &size, sizeof(int)) != sizeof(int)) {
            std::cout << "Data can't be read???"
                      << "\n";
            return -1;
        }
        numbers.resize(size);
        if (read(fd, &numbers[0], sizeof(int) * numbers.size()) !=
            sizeof(int) * numbers.size()) {
            std::cout << "Data can't be read???"
                      << "\n";
            return -1;
        }
        answer += DoSomeComputation(&numbers[0], numbers.size());
    }
    ::close(fd);
    return answer;
}

class WallClockTimer {
  public:
    struct timeval t1, t2;
    WallClockTimer() : t1(), t2() {
        gettimeofday(&t1, 0);
        t2 = t1;
    }
    void reset() {
        gettimeofday(&t1, 0);
        t2 = t1;
    }
    unsigned long long elapsed() {
        return ((t2.tv_sec - t1.tv_sec) * 1000ULL * 1000ULL) + ((t2.tv_usec - t1.tv_usec));
    }
    unsigned long long split() {
        gettimeofday(&t2, 0);
        return elapsed();
    }
};

class CPUTimer {
  public:
    // clock_t t1, t2;
    struct rusage t1, t2;

    CPUTimer() : t1(), t2() {
        getrusage(RUSAGE_SELF, &t1);
        // t1 = clock();
        t2 = t1;
    }
    void reset() {
        getrusage(RUSAGE_SELF, &t1);
        // t1 = clock();
        t2 = t1;
    }
    // proxy for userelapsed
    unsigned long long elapsed() {
        return totalelapsed(); // userelapsed();
    }

    unsigned long long totalelapsed() { return userelapsed() + systemelapsed(); }
    // returns the *user* CPU time in micro seconds (mu s)
    unsigned long long userelapsed() {
        return ((t2.ru_utime.tv_sec - t1.ru_utime.tv_sec) * 1000ULL * 1000ULL) +
               ((t2.ru_utime.tv_usec - t1.ru_utime.tv_usec));
    }

    // returns the *system* CPU time in micro seconds (mu s)
    unsigned long long systemelapsed() {
        return ((t2.ru_stime.tv_sec - t1.ru_stime.tv_sec) * 1000ULL * 1000ULL) +
               ((t2.ru_stime.tv_usec - t1.ru_stime.tv_usec));
    }

    unsigned long long split() {
        getrusage(RUSAGE_SELF, &t2);
        return elapsed();
    }
};

constexpr int N = 16384 * 16;

int main() {
    int tot = 0;
    CPUTimer cput;
    WallClockTimer wct;
	const int number_of_loops = 30;
	const int number_of_trials = 10;
	
    for (int T = 0; T < number_of_loops; ++T) {
		char *name = tmpnam(NULL); // unsafe but for these purposes, ok
		fillFile(N, name);
		std::cout << "Test file: " << name << "\n";
		
        std::cout << "\n";
        // don't report times
        tot += testread(name, N);

        // fread
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testfread(name, N);
        std::cout << "fread\t\t\t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        // fread with set buffer
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testfreadwithsetbuffer(name, N);
        std::cout << "fread w sbuffer\t\t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        // fread with large buffer
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testfreadwithlargebuffer(name, N);
        std::cout << "fread w lbuffer\t\t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        // read
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testread(name, N);
        std::cout << "read2 \t\t\t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        // mmap
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testwmmap(name, N, false, false);
        std::cout << "mmap \t\t\t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        // fancy mmap
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testwmmap(name, N, true, false);
        std::cout << "fancy mmap \t\t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        // mmap
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testwmmap(name, N, false, true);
        std::cout << "mmap (shared) \t\t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        // fancy mmap
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testwmmap(name, N, true, true);
        std::cout << "fancy mmap (shared) \t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        // C++
        cput.reset();
        wct.reset();
        for (int x = 0; x < number_of_trials; ++x) tot += testwithCpp(name, N);
        std::cout << "Cpp\t\t\t" << 512 * N * 1.0 / cput.split() << " "
                  << 512 * N * 1.0 / wct.split() << "\n";

        ::remove(name);
    }
}
