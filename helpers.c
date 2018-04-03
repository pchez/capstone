//
//  Based on Acquire_LowPass_Continuous.c
//  Created by Priscilla Cheng on 3/26/2018
//
//  Helper functions for parsing and conversion


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <liquid/liquid.h>
#include <unistd.h>
#include "main.h"

int char_to_decimal(char letter) {

	switch(letter) {
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'a':
			return 10;
		case 'b':
			return 11;
		case 'c':
			return 12;
		case 'd':
			return 13;
		case 'e':
			return 14;
		case 'f':
			return 15;
		default:
			return -1;
	}
}

int hex_to_decimal_4bit(char seq[4]){  //, int twos_comp){
	int sum = char_to_decimal(seq[1]);
	sum += (char_to_decimal(seq[0])*16);
	sum += (char_to_decimal(seq[3])*16*16);
	sum += (char_to_decimal(seq[2])*16*16*16);

	if(sum > 32767) sum = sum - 65536; //twos_comp == 1 && 

	return sum;
}

int hex_to_decimal_time(char seq[4]) {
	int sum2 = char_to_decimal(seq[1]);
	sum2 += (char_to_decimal(seq[0])*16);
	int sum1 = char_to_decimal(seq[3]);
	sum1 += (char_to_decimal(seq[2])*16); 
	sum1 = (sum1 * 1000) + sum2;
	return sum1;
}

void initSensorsBuf(float*** sensors_buf, float complex*** fft_buf, int num_sensors) {
    // init buffer of pointers to sensor values
    // 0 1 2 accel; 3 4 5 gyro; 6 7 8 mag
    float** sbuf;
    float complex** fbuf;
    *sbuf = (float**) malloc(sizeof(float*) * num_sensors * 6);
    *fbuf = (float complex**) malloc(sizeof(float complex*) * num_sensors * 3);
    
    // allocate memory for all sensor buffers
    int i;
    for (i=0; i<num_sensors*3; i++) {
        *sbuf[i] = (float*) malloc(sizeof(float) * WINDOW_SIZE);
        *fbuf[i] = (float complex*) malloc(sizeof(float complex) * FFT_SIZE);
    }
    *sensors_buf = sbuf;
    *fft_buf = fbuf;
}

void clearSensorsBuf(float*** sensors_buf, float complex*** fft_buf, int num_sensors) {
    if (num_sensors >= 1) {
        memset(*sensors_buf[0], 0, WINDOW_SIZE * sizeof(float)); 
        memset(*sensors_buf[1], 0, WINDOW_SIZE * sizeof(float)); 
        memset(*sensors_buf[2], 0, WINDOW_SIZE * sizeof(float)); 
        memset(*fft_buf[0], 0, FFT_SIZE); 
        memset(*fft_buf[1], 0, FFT_SIZE); 
        memset(*fft_buf[2], 0, FFT_SIZE); 
    }
    if (num_sensors >= 2) {
        memset(*sensors_buf[3], 0, WINDOW_SIZE); 
        memset(*sensors_buf[4], 0, WINDOW_SIZE); 
        memset(*sensors_buf[5], 0, WINDOW_SIZE); 
        memset(*fft_buf[3], 0, FFT_SIZE); 
        memset(*fft_buf[4], 0, FFT_SIZE); 
        memset(*fft_buf[5], 0, FFT_SIZE); 
    }
    if (num_sensors >= 3) {
        memset(*sensors_buf[6], 0, WINDOW_SIZE); 
        memset(*sensors_buf[7], 0, WINDOW_SIZE); 
        memset(*sensors_buf[8], 0, WINDOW_SIZE); 
        memset(*fft_buf[6], 0, FFT_SIZE); 
        memset(*fft_buf[7], 0, FFT_SIZE); 
        memset(*fft_buf[8], 0, FFT_SIZE); 
    }
}

int stream_parser(char raw[BUFF_MAX], int num_sensors, float** sensors_buf, int index) {
 	int i = 0; 
	while(i < BUFF_MAX && raw[i] != ':'){ ++i; }
	i += 2;
	char *ptr = &raw[i];
	
	int iter = 0;
	char data[40]; 
	int lock = 0;
	while(iter < 40 && ptr != NULL && *ptr != '\0' && *ptr != '\n') {
		if(lock < 40 && *ptr != ' '){ 
			data[iter] = *ptr;
			++lock;
			++iter;
		}
        ptr++;
    }
    
    if (num_sensors >= 1) { // accel only
        sensors_buf[0][index] = (float)(hex_to_decimal_4bit(&data[0]));
        sensors_buf[1][index] = (float)(hex_to_decimal_4bit(&data[1]));
        sensors_buf[2][index] = (float)(hex_to_decimal_4bit(&data[2]));
    }
    if (num_sensors >= 2) { // gyro + accel
        sensors_buf[3][index] = (float)(hex_to_decimal_4bit(&data[3]));
        sensors_buf[4][index] = (float)(hex_to_decimal_4bit(&data[4]));
        sensors_buf[5][index] = (float)(hex_to_decimal_4bit(&data[5]));
    }
    if (num_sensors >= 3) { // gyro + accel + mag
        sensors_buf[6][index] = (float)(hex_to_decimal_4bit(&data[6]));
        sensors_buf[7][index] = (float)(hex_to_decimal_4bit(&data[7]));
        sensors_buf[8][index] = (float)(hex_to_decimal_4bit(&data[8]));
    }
     
}

