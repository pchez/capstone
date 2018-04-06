#ifndef PREP_TRAIN_DATA_H
#define PREP_TRAIN_DATA_H

void normalize(float* data);
void compute_average(unsigned int size, float* outbuf);
void update_train_file(FILE* output_file, float* input, int cycle_count, int ncycles);

#endif
