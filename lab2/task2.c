#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <inttypes.h>

#ifdef NTHREADS
#else
#error "NTHREADS is not defined. Please specify -DNTHREADS=value during compilation."
#endif

const int nsteps = 40000000;

/**
 * @brief Returns the current time in seconds.
 *        Time is measured using a system call.
 * @return Current time in seconds, including fractional part with 
 *         nanosecond precision.
 */
double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

/* To make it easier to check the correct work of programm, let's take ann 
    odd function, so that integral[-a,a](f)dx = 0.
*/
double f(double x){
    return sin(x);
}

/**
 * @brief Compute integral[a,b]f(x)dx using the parallel numerical
 *  midpoint rectangle method.
 */
double ParallelIntegral(double a, double b, double(*f)(double)){

    double h = (b-a)/nsteps;
    double sum = 0.0;

    #pragma omp parallel num_threads(NTHREADS) 
    {
        int threadid = omp_get_thread_num();
        int items_per_thread = nsteps / NTHREADS;
        int lb = threadid * items_per_thread;
        int ub = (threadid == NTHREADS) ? (nsteps - 1) : (lb + items_per_thread - 1);

        double sum_per_thread = 0.0;

        for (int i = lb; i<=ub; i++){
            sum_per_thread += f(a + h * (i + 0.5));
        }

        #pragma omp atomic
        sum += sum_per_thread;
    }
    sum *=h;
    return sum;
}


/**
 * @brief "Calculates the time spent on the parallel computation of the integral 
 *          using the numerical midpoint rectangle method. 
 * @return returns the minimum time (20 launches) spent on executing 
 *          the parallel part .
 */
void TimeCheckParallel() {

    double a = 10;
    printf("Integration f(x) on [%.12f, %.12f], nsteps = %d\n", -a, a, nsteps);

    double min_time = 1000000000;
    double result;

    for (int i = 0; i<20; i++){
        double start = cpuSecond();
        result = ParallelIntegral(-a, a, f);
        double stop = cpuSecond();
        min_time = (min_time < (stop - start)) ? min_time : stop - start;
    }
    
    printf("Your calculations took %.4lf seconds.\n", min_time);
    printf("Result (parallel): %.12f; error %.12f\n", result, fabs(result - 0));
    
}

int main(){
    printf("Number of threads: %d\n", NTHREADS);
    TimeCheckParallel();
}