#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "1_task.hpp"

int main() {
    FILE *nonParallel, *parallel;
    nonParallel = fopen("non_parallel.txt", "w+");
    parallel = fopen("parallel.txt", "w+");
    assert(nonParallel != NULL);
    assert(parallel != NULL);
    timespec result;

    for (int i = 100; i < 2051; i += 50) {
        result = CreateAndMultiplyMatrcicies(i);
        printf("size: %d, time: %d.%.9ld\n", i, (int)result.tv_sec, result.tv_nsec);
        fprintf(nonParallel, "size: %d, time: %d.%.9ld\n", i, (int)result.tv_sec, result.tv_nsec);
    }
    int size = 2048; 

    for (int i = 1; i <= 16; i++) {
        result = RunParallelMultiplying(size, i);
        printf("threads_num: %d, time: %d.%.9ld\n", i, (int)result.tv_sec, result.tv_nsec);
        fprintf(parallel, "threads_num: %d, time: %d.%.9ld\n", i, (int)result.tv_sec, result.tv_nsec);
    }

    fclose(nonParallel);
    fclose(parallel);
    return 0;
}