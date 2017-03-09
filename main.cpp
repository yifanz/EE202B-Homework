#include "mbed.h"
#include "stats.h"

#define REC_WIN 3
#define RX_BUF_SIZE (REC_WIN * sizeof(float))
#define TX_BUF_SIZE (24 * sizeof(float))
uint8_t rx_buf[RX_BUF_SIZE];
uint8_t tx_buf[TX_BUF_SIZE];

event_callback_t serialEventCb;
void serialCb(int events);

Serial pc(USBTX, USBRX, 115200);

// Debug
//DigitalOut led1(LED1);

#define SAMPLE_SIZE 10
// TODO maybe we don't need so much precision. Try float instead of double.
GlobalStats<double> stats(SAMPLE_SIZE);

int main() {
    serialEventCb.attach(serialCb);

    // When rx_buf is full, the serialEventCb callback will be invoked.
    pc.read(rx_buf, sizeof(rx_buf),
            serialEventCb, SERIAL_EVENT_RX_COMPLETE);

    while (true) {
        // Execution halts here.
        // Will resume when an interrupt happens.
        sleep();

        // Debug
        //led1 = !led1;
        //wait(0.5);
    }
}

void serialCb(int events) {
    // SERIAL_EVENT_RX_COMPLETE indicates that receive buffer is full.
    if (events & SERIAL_EVENT_RX_COMPLETE) {
        float *start = (float*) rx_buf;
        float *end = start + REC_WIN;

        for (float *p = start; p < end; p += 3) {
            float x = *p;
            float y = *(p+1);
            float z = *(p+2);

            bool send_report = false;
            bool send_median = false;

            if (x == 0 && y == 0 && z == 0) {
                // When x, y and z are zero, we are done.
                // Do not include this sample in the stats.
                // Send final report and the Median.

                // TODO Is the final sample size 99 or is it still 100 and
                // the zero sample is sent right after?
                if (stats.GetFill()) {
                    send_report = true;
                }
                send_median = true;

                // Debug
                //led1 = 1;
            } else {
                // Update stats with sample.
                send_report = stats.Push(x, y, z);
            }

            start = (float*) tx_buf;
            end = start;

            // We have collected enough samples.
            // Send report.
            if (send_report) {
                *end++ = stats.X.Min();
                *end++ = stats.X.Max();
                *end++ = stats.X.Mean();
                *end++ = stats.X.Variance();
                *end++ = stats.X.Skewness();
                *end++ = stats.X.Kurtosis();
                *end++ = stats.Y.Min();
                *end++ = stats.Y.Max();
                *end++ = stats.Y.Mean();
                *end++ = stats.Y.Variance();
                *end++ = stats.Y.Skewness();
                *end++ = stats.Y.Kurtosis();
                *end++ = stats.Z.Min();
                *end++ = stats.Z.Max();
                *end++ = stats.Z.Mean();
                *end++ = stats.Z.Variance();
                *end++ = stats.Z.Skewness();
                *end++ = stats.Z.Kurtosis();
                *end++ = stats.Correlation_XY();
                *end++ = stats.Correlation_XZ();
                *end++ = stats.Correlation_YZ();
            }

            if (send_median) {
                *end++ = stats.X.Median();
                *end++ = stats.Y.Median();
                *end++ = stats.Z.Median();
            }

            if (send_report || send_median) {
                pc.write(tx_buf, sizeof(float) * (end - start), 0, 0);
            }

            if (send_median) {
                // Median is the final data sent.
                // Reset stats.
                // TODO not sure if this is needed. Depends on whether or
                // not the autograder resets the board.
                stats = GlobalStats<double>(SAMPLE_SIZE);
            }
        }

    } else {
        tx_buf[0] = 'E';
        tx_buf[1] = 'R';
        tx_buf[2] = 'R';
        tx_buf[3] = '!';
        tx_buf[4] = 0;
        pc.write(tx_buf, 5, 0, 0);
    }

    // When rx_buf is full, the serialEventCb callback will be invoked.
    pc.read(rx_buf, sizeof(rx_buf),
            serialEventCb, SERIAL_EVENT_RX_COMPLETE);
}
