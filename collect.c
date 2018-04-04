// Created by Priscilla Cheng 3/26/2018
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <liquid/liquid.h>
#include <unistd.h>
#include "main.h"
#include "dsp.h"
#include "helpers.h"

char SIGNAL_AX[] = "tempfile_ax.txt";
char SIGNAL_AY[] = "tempfile_ay.txt";
char SIGNAL_AZ[] = "tempfile_az.txt";
char SIGNAL_GX[] = "tempfile_gx.txt";
char SIGNAL_GY[] = "tempfile_gy.txt";
char SIGNAL_GZ[] = "tempfile_gz.txt";
char SIGNAL_MX[] = "tempfile_mx.txt";
char SIGNAL_MY[] = "tempfile_my.txt";
char SIGNAL_MZ[] = "tempfile_mz.txt";


int main(int argc, char *argv[]) {	

	float norm_cutoff;
	float norm_centerf;
	float t_start, t_stop;
	float rms_signal;
	unsigned int size;
	char * input_file = "motion_data.dat";
	int n_cycles;
	int cycle_count;

	//
	// Following input request applies to bandpass filter implementation requiring 
	// specification of center frequency
	//
	
    	// if(argc != 6 ){
    	// printf("Please provide cutoff frequency, center frequency, lower time limit and upper time limit for RMS computation and number of measurement cycles.\n");
	//	 return 0;
    	// }

	// norm_cutoff = atof(argv[1]);
	// norm_centerf = atof(argv[2]);
	// t_start = atof(argv[3]);
	// t_stop = atof(argv[4]);
	// n_cycles = atoi(argv[5]);


	//
	// Following input request applies to low pass filter implementation requiring 
	// specification of center frequency
	//	
					
	if(argc != 3 ){
		printf("Please provide cutoff frequency and number of measurement cycles\n");
		return 0;
		}
		
	norm_cutoff = atof(argv[1]);
	norm_centerf = 0.0;
	n_cycles = atoi(argv[2]);

	//
	// Acquire 10 second data stream at 20 Hz sampling rate
	//
	// Note: motion_data.sh must be executing with filename argument 
	// of sensor_data_stream.dat
	//
	//

	cycle_count = 0;

	while(cycle_count < n_cycles ) {

	cycle_count++;

	printf("Acquire 10 second sample in cycle %i of %i \n", cycle_count, n_cycles);

	system("sh motion_data.sh -t 10 -f sensor_data_stream.dat");

	system("tail -n 199 sensor_data_stream.dat > motion_data.dat");

	size = BLE_parse(input_file, TRAIN_MODE, 1, NULL);
	if(size == 0){
			printf("ERROR (stream_parser): BLE Data formatted incorrectly.\n");
		    	return 0;
    	}

	printf(" Number of samples acquired =  %i \n", size);


    	// Filter Documentation:
    	// http://liquidsdr.org/doc/iirdes/

    	struct filter_options ellip;
    	ellip.order =   4;       // filter order
    	ellip.fc    =   norm_cutoff;    // cutoff frequency
    	ellip.f0    =   norm_centerf;    // center frequency
    	ellip.Ap    =   3.0f;    // pass-band ripple
    	ellip.As    =   60.0f;   // stop-band attenuation
    	ellip.ftype  = LIQUID_IIRDES_ELLIP;
    	ellip.btype  = LIQUID_IIRDES_LOWPASS;
    	ellip.format = LIQUID_IIRDES_SOS;

    	filter(SIGNAL_AX,size,ellip);
    	filter(SIGNAL_AY,size,ellip);
    	filter(SIGNAL_AZ,size,ellip);

	t_start = SAMPLE_PERIOD;;
	t_stop  = size*SAMPLE_PERIOD;
	
    //rms_comp(SIGNAL_AX,size,&t_start, &t_stop, &rms_signal);

	printf(" RMS signal amplitude over time window t_start %f to t_step %f = %f\n", t_start, t_stop, rms_signal);

	makeCSV(size);

	printf(" Filtered motion data for cycle written to output data file\n");

	cleanup();

	}

    	return 0;
}
