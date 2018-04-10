#ifndef PREP_TRAIN_DATA_H
#define PREP_TRAIN_DATA_H

void encode(char* buf, int class_index, int num_classes);
void buf_to_string(char* buf, float* input, int num_inputs);
void update_train_file(FILE* train_file, float* input, int class_index, int num_classes);
int prepare_train_file(char* curr_train_file, int num_classes);

#endif