int stream_to_file(char raw[BUFF_MAX]) {
	int i = 0; 
	while(i < BUFF_MAX && raw[i] != ':'){ ++i; }
	i += 2;
	char *ptr = &raw[i];
	
	int iter = 0;
	char data[40]; 
	int lock = 0;
	while(iter < 40 && ptr != NULL && *ptr != '\0' && *ptr != '\n') {
		if(lock < 40 && *ptr != ' '){ 
			data[iter] = *ptr;
			++lock;
			++iter;
		}
        ptr++;
    }

    FILE * out_ax = fopen(SIGNAL_AX,"a");
    FILE * out_ay = fopen(SIGNAL_AY,"a");
    FILE * out_az = fopen(SIGNAL_AZ,"a");

    FILE * out_gx = fopen(SIGNAL_GX,"a");
    FILE * out_gy = fopen(SIGNAL_GY,"a");
    FILE * out_gz = fopen(SIGNAL_GZ,"a");

    FILE * out_mx = fopen(SIGNAL_MX,"a");
    FILE * out_my = fopen(SIGNAL_MY,"a");
    FILE * out_mz = fopen(SIGNAL_MZ,"a");
    
    int y;
	int index;
   	float val;
    index = 0; 
    for(y = 0; y < 10; y++) {
		val = (float)(hex_to_decimal_4bit(&data[index]));

        switch(y) {
			case 0:
				//sensortile.timestamp = hex_to_decimal_time(&data[y]);
				break;
			case 1:
				fprintf(out_ax, "%f\n", val); 
				break;
			case 2:
				fprintf(out_ay, "%f\n", val); 
				break;
			case 3:
				fprintf(out_az, "%f\n", val); 
				break;
			case 4:
				fprintf(out_gx, "%f\n", val);
				break;
			case 5:
				fprintf(out_gy, "%f\n", val);
				break;
			case 6: 
				fprintf(out_gz, "%f\n", val);
				break;
			case 7:
				fprintf(out_mx, "%f\n", val);
				break;
			case 8:
				fprintf(out_my, "%f\n", val);
				break;
			case 9: 
				fprintf(out_mz, "%f\n", val);
				break;
			default:
				return 0;
    	}
	index = index + 4;
    }

    fclose(out_ax);
    fclose(out_ay);
    fclose(out_az);

    fclose(out_gx);
    fclose(out_gy);
    fclose(out_gz);

    fclose(out_mx);
    fclose(out_my);
    fclose(out_mz);

    return 1;
}      

unsigned int BLE_parse(const char *inFile, int mode, int num_sensors, float** sensors_buf) {
	FILE* ble_file;
	ble_file = fopen(inFile, "r");

	char raw[BUFF_MAX];

	// Only when training
	// Advance line over first line of file 
	// since first line of file may be concatenated 
	// or may contain header

	unsigned int iter = 0;
	if (mode == TRAIN_MODE) {
        fgets(raw, BUFF_MAX, ble_file);

	    // Read motion data
	    while(fgets(raw, BUFF_MAX, ble_file)){
		    if(stream_to_file(raw) == 0) return 0;
		    iter++;
	    }
    }
    else {
        // fill buffer until full or no more data avail
	    printf("before fgets\n");
        while(fgets(raw, BUFF_MAX, ble_file) && iter < WINDOW_SIZE){
		    if(stream_parser(raw, num_sensors, sensors_buf, iter) == 0) return 0;
		    printf("%d\n", iter);
            iter++;
	    }

    }
	//
	// Decrement iter to ensure that last line of file is eliminated
	// since last line may be concatenated in data transfer
	//
	
	return iter--;
}

void makeCSV(unsigned int size) {
	FILE *output = fopen(OUTPUT_FILE, "w");
	fprintf(output, "Accel_x,Accel_y,Accel_z,Gyro_x,Gyro_y,Gyro_z,Magneto_x,Magneto_y,Magneto_z\n");

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

        fscanf(out_gx,"%f",&gx);
        fscanf(out_gy,"%f",&gy);
        fscanf(out_gz,"%f",&gz);

        fscanf(out_mx,"%f",&mx);
        fscanf(out_my,"%f",&my);
        fscanf(out_mz,"%f",&mz);

        fprintf(output, "%f,%f,%f,%f,%f,%f,%f,%f,%f\n",ax,ay,az,gx,gy,gz,mx,my,mz);
    
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

void cleanup() {

	if (remove(SIGNAL_AX) != 0)
      		printf("Unable to delete tempfile_ax.txt");
 	if (remove(SIGNAL_AY) != 0)
    		printf("Unable to delete tempfile_ay.txt");
    	if (remove(SIGNAL_AZ) != 0)
    		printf("Unable to delete tempfile_az.txt");
    
    	if (remove(SIGNAL_GX) != 0)
      		printf("Unable to delete tempfile_gx.txt");
 	if (remove(SIGNAL_GY) != 0)
    		printf("Unable to delete tempfile_gy.txt");
    	if (remove(SIGNAL_GZ) != 0)
    		printf("Unable to delete tempfile_gz.txt");
    
    	if (remove(SIGNAL_MX) != 0)
      		printf("Unable to delete tempfile_mx.txt");
 	if (remove(SIGNAL_MY) != 0)
    		printf("Unable to delete tempfile_my.txt");
    	if (remove(SIGNAL_MZ) != 0)
    		printf("Unable to delete tempfile_mz.txt");
}


