#ifndef STATS_H
#define STATS_H

#include <queue>
#include <deque>
#include <limits>

template<typename T>
class RunningStats
{
    public:
        RunningStats()
        {
            Clear();
        }

        void Clear()
        {
            n = 0;
            M1 = M2 = M3 = M4 = 0.0;
            min = std::numeric_limits<T>::max();
            max = std::numeric_limits<T>::min();
        }

        void Reset()
        {
            Clear();
            min_heap =
                std::priority_queue<T, std::deque<T>, std::greater<T> >();
            max_heap =
                std::priority_queue<T, std::deque<T>, std::less<T> >();
        }

        T Median() const
        {
            if (min_heap.size() == max_heap.size())
                return(min_heap.top() + max_heap.top()) / ((T) 2.0);
            else if (min_heap.size() > max_heap.size())
                return min_heap.top();
            else
                return max_heap.top();
        }

        void PushMedian(T x)
        {
            if (min_heap.empty()) {
                min_heap.push(std::numeric_limits<T>::max());
            }

            if (max_heap.empty()) {
                max_heap.push (std::numeric_limits<T>::min());
            }

            if (x >= min_heap.top()) {
                min_heap.push(x);
            } else {
                max_heap.push(x);
            }

            if (min_heap.size() - max_heap.size() == 2) {
                max_heap.push(min_heap.top());
                min_heap.pop();
            } else if (max_heap.size() - min_heap.size() == 2) {
                min_heap.push(max_heap.top());
                max_heap.pop();
            }
        }

        void Push(T x)
        {
            T delta, delta_n, delta_n2, term1;

            long long n1 = n;
            n++;
            delta = x - M1;
            delta_n = delta / n;
            delta_n2 = delta_n * delta_n;
            term1 = delta * delta_n * n1;
            M1 += delta_n;
            M4 += term1 * delta_n2 * (n*n - 3*n + 3) + 6 * delta_n2
                * M2 - 4 * delta_n * M3;
            M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
            M2 += term1;

            if (x < min) {
                min = x;
            }

            if (x > max) {
                max = x;
            }

            PushMedian(x);
        }

        long long NumDataValues() const
        {
            return n;
        }

        T Mean() const
        {
            return M1;
        }

        T VarianceSample() const
        {
            return M2/(n-1.0);
        }

        T Variance() const
        {
            return M2/(n);
        }

        T StandardDeviation() const
        {
            return sqrt(Variance());
        }

        T Skewness() const
        {
            return sqrt(T(n)) * M3/ pow(M2, 1.5);
        }

        T Kurtosis() const
        {
            // Fisher's definition
            //return T(n)*M4 / (M2*M2) - 3.0;

            // We use Pearson's definition
            return T(n)*M4 / (M2*M2);
        }

        T Max() const
        {
            return max;
        }

        T Min() const
        {
            return min;
        }

    private:
        long long n;
        T M1, M2, M3, M4;
        T min, max;

        std::priority_queue<T, std::deque<T>, std::greater<T> > min_heap;
        std::priority_queue<T, std::deque<T>, std::less<T> > max_heap;
};

template<typename T>
class GlobalStats
{
    public:
        GlobalStats(unsigned int window)
            : window(window), M_XY(0), M_XZ(0), M_YZ(0)
        {
        }

        bool Push(T x, T y, T z)
        {
            if (X.NumDataValues() >= window) Clear();

            long long n = X.NumDataValues() + 1;
            T delta_X = (x - X.Mean()) / n;
            T delta_Y = (y - Y.Mean()) / n;
            T delta_Z = (z - Z.Mean()) / n;
            M_XY += (n - 1) * delta_X * delta_Y - M_XY / n;
            M_XZ += (n - 1) * delta_X * delta_Z - M_XZ / n;
            M_YZ += (n - 1) * delta_Y * delta_Z - M_YZ / n;

            X.Push(x);
            Y.Push(y);
            Z.Push(z);

            if (X.NumDataValues() == window) return true;
            else return false;
        }

        unsigned int GetFill()
        {
            return X.NumDataValues();
        }

        void Clear()
        {
            // Clear does NOT clear the Median because
            // we need to track the Median for all samples.
            X.Clear();
            Y.Clear();
            Z.Clear();
        }

        void Reset()
        {
            M_XY = 0;
            M_XZ = 0;
            M_YZ = 0;
            X.Reset();
            Y.Reset();
            Z.Reset();
        }

        T Correlation_XY()
        {
            long long n = X.NumDataValues();
            T cov = n / (n - 1) * M_XY;
            return cov / (X.StandardDeviation() * Y.StandardDeviation());
        }

        T Correlation_XZ()
        {
            long long n = X.NumDataValues();
            T cov = n / (n - 1) * M_XZ;
            return cov / (X.StandardDeviation() * Z.StandardDeviation());
        }

        T Correlation_YZ()
        {
            long long n = Y.NumDataValues();
            T cov = n / (n - 1) * M_YZ;
            return cov / (Y.StandardDeviation() * Z.StandardDeviation());
        }

        RunningStats<T> X;
        RunningStats<T> Y;
        RunningStats<T> Z;

    private:
        unsigned int window;
        T M_XY;
        T M_XZ;
        T M_YZ;
};

#endif /* STATS_H */
