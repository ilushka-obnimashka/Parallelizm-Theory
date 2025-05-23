#include <iostream>
#include <vector>
#include <omp.h>
#include <cmath>
#include <iomanip>
#include <random>

#ifdef NTHREADS
#else
#error "NTHREADS is not defined. Please specified -DNTHREADS=value during compilation."
#endif

#ifdef MATRIX_SIZE
#else
#error "MATRIX_SIZE is not defined. Please specify -DMATRIX_SIZE=value during compilation.(20000x20000 or 40000x40000)"
#endif

const double kITERATION_STEP = 1.0 / 100000.0;
double epsilon = 0.00001;
const int kMAX_ITERATIONS = 10000000; 

/**
 * @brief Returns the current time in seconds.
 *        Time is measured using a system call.
 * @return Current time in seconds, including fractional part 
 *         with nanosecond precision.
 */
double CpuSecond() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);`
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

/**
 * @brief Computes the matrix-vector product vecRes[MATRIX_SIZE] = matrix[MATRIX_SIZE][MATRIX_SIZE] * vec[MATRIX_SIZE].
 * @warning The matrix must be represented in linear form.
 */
void MatrixVectorProductOmp(const long double *matrix, const long double *vec, long double *vecRes) {
    #pragma omp parallel for num_threads(NTHREADS) schedule(static)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        vecRes[i] = 0;
        for (size_t j = 0; j < MATRIX_SIZE; j++) {
            vecRes[i] += matrix[i * MATRIX_SIZE + j] * vec[j];
        }
    }
}

/**
 * @brief Computes the difference between two vectors vec1[n] -= vec2[MATRIX_SIZE].
 * @warning Both vectors must have the same size.
 */
void SubtractVecFromVec(long double *vec1, const long double *vec2) {
    #pragma omp parallel for num_threads(NTHREADS) schedule(static)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        vec1[i] -= vec2[i];
    }
}

/**
 * @brief Computes the scalar-vector product vec[MATRIX_SIZE] *= scalar.
 */
void MultiplyVecByScalar(long double *vec, const long double &scalar) {
    #pragma omp parallel for num_threads(NTHREADS) schedule(static)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        vec[i] *= scalar;
    }
}

/**
 * @brief Computes the L2 norm of a vector.
 * @return The L2 norm of the vector.
 */
double VecL2Norm(const long double *vec) {
    long double l2Norm = 0.0;
    #pragma omp parallel for num_threads(NTHREADS) schedule(static) reduction(+:l2Norm)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        l2Norm += vec[i] * vec[i];
    }

    return std::sqrt(l2Norm);
}

/**
 * @brief Implements the simple iteration method for solving systems of linear equations.
 * @return The time taken to execute the method.
 */
double IterationMethod() {
    long double* matrixAData = new long double[MATRIX_SIZE * MATRIX_SIZE];
    long double* vecBData = new long double[MATRIX_SIZE];
    long double* vecX = new long double[MATRIX_SIZE];
    long double* vecTemp = new long double[MATRIX_SIZE];

    // Initialization of matrix A
    #pragma omp parallel for num_threads(NTHREADS) schedule(static)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        for (size_t j = 0; j < MATRIX_SIZE; j++) {
            matrixAData[i * MATRIX_SIZE + j] = (i == j) ? 2.0 : 1.0;
        }
    }

    // Initialization of vectors b and x
    #pragma omp parallel for num_threads(NTHREADS) schedule(static)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        vecBData[i] = MATRIX_SIZE + 1;
        vecX[i] = 0.0;
    }

    const long double* matrixA = matrixAData;
    const long double* vecB = vecBData;

    epsilon *= VecL2Norm(vecB);

    int iterationCount = 0;

    double startTime = CpuSecond();

    while (iterationCount++ >= 0) {
        // vecTemp = matrixA * vecX
        MatrixVectorProductOmp(matrixA, vecX, vecTemp);

        // vecTemp = matrixA * vecX - b
        SubtractVecFromVec(vecTemp, vecB);

        // Check for convergence
        if (VecL2Norm(vecTemp) < epsilon) break;

        // Check for exceeding the maximum number of iterations
        if (iterationCount >= kMAX_ITERATIONS) {
            std::cerr << "Error: Exceeded maximum number of iterations (" << MAX_ITERATIONS << ")." << std::endl;
            delete[] matrixAData;
            delete[] vecBData;
            delete[] vecX;
            delete[] vecTemp;
            exit(13);
        }

        // vecTemp = iterationStep * (matrixA * vecX - b)
        MultiplyVecByScalar(vecTemp, kITERATION_STEP);

        // vecX -= iterationStep * (matrixA * vecX - b)
        SubtractVecFromVec(vecX, vecTemp);
    }

    double endTime = CpuSecond();

    long double sumAbsoluteError = 0.0;
    long double sumRelativeError = 0.0;

    #pragma omp parallel for num_threads(NTHREADS) schedule(static) reduction(+:sumAbsoluteError, sumRelativeError)
    for (int i = 0; i < MATRIX_SIZE; i++) {
        long double absoluteError = std::abs(vecX[i] - 1.0);
        long double relativeError = std::abs((vecX[i] - 1.0) / 1.0);
        
        sumAbsoluteError += absoluteError;
        sumRelativeError += relativeError;
    }

    std::cout << std::endl;
    std::cout << "Number of iterations performed: " << iterationCount << std::endl;
    std::cout << "Sum of absolute errors: " << sumAbsoluteError << std::endl;
    std::cout << "Sum of relative errors: " << sumRelativeError << std::endl;

    delete[] matrixAData;
    delete[] vecBData;
    delete[] vecX;
    delete[] vecTemp;

    return endTime - startTime;
}

int main(int argc, char* argv[]) {
    int matrixSize = MATRIX_SIZE;

    std::cout << "Program using Simple Iteration method for solving linear systems (CLAY)" << std::endl;
    std::cout << "CLAY : A[" << MATRIX_SIZE << "][" << MATRIX_SIZE << "] * x[" << MATRIX_SIZE << "] = b[" << MATRIX_SIZE << "]\n";
    std::cout << "Number of threads: " << NTHREADS << std::endl;
    std::cout << "Memory used: " << static_cast<long double>((MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE + MATRIX_SIZE + MATRIX_SIZE) * sizeof(long double)) / (1024 * 1024) << " MiB\n";

    double time = IterationMethod();
    std::cout << "Your calculations took " << std::fixed << std::setprecision(4) << time << " seconds." << std::endl;

    return 0;
}