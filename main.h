#ifndef MAIN_H
#define MAIN_H

extern char SIGNAL_AX[];
extern char SIGNAL_AY[];
extern char SIGNAL_AZ[];
extern char SIGNAL_GX[];
extern char SIGNAL_GY[];
extern char SIGNAL_GZ[];
extern char SIGNAL_MX[];
extern char SIGNAL_MY[];
extern char SIGNAL_MZ[];

#define BUFF_MAX 256
#define OUTPUT_FILE "motion_data_output.csv"
#define SAMPLE_PERIOD 0.050
#define SAMPLES_PER_CYCLE 199
#define CYCLE_LENGTH 3
#define WINDOW_SIZE 30
#define FFT_SIZE (2 * WINDOW_SIZE)
#define NUM_LABELS 3
#define NUM_INPUTS 12

#define TRAIN_MODE 0
#define TEST_MODE 1
#endif
