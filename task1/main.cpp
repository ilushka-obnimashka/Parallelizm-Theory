#include <vector>
#include <cmath>
#include <iostream>
#include <numeric>
#include <ctime>

#ifdef USE_DOUBLE
    typedef double my_type;
#elif defined(USE_FLOAT)
    typedef float my_type;
#else
    #error "It is necessary to determine DOUBLE or FLOAT."
#endif

int main() {
    unsigned int start_time =  clock();

    std::vector <my_type> arr(10000000);

    for (size_t i = 0 ; i<arr.size(); i++) {
        arr[i] = sin(i * M_PI / arr.size());
    }

    unsigned int end_time = clock();

    std::cout << "time: " << end_time - start_time << std::endl;
    std::cout << "sum: " << std::accumulate(arr.begin(), arr.end(), static_cast<my_type>(0)) << std::endl;
}
