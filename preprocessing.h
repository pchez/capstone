#ifndef PREP_TRAIN_DATA_H
#define PREP_TRAIN_DATA_H

void normalize(float* data);
void make_train_file(unsigned int size, int cycle_count, int ncycles);

#endif
