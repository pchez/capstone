#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <liquid/liquid.h>
#include <unistd.h>
#include "main.h"
#include "prep_train_data.h"
#include "helpers.h"

void normalize(float* data) {
    *data = *data / 1024.0;
}

char* encode(int cycle_count, int nclasses) {
    char* buf = (char*) malloc(30);
    uint8_t* arr = (uint8_t*) calloc(nclasses, sizeof(uint8_t));
    arr[cycle_count] = 1;
    sprintf(buf, "%d %d %d %d %d %d", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
    return buf;
}

void make_train_file(unsigned int size, int cycle_count, int nclasses) {
	FILE * output;
    char* encoded_output;

    if (cycle_count == 0) {
        output = fopen(OUTPUT_FILE, "w");
        fprintf(output, "%d %d %d\n", SAMPLES_PER_CYCLE * nclasses, 3, nclasses);  
    }
    else {
        output = fopen(OUTPUT_FILE, "a");
    } 

	FILE * out_ax = fopen(SIGNAL_AX,"r");
    FILE * out_ay = fopen(SIGNAL_AY,"r");
    FILE * out_az = fopen(SIGNAL_AZ,"r");

    FILE * out_gx = fopen(SIGNAL_GX,"r");
    FILE * out_gy = fopen(SIGNAL_GY,"r");
    FILE * out_gz = fopen(SIGNAL_GZ,"r");

    FILE * out_mx = fopen(SIGNAL_MX,"r");
    FILE * out_my = fopen(SIGNAL_MY,"r");
    FILE * out_mz = fopen(SIGNAL_MZ,"r");

    unsigned int i;
    float ax,ay,az,gx,gy,gz,mx,my,mz;
    for(i=0; i<size; ++i) {

        fscanf(out_ax,"%f",&ax);
        fscanf(out_ay,"%f",&ay);
        fscanf(out_az,"%f",&az);

//        fscanf(out_gx,"%f",&gx);
//        fscanf(out_gy,"%f",&gy);
//        fscanf(out_gz,"%f",&gz);
//
//        fscanf(out_mx,"%f",&mx);
//        fscanf(out_my,"%f",&my);
//        fscanf(out_mz,"%f",&mz);
//
        normalize(&ax);
        normalize(&ay);
        normalize(&az);
//        normalize(&gx);
//        normalize(&gx);
//        normalize(&gy);
//        normalize(&mz);
//        normalize(&my);
//        normalize(&mz);
//  
        encoded_output = encode(cycle_count, nclasses);      
        fprintf(output, "%f %f %f\n",ax,ay,az);
        fprintf(output, "%s\n", encoded_output);
    }

    fclose(output);

    fclose(out_ax);
    fclose(out_ay);
    fclose(out_az);

    fclose(out_gx);
    fclose(out_gy);
    fclose(out_gz);

    fclose(out_mx);
    fclose(out_my);
    fclose(out_mz);
}
