#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <liquid/liquid.h>
#include <unistd.h>
#include "main.h"
#include "preparetrain.h"
#include "helpers.h"
#include "../fann/src/include/fann.h"
#include "../fann/src/include/floatfann.h"


void encode(char* buf, int class_index, int num_classes) {
    int i;
    strcpy(buf, "");
    for (i=0; i<num_classes; i++) {
        if (i != 0) {
            strcat(buf, " ");
        }
        if (i == class_index) {
            strcat(buf, "1");
        }
        else {
            strcat(buf, "0");
        }
    }
}

void buf_to_string(char* buf, float* input, int num_inputs) {
    int i;
    char float_buf[30];
    strcpy(float_buf, "");
    strcpy(buf, "");
    for (i=0; i<num_inputs; i++) {
        if (i != 0) {
            strcat(buf, " ");
        }
        sprintf(float_buf, "%f", input[i]);
        strcat(buf, float_buf);
    }
}

void update_train_file(FILE* train_file, float* input, int class_index, int num_classes) {
    char encoded_output[80];
    char buf[512];
    
    encode(encoded_output, class_index, num_classes);      
    buf_to_string(buf, input, NUM_INPUTS);
    
    fprintf(train_file, "%s\n", buf);
    fprintf(train_file, "%s\n", encoded_output);
   
}

// returns current number of inputs
int prepare_train_file(char* curr_train_file, int num_classes) {
    FILE* old_train_file = fopen(curr_train_file, "r");
    char update_command[256];
    char raw[BUFF_MAX];

    // read existing number of samples from first token
    fgets(raw, BUFF_MAX, old_train_file);
    char* token = strtok(raw, " "); 
    int num_samples = atoi(token);
    
    // read existing number of classes from third token
    token = strtok(NULL, " "); 
    token = strtok(NULL, " "); 
    int old_num_classes = atoi(token);
    fclose(old_train_file);

    // if currently using original train file, make a copy
    if (strcmp(curr_train_file, ORIG_TRAIN_FILE) == 0) {
        sprintf(update_command, "cp %s %s", ORIG_TRAIN_FILE, NEW_TRAIN_FILE);
        system(update_command);
    }
    
    // replace first line with placeholders
    sprintf(update_command, "sed -i '1 s/%d/nsamples/' %s", num_samples, NEW_TRAIN_FILE);
    system(update_command);
    sprintf(update_command, "sed -i '1 s/%d/nclasses/' %s", old_num_classes, NEW_TRAIN_FILE);
    system(update_command);
    
    // append 0 to one-hot encoded output lines to prepare for additional class
    sprintf(update_command, "sed -i '3~2 s/$/ 0/' %s", NEW_TRAIN_FILE);
    system(update_command);
    
    return num_samples;
}

void train(struct fann* ann, char* train_file, char* output_net, int num_classes) {

	const unsigned int num_input = 12;
	const unsigned int num_output = num_classes;
	const unsigned int num_layers = 3;
	const unsigned int num_neurons_hidden = 10;
	const float desired_error = (const float) 0.001;
	const unsigned int max_epochs = 50000;
	const unsigned int epochs_between_reports = 1000;
	
    ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);

	fann_set_activation_function_hidden(ann, FANN_SIGMOID);
	fann_set_activation_function_output(ann, FANN_SIGMOID);
    fann_train_on_file(ann, train_file, max_epochs, epochs_between_reports, desired_error);

	fann_save(ann, output_net);
    fann_destroy(ann);
}

