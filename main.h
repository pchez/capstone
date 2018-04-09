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
#define TRAIN_FILE "motion_data_output.csv"
#define NEW_TRAIN_FILE "train_motion.csv"
#define SAMPLE_PERIOD 0.050
#define SAMPLES_PER_CYCLE 199
#define CYCLE_LENGTH 3
#define WINDOW_SIZE 30
#define FFT_SIZE (2 * WINDOW_SIZE)
#define NUM_CLASSES 4
#define NUM_INPUTS 12
#define NUM_SENSORS 1
#define NUM_CYCLES 10
#define RUN_MODE 0x01
#define PROMPT_MODE 0x02
#define TRAIN_MODE 0x04

#endif
