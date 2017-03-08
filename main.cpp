#include "mbed.h"

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
            first = true;
            min = 0;
            max = 0;
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

            if (x < min || first) {
                min = x;
                first = false;
            }

            if (x > max || first) {
                max = x;
                first = false;
            }
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

        T Skewness() const
        {
            return sqrt(T(n)) * M3/ pow(M2, 1.5);
        }

        T Kurtosis() const
        {
            return T(n)*M4 / (M2*M2) - 3.0;
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
        bool first;
};

#define REC_WIN 10
#define RX_BUF_SIZE (REC_WIN * sizeof(float))

event_callback_t serialEventCb;
void serialCb(int events);

uint8_t rx_buf[RX_BUF_SIZE];

Serial pc(USBTX, USBRX, 115200);
DigitalOut led1(LED1);

int main() {
    serialEventCb.attach(serialCb);
    pc.read(rx_buf, sizeof(rx_buf),
            serialEventCb, SERIAL_EVENT_RX_COMPLETE);

    while (true) {
        sleep();
        led1 = !led1;
    }
}

void serialCb(int events) {
    if (events & SERIAL_EVENT_RX_COMPLETE) {
        RunningStats<double> x_stats;
        RunningStats<double> y_stats;
        RunningStats<double> z_stats;

        float *start = (float*) rx_buf;
        float *end = start + REC_WIN;

        for (float *p = start; p < end; p++) {
            x_stats.Push(*p);
        }

        end = start;
        //*end++ = x_stats.NumDataValues();
        *end++ = x_stats.Min();
        *end++ = x_stats.Max();
        *end++ = x_stats.Mean();
        *end++ = x_stats.Variance();
        *end++ = x_stats.Skewness();
        *end++ = x_stats.Kurtosis();
        pc.write(rx_buf, sizeof(float) * (end - start), 0, 0);
    } else {
        rx_buf[0] = 'E';
        rx_buf[1] = 'R';
        rx_buf[2] = 'R';
        rx_buf[3] = '!';
        rx_buf[4] = 0;
        pc.write(rx_buf, 5, 0, 0);
    }

    pc.read(rx_buf, sizeof(rx_buf),
            serialEventCb, SERIAL_EVENT_RX_COMPLETE);
}
