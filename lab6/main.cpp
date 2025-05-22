#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include "all_classes.h"

#define YELLOW "\033[33m"
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define ITERATION_STEP -0.01
#define OUT_FILE "result.dat"

namespace po = boost::program_options;

void report_option_defaults(po::variables_map &vm);

void record_results(int grid_size, long double *x) {
    FILE *f = fopen(OUT_FILE, "w");
    fwrite(x, sizeof(double), grid_size * grid_size, f);
    fclose(f);
}

void saveSolution(const char* filename, const double* x, int size) {
    FILE* f = fopen(filename, "wb");
    if (f) {
        fwrite(x, sizeof(double), size, f);
        fclose(f);
    } else {
        std::cerr << "Error opening file " << filename << " for writing.\n";
    }
}

int main(int argc, char *argv[]) {
    double epsilon;
    int grid_size, itterations;

    po::options_description desc("Параметры запуска");
    desc.add_options()
            ("help,h", "Показать справку")
            ("epsilon,e", po::value<double>()->default_value(0.01), "Точность вычислений")
            ("grid-size,g", po::value<int>()->default_value(30), "Размер сетки")
            ("itterations,i", po::value<int>()->default_value(1e6), "Количество иттераций");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    report_option_defaults(vm);
    epsilon = vm["epsilon"].as<double>();
    grid_size = vm["grid-size"].as<int>();
    itterations = vm["itterations"].as<int>();

    auto start_time = std::chrono::high_resolution_clock::now();

    ThermalSolver solver(grid_size, grid_size, epsilon, itterations, -0.01);
    solver.solve();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    std::cout << "Время работы алгоритма: " << elapsed_seconds.count() << " секунд" << std::endl;

    saveSolution("result.dat", solver.getSolution(), solver.getSize());
}


void report_option_defaults(po::variables_map &vm) {
    if (vm["epsilon"].defaulted()) {
        std::cout << YELLOW << "[Warning] Параметр 'epsilon' не задан. Используется значение по умолчанию: "
                << vm["epsilon"].as<double>() << RESET << std::endl;
    } else {
        std::cout << GREEN << "Параметр задан явно: " << vm["epsilon"].as<double>() << RESET << std::endl;
    }

    if (vm["grid-size"].defaulted()) {
        std::cout << YELLOW << "[Warning] Параметр 'grid-size' не задан. Используется значение по умолчанию: "
                << vm["grid-size"].as<int>() << RESET << std::endl;
    } else {
        std::cout << GREEN << "Параметр 'grid-size' задан явно: "
                << vm["grid-size"].as<int>() << RESET << std::endl;
    }

    if (vm["itterations"].defaulted()) {
        std::cout << YELLOW << "[Warning] Параметр 'itterations' не задан. Используется значение по умолчанию: "
                << vm["itterations"].as<int>() << RESET << std::endl;
    } else {
        std::cout << GREEN << "Параметр 'itterations' задан явно: "
                << vm["itterations"].as<int>() << RESET << std::endl;
    }
}
