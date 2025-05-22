#ifndef ALL_CLASSES_H
#define ALL_CLASSES_H

class ThermalSolver {
private:
    int Nx_, Ny_;
    int size_;
    double tau_;
    double epsilon_;
    int max_iter_;

    double *A_; // размер size_ * size_
    double *b_; // размер size_
    double *x_; // размер size_

    const double corners_[4] = {10.0, 20.0, 30.0, 20.0};

    int idx(int row, int col) const { return row * Nx_ + col; }

    void initMatrix() {
        std::memset(A_, 0, size_ * size_ * sizeof(double));

        for (int row = 0; row < size_; ++row) {
            A_[row * size_ + row] = -4.0;

            if (row % Nx_ != 0)
                A_[row * size_ + (row - 1)] = 1.0;

            if ((row + 1) % Nx_ != 0)
                A_[row * size_ + (row + 1)] = 1.0;

            if (row - Nx_ >= 0)
                A_[row * size_ + (row - Nx_)] = 1.0;

            if (row + Nx_ < size_)
                A_[row * size_ + (row + Nx_)] = 1.0;
        }
    }

    void initB() {
        std::memset(b_, 0, size_ * sizeof(double));

        // Верхняя граница (row=0)
        for (int col = 0; col < Nx_; ++col) {
            double t = static_cast<double>(col) / (Nx_ - 1);
            b_[idx(0, col)] = corners_[0] * (1 - t) + corners_[1] * t;
        }

        // Нижняя граница (row=Ny_-1)
        for (int col = 0; col < Nx_; ++col) {
            double t = static_cast<double>(col) / (Nx_ - 1);
            b_[idx(Ny_ - 1, col)] = corners_[3] * (1 - t) + corners_[2] * t;
        }

        // Левая граница (col=0)
        for (int row = 0; row < Ny_; ++row) {
            double t = static_cast<double>(row) / (Ny_ - 1);
            b_[idx(row, 0)] = corners_[0] * (1 - t) + corners_[3] * t;
        }

        // Правая граница (col=Nx_-1)
        for (int row = 0; row < Ny_; ++row) {
            double t = static_cast<double>(row) / (Ny_ - 1);
            b_[idx(row, Nx_ - 1)] = corners_[1] * (1 - t) + corners_[2] * t;
        }
    }

    double norm(const double *v) const {
        double s = 0.0;
        for (int i = 0; i < size_; ++i) {
            s += v[i] * v[i];
        }
        return std::sqrt(s);
    }

    void mul_mv_sub(double *res, const double *A, const double *x, const double *y) {
        for (int i = 0; i < size_; ++i) {
            double sum = -y[i];
            for (int j = 0; j < size_; ++j) {
                sum += A[i * size_ + j] * x[j];
            }
            res[i] = sum;
        }
    }

    void next(double *x, const double *delta) {
        for (int i = 0; i < size_; ++i) {
            x[i] -= tau_ * delta[i];
        }
    }

public:
    ThermalSolver(int Nx, int Ny, double epsilon, int max_iter, double tau)
        : Nx_(Nx), Ny_(Ny), size_(Nx * Ny), epsilon_(epsilon), max_iter_(max_iter), tau_(tau) {
        A_ = new double[size_ * size_];
        b_ = new double[size_];
        x_ = new double[size_];
        std::memset(x_, 0, size_ * sizeof(double));
    }

    ~ThermalSolver() {
        delete[] A_;
        delete[] b_;
        delete[] x_;
    }

    void solve() {
        initMatrix();
        initB();

        double *Axmb = new double[size_];
        double norm_b = norm(b_);
        double norm_Axmb;
        int iter = 0;

        do {
            mul_mv_sub(Axmb, A_, x_, b_);
            norm_Axmb = norm(Axmb);
            next(x_, Axmb);
            iter++;

            if (iter % 1000 == 0 || iter == 1) {
                std::cout << "Iteration " << iter << ": Residual norm = " << norm_Axmb / norm_b << std::endl;
            }

            if (iter >= max_iter_) break;
        } while (norm_Axmb / norm_b >= epsilon_);

        std::cout << "\nConverged after " << iter << " iterations. Final relative residual = "
                << norm_Axmb / norm_b << std::endl;

        delete[] Axmb;
    }

    const double *getSolution() const {
        return x_;
    }

    int getSize() const {
        return size_;
    }
};

#endif