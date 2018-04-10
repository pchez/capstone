#ifndef PREPARE_TRAIN_H
#define PREPARE_TRAIN_H

#include "../fann/src/include/fann.h"
#include "../fann/src/include/floatfann.h"

void encode(char* buf, int class_index, int num_classes);
void buf_to_string(char* buf, float* input, int num_inputs);
void update_train_file(FILE* train_file, float* input, int class_index, int num_classes);
int prepare_train_file(char* curr_train_file, int num_classes);
void train(struct fann* ann, char* train_file, char* output_net, int num_classes); 

#endif
