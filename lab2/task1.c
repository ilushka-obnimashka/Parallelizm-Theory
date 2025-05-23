#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <inttypes.h>

#ifdef NTHREADS
#else
#error "NTHREADS is not defined. Please specify -DNTHREADS=value during compilation."
#endif

#ifdef MATRIX_SIZE
#else
#error "MATRIX_SIZE is not defined. Please specify -DMATRIX_SIZE=value during compilation.(20000x20000 or 40000x40000)"
#endif

/**
 * @brief Displays an error message in Stderr.
 * @param message Error message.
 */
void fatal(char *message) {
    fprintf(stderr, "%s", message);
    exit(13);
}

/**
 * @brief calls malloc and makes an error if the value is null,
 * and returns the result only if the value is excellent from zero.
 * @param size Size isolated memory.
 */
void *xmalloc(size_t size) {
    register void *value = malloc(size);
    if (value == 0) fatal("Virtuel memory exhausted");
    return value;
}

 
/**
 * @brief Returns the current time in seconds.
 *        Time is measured using a system call.
 * @return Current time in seconds, including fractional part 
 *        with nanosecond precision.
 */
double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

/**
 * @briefCompute matrix-vector product vecRes[MATRIX_SIZE] = matrix[MATRIX_SIZE][MATRIX_SIZE] * vec[MATRIX_SIZE].
 * @warning the matrix must be represented in linear form.
 */
void MatrixVectorProductOmp(double *a, double *b, double *c, int m, int n) {
#pragma omp parallel num_threads(NTHREADS)
    {
        int tid = omp_get_thread_num();
        // number of vector elements counted by one thread
        int items_per_thread = m / NTHREADS; 
        /* The parallel part of the code will find the elements of the vector 
            in the range [LB, UB].
            LB - Lower_Bound
            UB - pper_bound
        */
        int lb = tid * items_per_thread;
        int ub = (tid == NTHREADS) ? (m - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; i++) {
            c[i] = 0;
            for (int j = 0; j < n; j++) {
                c[i] += a[i * m + j] * b[j];
            }
        }
    }
}

/**
 * @brief calculates the time spent on the parallel multiplication of the matrix 
 *        by the vector.
 * @return returns the minimum time (20 launches) spent on executing the parallel part.
 */
void TimeCheckParallel() {
    double *a, *b, *c;

    a = xmalloc(sizeof(*a) * MATRIX_SIZE * MATRIX_SIZE);
    b = xmalloc(sizeof(*b) * MATRIX_SIZE);
    c = xmalloc(sizeof(*c) * MATRIX_SIZE);

    #pragma omp parallel num_threads(NTHREADS)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = MATRIX_SIZE / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (MATRIX_SIZE - 1) : (lb + items_per_thread - 1);
        for (int i = lb; i <= ub; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++)
                a[i * MATRIX_SIZE + j] = i + j;
            c[i] = 0.0;
        }
    }
    for (int j = 0; j < MATRIX_SIZE; j++)
        b[j] = j;

    double min_time = 1000000000;

    for (int i = 0; i<20; i++){

        double start = cpuSecond();

        MatrixVectorProductOmp(a, b, c, MATRIX_SIZE, MATRIX_SIZE);
        
        double stop = cpuSecond();

        min_time = (min_time < (stop - start)) ? min_time : stop - start;
    }
    
    printf("Your calculations took %.4lf seconds.\n", min_time);


    free(a);
    free(b);
    free(c);
    
}

int main() {
    int m = MATRIX_SIZE;
    int n = MATRIX_SIZE;
    printf("Matrix-vector product (c[m] = a[m, n] * b[n]; m = %d, n = %d)\n", m, n);
    printf("Memory used: %" PRIu64 " MiB\n", ((m * n + m + n) * sizeof(double)) >> 20);
    printf("Number of threads: %d\n", NTHREADS);
    
    TimeCheckParallel();
}