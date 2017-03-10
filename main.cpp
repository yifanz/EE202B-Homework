#include "mbed.h"
#include "stats.h"

// Turn on for debugging.
//#define DEBUG

/**
 *
 *   Sample Win  |---------------n-samples--------------|
 *
 *   Receive Win |-x-y-z-||-x-y-z-| ... ... ... |-x-y-z-|
 *
 *   RX Buffer   |--------------------------------------------------------|
 *
 * Hardware writes to a physical RX buffer using DMA or otherwise.
 * Software maintains a logical receive and sample window over the
 * RX buffer.
 *
 * The hardware asynchronously fills the RX buffer and grows the end of
 * the receive window. After receiving every three floats (12 bytes) the
 * hardware should interrupt the processor.
 * The processor will wake-up and Push the sample (x,y,z) into the stats
 * calculator. The Push method in the stats calculator will keep track
 * of a sample window and return true when that window is full. Then
 * we transmit the stats report for those samples in the window.
 * The stats calculator will handle resetting the state for the next set
 * of samples.
 *
 **/

#define SAMPLE_SIZE 100
// Stats Calculator
GlobalStats<float> stats(SAMPLE_SIZE);

// Each sample has 3 floats: x, y and z.
#define REC_WIN 3
#define REC_WIN_BYTES (REC_WIN * sizeof(float))

// The hardware may fill the RX buffer faster than the processor can
// process the samples. Hence, we should make this big enough to account
// for that.
#define RX_BUF_SIZE 1024
#define RX_BUF_BYTES (REC_WIN * RX_BUF_SIZE * sizeof(float))
#define TX_BUF_BYTES (24 * sizeof(float))

// RX and TX Buffers
uint8_t rx_buf[RX_BUF_BYTES];
uint8_t tx_buf[TX_BUF_BYTES];

// Receive Window (byte index)
volatile size_t rec_wnd_start;
volatile size_t rec_wnd_end;

Serial pc(USBTX, USBRX, 115200);

event_callback_t serialEventCb;
void serialCb(int events);

#ifdef DEBUG
// Debug
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
#endif

int main() {
    rec_wnd_start = 0;
    rec_wnd_end = 0;

    serialEventCb.attach(serialCb);

    // When the receive window is full, the callback is invoked.
    pc.read(rx_buf, REC_WIN_BYTES,
            serialEventCb, SERIAL_EVENT_RX_COMPLETE);

    stats.Reset();
    while (true) {
        // Execution halts here.
        // Will resume when an interrupt happens.
        sleep();

#ifdef DEBUG
        // Debug
        //led1 = !led1;
        //wait(0.5);
#endif
        if (rec_wnd_start != rec_wnd_end) {
            float *start = (float*) (rx_buf + rec_wnd_start);
            float *end = start + REC_WIN;

            // Move start of the receive window starting forward.
            // Wrap if necessary.
            rec_wnd_start = (rec_wnd_start + REC_WIN_BYTES) % RX_BUF_BYTES;

            // Sample
            float x = *start;
            float y = *(start+1);
            float z = *(start+2);

            bool send_report = false;
            bool send_median = false;

            if (x == 0 && y == 0 && z == 0) {
                // When x, y and z are zero, we are done.
                // Do not include this sample in the stats.
                // Send final report and the Median.
                send_median = true;
            } else {
                // Update stats with sample.
                // When the sample window is full,
                // Push() will return true.
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
                // Send the report.

                for (uint8_t *p = (uint8_t*) start;
                        p < (uint8_t*) end;
                        p++) {
                    pc.putc(*p);
                }

                /*
                // Don't know why, but the async write just isn't reliable.
                int status = pc.write((uint8_t*) start,
                        sizeof(float) * (end - start),
                        0, 0);

                if (status) {
#ifdef DEBUG
                    // Error!
                    // We should not get here. If we do, it most
                    // likely means that we are transmitting the
                    // current report before the last one has finished.
                    led3 = 1;
#endif
                }
                */
            }

            if (send_median) {
                // Reset stats.
                // TODO Probably not needed. If the autograder
                // resets the board after each test, then we
                // don't need this here.
                stats.Reset();
                rec_wnd_start = 0;
                rec_wnd_end = 0;
            }
        }
    }
}

void serialCb(int events) {
    if (events & SERIAL_EVENT_RX_COMPLETE) {
        // Receive window is full.

        // Grow the receive window.
        // Wrap if necessary.
        rec_wnd_end = (rec_wnd_end + REC_WIN_BYTES) % RX_BUF_BYTES;

        // The main thread will periodically check if to see if
        // rec_wnd_end is != rec_wnd_start. If so, the main thread will
        // process the data up to rec_wnd_end.
        //
        // We CANNOT process the data inside the interrupt handler.
        // I tried and the time it takes to process the data will
        // cause us to miss the next transmission.
    } else {
        // Error!
        // We should never get here.
#ifdef DEBUG
        led3 = 1;
        tx_buf[0] = 'E';
        tx_buf[1] = 'R';
        tx_buf[2] = 'R';
        tx_buf[3] = '!';
        tx_buf[4] = 0;
        pc.write(tx_buf, 5, 0, 0);
#endif
    }

	pc.read(rx_buf + rec_wnd_end, REC_WIN_BYTES,
            serialEventCb, SERIAL_EVENT_RX_COMPLETE);
}
