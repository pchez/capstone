#ifndef HELPERS_H
#define HELPERS_H

int char_to_decimal(char letter);
int hex_to_decimal_4bit(char seq[4]);
int hex_to_decimal_time(char seq[4]);
void initSensorsBuf(float*** sensors_buf, float complex*** fft_buf, float*** freq_buf, float** results_buf, int num_sensors);
void clearSensorsBuf(float** sensors_buf, float complex** fft_buf, int num_sensors);
int stream_to_file(char raw[BUFF_MAX]);
int stream_parser(char raw[BUFF_MAX], int num_sensors, float** sensors_buf, int index);
unsigned int BLE_parse(const char *inFile, int mode, int num_sensors, float** sensors_buf);
void get_all_features(float** sensors_buf, float complex** fft_buf, float* input, int num_sensors, float* t_start, float* t_stop);
int detect_new_gesture(float** freq_buf, float* results_buf, int this_frame_index, float mse_threshold);
void cleanup();

#endif
