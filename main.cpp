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
// The MCU may fill the TX buffer faster than the hardware can send it.
#define TX_BUF_SIZE (24 * 8)
// Must be aligned to sizeof float
#define TX_BUF_BYTES (TX_BUF_SIZE * sizeof(float))

// RX and TX Buffers
uint8_t rx_buf[RX_BUF_BYTES];
uint8_t tx_buf[TX_BUF_BYTES];

// Receive Window (byte index)
volatile size_t rec_wnd_start;
volatile size_t rec_wnd_end;

// Send Window (4 byte index)
volatile size_t send_wnd_start;
volatile size_t send_wnd_end;
volatile size_t send_num_floats;
volatile bool tx_busy = false;
volatile bool send_median = false;

Serial pc(USBTX, USBRX, 115200);

event_callback_t serialEventCb;
event_callback_t serialTxEventCb;
void serialCb(int events);
void serialTxCb(int events);

#ifdef DEBUG
// Debug
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
#endif

int main() {
    rec_wnd_start = 0;
    rec_wnd_end = 0;
    send_wnd_start = 0;
    send_wnd_end = 0;

    serialEventCb.attach(serialCb);
    serialTxEventCb.attach(serialTxCb);

    // When the receive window is full, the callback is invoked.
    pc.read(rx_buf, REC_WIN_BYTES,
            serialEventCb, SERIAL_EVENT_RX_COMPLETE);

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

            // Move start of the receive window starting forward.
            // Wrap if necessary.
            rec_wnd_start = (rec_wnd_start + REC_WIN_BYTES) % RX_BUF_BYTES;

            // Sample
            float x = *start;
            float y = *(start+1);
            float z = *(start+2);

            bool send_report = false;
            send_median = false;

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

#define TX_BUF_PUT(fval) \
            *(start + send_wnd_end) = fval; \
            send_wnd_end = (send_wnd_end + 1) % TX_BUF_SIZE;

            // We have collected enough samples.
            // Send report.
            if (send_report) {
                TX_BUF_PUT(stats.X.Min());
                TX_BUF_PUT(stats.X.Max());
                TX_BUF_PUT(stats.X.Mean());
                TX_BUF_PUT(stats.X.Variance());
                TX_BUF_PUT(stats.X.Skewness());
                TX_BUF_PUT(stats.X.Kurtosis());
                TX_BUF_PUT(stats.Y.Min());
                TX_BUF_PUT(stats.Y.Max());
                TX_BUF_PUT(stats.Y.Mean());
                TX_BUF_PUT(stats.Y.Variance());
                TX_BUF_PUT(stats.Y.Skewness());
                TX_BUF_PUT(stats.Y.Kurtosis());
                TX_BUF_PUT(stats.Z.Min());
                TX_BUF_PUT(stats.Z.Max());
                TX_BUF_PUT(stats.Z.Mean());
                TX_BUF_PUT(stats.Z.Variance());
                TX_BUF_PUT(stats.Z.Skewness());
                TX_BUF_PUT(stats.Z.Kurtosis());
                TX_BUF_PUT(stats.Correlation_XY());
                TX_BUF_PUT(stats.Correlation_XZ());
                TX_BUF_PUT(stats.Correlation_YZ());
            }

            if (send_median) {
                TX_BUF_PUT(stats.X.Median());
                TX_BUF_PUT(stats.Y.Median());
                TX_BUF_PUT(stats.Z.Median());
            }
#undef TX_BUF_PUT

            if ((send_report || send_median)
                    && !tx_busy && send_wnd_start != send_wnd_end) {
                // Send the report.

                tx_busy = true;

                send_num_floats = send_wnd_start <= send_wnd_end ?
                    (send_wnd_end - send_wnd_start) :
                    (TX_BUF_SIZE - send_wnd_start);

                if (send_num_floats > 0) {
                    int status = pc.write(
                            tx_buf + (send_wnd_start * sizeof(float)),
                            send_num_floats * sizeof(float),
                            serialTxEventCb, SERIAL_EVENT_TX_COMPLETE);

                    if (status) {
#ifdef DEBUG
                        // Error!
                        // We should not get here. If we do, it most
                        // likely means that we are transmitting the
                        // current report before the last one has finished.
                        led3 = 1;
#endif
                    }
                }
            }

            if (send_median) {
                // Reset everything.
                // TODO Probably not needed. If the autograder
                // resets the board after each test, then we
                // don't need this here.
                stats.Reset();
                //pc.abort_write();
                pc.abort_read();
                rec_wnd_start = 0;
                rec_wnd_end = 0;
                //memset(rx_buf, 0, RX_BUF_BYTES);
                //memset(tx_buf, 0, TX_BUF_BYTES);
                pc.read(rx_buf, REC_WIN_BYTES,
                        serialEventCb, SERIAL_EVENT_RX_COMPLETE);
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

void serialTxCb(int events) {
    if (events & SERIAL_EVENT_TX_COMPLETE) {
        // Move send window up
        send_wnd_start =
            (send_wnd_start + send_num_floats) % TX_BUF_SIZE;

        if (send_wnd_start != send_wnd_end) {
            send_num_floats = send_wnd_start <= send_wnd_end ?
                (send_wnd_end - send_wnd_start) :
                (TX_BUF_SIZE - send_wnd_start);

            if (send_num_floats > 0) {
                pc.write(
                        tx_buf + (send_wnd_start * sizeof(float)),
                        send_num_floats * sizeof(float),
                        serialTxEventCb, SERIAL_EVENT_TX_COMPLETE);
            }
        } else {
            if (send_median) {
                // That was the last transmission.
                send_wnd_start = 0;
                send_wnd_end = 0;
                pc.abort_write();
            }
            tx_busy = false;
        }
    }
}
