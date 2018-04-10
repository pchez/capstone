#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <liquid/liquid.h>
#include <unistd.h>
#include "main.h"
#include "preprocessing.h"
#include "helpers.h"

void normalize(float* data) {
    *data = *data / 1024.0;
}

void encode(char* buf, int cycle_count, int num_classes) {
    int arr[num_classes];
    memset(arr, 0, num_classes * sizeof(int));
    arr[cycle_count] = 1;
    sprintf(buf, "%d %d %d %d", arr[0], arr[1], arr[2], arr[3]);
    return buf;
}

void compute_average(unsigned int size, float* outbuf) {

    FILE * out_ax = fopen(SIGNAL_AX,"r");
    FILE * out_ay = fopen(SIGNAL_AY,"r");
    FILE * out_az = fopen(SIGNAL_AZ,"r");

    unsigned int i;
    float ax,ay,az;
    float ax_avg = 0;
    float ay_avg = 0;
    float az_avg = 0;
    for(i=0; i<size; ++i) {
    
        fscanf(out_ax,"%f",&ax);
        fscanf(out_ay,"%f",&ay);
        fscanf(out_az,"%f",&az);
        
        ax_avg += ax;
        ay_avg += ay;
        az_avg += az;
    }

    ax_avg /= size;
    ay_avg /= size;
    az_avg /= size;
   
    normalize(&ax_avg);
    normalize(&ay_avg);
    normalize(&az_avg);

    outbuf[0] = ax_avg;
    outbuf[1] = ay_avg;
    outbuf[2] = az_avg;
    
    fclose(out_ax);
    fclose(out_ay);
    fclose(out_az);
}

void update_train_file(FILE* output_file, float* input, int cycle_count, int nclasses) {
    char* encoded_output = encode(cycle_count, nclasses);      
    fprintf(output_file, "%f %f %f %f %f %f %f %f %f %f %f %f\n",input[0],input[1],input[2],input[3],input[4],input[5],input[6],input[7],input[8],input[9],input[10],input[11]);
    fprintf(output_file, "%s\n", encoded_output);
    printf("%f %f %f %f %f %f %f %f %f %f %f %f\n",input[0],input[1],input[2],input[3],input[4],input[5],input[6],input[7],input[8],input[9],input[10],input[11]);
   
}
