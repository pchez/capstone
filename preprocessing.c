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

char* encode(int cycle_count, int nclasses) {
    char* buf = (char*) malloc(30);
    int8_t* arr = (int8_t*) malloc(nclasses * sizeof(int8_t));
    memset(arr, 0, nclasses * sizeof(int8_t));
    arr[cycle_count] = 1;
    sprintf(buf, "%d %d %d %d %d %d", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
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

void make_train_file(unsigned int size, int cycle_count, int nclasses) {
	FILE * output;
    char* encoded_output;

    // Write header to first line
    if (cycle_count == 0) {
        output = fopen(OUTPUT_FILE, "w");
        fprintf(output, "%s %d %d\n", "placeholder", 3, nclasses);  
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