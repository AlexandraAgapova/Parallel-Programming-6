#include <boost/program_options.hpp>
#include <iostream>
#include <chrono>
#include <memory>
#include <nvtx3/nvToolsExt.h>
#include <cmath>
#include <cstring>

// получаем одномерный индекс элемента из двумерного массива
inline int offset(int row, int col, int num_cols) {
    return row * num_cols + col;
}

void initialize(double* A, double* Anew, int rows, int cols) {
    memset(A, 0, sizeof(double) * rows * cols);
    memset(Anew, 0, sizeof(double) * rows * cols);

    double topLeft = 10.0;
    double topRight = 20.0;
    double bottomLeft = 20.0;
    double bottomRight = 30.0;
    
    // Линейная интерполяция
    // Верх матрицы
    for (int j = 0; j < cols; ++j) {
        double alpha = static_cast<double>(j) / (cols - 1);
        A[offset(0, j, cols)] = Anew[offset(0, j, cols)] = (1 - alpha) * topLeft + alpha * topRight;
    }

    // Низ матрицы
    for (int j = 0; j < cols; ++j) {
        double alpha = static_cast<double>(j) / (cols - 1);
        A[offset(rows - 1, j, cols)] = Anew[offset(rows - 1, j, cols)] = (1 - alpha) * bottomLeft + alpha * bottomRight;
    }

    // Левая часть матрицы
    for (int i = 0; i < rows; ++i) {
        double alpha = static_cast<double>(i) / (rows - 1);
        A[offset(i, 0, cols)] = Anew[offset(i, 0, cols)] = (1 - alpha) * topLeft + alpha * bottomLeft;
    }

    // Правая часть матрицы
    for (int i = 0; i < rows; ++i) {
        double alpha = static_cast<double>(i) / (rows - 1);
        A[offset(i, cols - 1, cols)] = Anew[offset(i, cols - 1, cols)] = (1 - alpha) * topRight + alpha * bottomRight;
    }
}

namespace po = boost::program_options;

int main(int argc, char** argv) {
    int size, iter_max;
    double tol;

    po::options_description desc("Доступные опции");
    desc.add_options()
        ("help", "помогите")
        ("size", po::value<int>(&size)->default_value(512), "размер (n x n)")
        ("tol", po::value<double>(&tol)->default_value(1.0e-6), "точность")
        ("max_iter", po::value<int>(&iter_max)->default_value(1000000), "максимальное кол-во итераций");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    int rows = size;
    int cols = size;

    std::unique_ptr<double[]> A_smart = std::make_unique<double[]>(rows * cols);
    std::unique_ptr<double[]> Anew_smart = std::make_unique<double[]>(rows * cols);

    nvtxRangePushA("init");
    initialize(A_smart.get(), Anew_smart.get(), rows, cols);
    nvtxRangePop();

    std::cout << "Посчитанная матрица " << rows << " x " << cols << "\n";

    std::chrono::steady_clock::time_point st = std::chrono::steady_clock::now();
    int iter = 0;
    double error = 1.0;
    auto A = A_smart.get();
    auto Anew = Anew_smart.get();

    nvtxRangePushA("while");
    #pragma acc data copy(A[0:rows*cols], Anew[0:rows*cols])
    {
        double inner_error = 0.0;
        while (error > tol && iter < iter_max) {

            #pragma acc parallel loop collapse(2) present(A, Anew)
            for (int i = 1; i < rows - 1; ++i) {
                for (int j = 1; j < cols - 1; ++j) {
                    Anew[offset(i, j, cols)] = 0.25 * (
                        A[offset(i, j+1, cols)] +
                        A[offset(i, j-1, cols)] +
                        A[offset(i+1, j, cols)] +
                        A[offset(i-1, j, cols)]
                    );
                }
            }

            std::swap(A, Anew);

            if (iter % 1000 == 0){
                inner_error = 0.0;
                #pragma acc parallel loop collapse(2) present(A, Anew) reduction(max:inner_error)
                for (int i = 1; i < rows - 1; ++i) {
                    for (int j = 1; j < cols - 1; ++j) {
                        inner_error = fmax(inner_error, fabs(Anew[offset(i, j, cols)] - A[offset(i, j, cols)]));
                    }
                }
                error = inner_error;
            }

            if(iter % 10000 == 0)
                std::cout << iter << ", ошибка = " << error << "\n";

            iter++;
        }
    }
    nvtxRangePop();

    if (size == 10 || size == 13) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << A[offset(i, j, cols)] << "\t";
            }
            std::cout << "\n";
        }
    }

    std::chrono::steady_clock::time_point fn = std::chrono::steady_clock::now();
    std::chrono::duration<double> runtime = std::chrono::duration_cast<std::chrono::duration<double>>(fn - st);

    std::cout << iter << " итераций\n";
    std::cout << "Ошибка: " << error << "\n";
    std::cout << "Время: " << runtime.count() << " секунд\n";

    return 0;
}
