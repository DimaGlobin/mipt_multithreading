#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <assert.h>

#define ZERO_SIZE 1
#define NULL_POINTER 2

#define BLOCK_SIZE 64
#define NS_PER_SECOND 1000000000

typedef struct {
    int threadId;
    int matrixSize;
    int blockSize;
    int startRow;
    int blocksPerThread;
    int** A;
    int** B;
    int** C;
} Data;

void subTimespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

int CacheFriendlyMultiply(int size, int** A, int** B, int** C) {
    if (size == 0) {
        return ZERO_SIZE;
    }

    if (A == NULL || C == NULL) {
        return NULL_POINTER;
    }

    for (int i = 0; i < size; i ++) {
        for (int k = 0; k < size; k++) {
            for (int j = 0; j < size; j++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return 0;
}

void *MultiplyMatrixThread(void *arg) {
    assert(arg != NULL);

    Data *data = (Data *)arg;

    int matrixSize = data->matrixSize;
    int blockSize = data->blockSize;
    int startRow = data->startRow;
    int blocksPerThread = data->blocksPerThread;

    int **A = data->A;
    int **B = data->B;
    int **C = data->C;

    int sum = 0;

    for (int ih = startRow; ih < matrixSize && ih < startRow + blockSize * blocksPerThread; ih += blockSize) {
        for (int kh = 0; kh < matrixSize; kh += blockSize) {
            for (int jh = 0; jh < matrixSize; jh += blockSize){
                for (int il = 0; il < blockSize && (ih + il) < matrixSize; ++il) {
                    for (int jl = 0; jl < blockSize && (jh + jl) < matrixSize; ++jl) {
                        for (int kl = 0; kl < blockSize && (kh + kl) < matrixSize; ++kl) {
                            sum += A[ih + il][kh + kl] * B[kh + kl][jh + jl];
                        }

                        C[ih + il][jh + jl] += sum;
                        sum = 0;
                    }
                }
            }
        }
    }

    pthread_exit(NULL);
}


int MultiplyMatrixParallel(int numOfthreads, int matrixSize, int** A, int** B, int** C) {
    assert(A != NULL);
    assert(B != NULL);
    assert(C != NULL);

    pthread_t threads[numOfthreads];
    Data threadData[numOfthreads];

    int blockSize = BLOCK_SIZE;
    int numBlocks = (matrixSize + blockSize - 1) / blockSize;
    int blocksPerThread = (numBlocks + numOfthreads - 1) / numOfthreads;
    
    for (int i = 0; i < numOfthreads; ++i) {
    threadData[i].threadId = i;
    threadData[i].matrixSize = matrixSize;
    threadData[i].blockSize = blockSize;
    threadData[i].A = A;
    threadData[i].B = B;
    threadData[i].C = C;
    threadData[i].blocksPerThread = blocksPerThread;
    threadData[i].startRow = i * blockSize * blocksPerThread;

    pthread_create(&threads[i], NULL, MultiplyMatrixThread, (void* )&threadData[i]);
}

    for (int i = 0; i < numOfthreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

timespec CreateAndMultiplyMatrcicies(int matrixSize) {
    struct timespec start, finish, delta;
    double duration;

    int** A = (int**)calloc(matrixSize, sizeof(int*));
    int** B = (int**)calloc(matrixSize, sizeof(int*));
    int** C = (int**)calloc(matrixSize, sizeof(int*));

    for (int i = 0; i < matrixSize; i++) {
        A[i] = (int* )calloc(matrixSize, sizeof(int));
        B[i] = (int* )calloc(matrixSize, sizeof(int));
        C[i] = (int* )calloc(matrixSize, sizeof(int));
    }

    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            A[i][j] = (int)(rand() % 100);
            B[i][j] = (int)(rand() % 100);
        }
    }

    clock_gettime(CLOCK_REALTIME, &start);
    int result = CacheFriendlyMultiply(matrixSize, A, B, C);
    clock_gettime(CLOCK_REALTIME, &finish);
    subTimespec(start, finish, &delta); 

    if (result != 0) {
        printf("Assert! result: %d", result);
    }

    for (int i = 0; i < matrixSize; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }

    free(A);
    free(B);
    free(C);

    return delta;
}

timespec RunParallelMultiplying(int matrixSize, int numOfThreads) {
    struct timespec start, finish, delta;
    double duration;

    int** A = (int**)calloc(matrixSize, sizeof(int*));
    int** B = (int**)calloc(matrixSize, sizeof(int*));
    int** C = (int**)calloc(matrixSize, sizeof(int*));

    for (int i = 0; i < matrixSize; i++) {
        A[i] = (int* )calloc(matrixSize, sizeof(int));
        B[i] = (int* )calloc(matrixSize, sizeof(int));
        C[i] = (int* )calloc(matrixSize, sizeof(int));
    }

    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            A[i][j] = (int)(rand() % 100);
            B[i][j] = (int)(rand() % 100);
        }
    }

    clock_gettime(CLOCK_REALTIME, &start);
    int result = MultiplyMatrixParallel(numOfThreads, matrixSize, A, B, C);
    clock_gettime(CLOCK_REALTIME, &finish);
    subTimespec(start, finish, &delta); 

    for (int i = 0; i < matrixSize; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }

    free(A);
    free(B);
    free(C);

    return delta;
}