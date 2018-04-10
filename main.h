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
#define RAW_INPUT_FILE "motion_data.dat"
#define ORIG_TRAIN_FILE "train_orig.csv"
#define NEW_TRAIN_FILE "train_new.csv"
#define ORIG_FANN_FILE "fann_motion_orig.net"
#define NEW_FANN_FILE "fann_motion_new.net"

#define SAMPLE_PERIOD 0.050
#define SAMPLES_PER_CYCLE 199
#define CYCLE_LENGTH 3
#define WINDOW_SIZE 30
#define HISTORY_SIZE 5
#define FFT_SIZE (2 * WINDOW_SIZE)

#define NUM_CLASSES 4
#define NUM_INPUTS 12
#define NUM_SENSORS 1
#define NUM_CYCLES 10

#define MSE_THRESHOLD 0.001
#define FREQ_DIFF_THRESHOLD 0.3
#define STDDEV_THRESHOLD 1.0

#define DETECT_NEW_GESTURES_ON 1
#define RUN_MODE 0x01
#define PROMPT_MODE 0x02
#define TRAIN_MODE 0x04

#endif
