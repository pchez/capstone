#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include "fann/src/include/fann.h"
#include "fann/src/include/floatfann.h"

int main() {
    fann_type *calc_out;
    fann_type input[6];
    struct fann *ann;
    struct fann_train_data *data;
    int i;

    printf("Creating network\n");
    ann = fann_create_from_file("fann_output.net");
    if (!ann) {
        printf("Failed to create network\n");
        exit(1);
    }
    
    input[0] = -9;
    input[1] = -48;
    input[2] = 1012;
    input[3] = 11;
    input[4] = -13;
    input[5] = 11;

    calc_out = fann_run(ann, input);
    printf("result: %f\n", calc_out[0]);
    fann_destroy(ann);
    return 0;
}
