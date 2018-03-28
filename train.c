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
#include "preprocessing.h"

char SIGNAL_AX[] = "tempfile_ax.txt";
char SIGNAL_AY[] = "tempfile_ay.txt";
char SIGNAL_AZ[] = "tempfile_az.txt";
char SIGNAL_GX[] = "tempfile_gx.txt";
char SIGNAL_GY[] = "tempfile_gy.txt";
char SIGNAL_GZ[] = "tempfile_gz.txt";
char SIGNAL_MX[] = "tempfile_mx.txt";
char SIGNAL_MY[] = "tempfile_my.txt";
char SIGNAL_MZ[] = "tempfile_mz.txt";

char* msg0 = "Place sensor upright. Press ENTER when ready.";
char* msg1 = "Place sensor face down. Press ENTER when ready.";
char* msg2 = "Tilt sensor 90 degrees upwards. Press ENTER when ready.";
char* msg3 = "Tilt sensor 90 degrees downwards. Press ENTER when ready.";
char* msg4 = "Tilt sensor 90 degrees to the right. Press ENTER when ready.";
char* msg5 = "Tilt sensor 90 degrees to the left. Press ENTER when ready.";

void print_instructions(unsigned int cycle_count) {
    char* msg;
    switch(cycle_count) {
        case 0:
            msg = msg0;
            break;
        case 1:
            msg = msg1;
            break;
        case 2:
            msg = msg2;
            break;
        case 3:
            msg = msg3;
            break;
        case 4:
            msg = msg4;
            break;
        case 5:
            msg = msg5;
            break;
        default:
            break;
    }
    printf("%s\n", msg);
}

int main(int argc, char *argv[]) {	

	float norm_cutoff;
	float norm_centerf;
	float t_start, t_stop;
	float rms_signal;
	unsigned int size;
	char * input_file = "motion_data.dat";
	int n_cycles, cycle_count; 
    int total_num_samples = 0;
    char call_shell_script[128];
    char call_tail[128];
    char buf[128];

	if(argc != 3){
		printf("Please provide cutoff frequency, number of measurement cycles, and cycle duration\n");
		return 0;
		}
		
	norm_cutoff = atof(argv[1]);
	norm_centerf = 0.0;
	n_cycles = atoi(argv[2]);

	//
	// Acquire 5 second data stream at 20 Hz sampling rate
	//
	// Note: motion_data.sh must be executing with filename argument 
	// of sensor_data_stream.dat
	//
	//
    sprintf(call_shell_script, "sh motion_data.sh -t %d -f sensor_data_stream.dat", CYCLE_LENGTH);
    sprintf(call_tail, "tail -n %d sensor_data_stream.dat > motion_data.dat", SAMPLES_PER_CYCLE);

	cycle_count = 0;

	while (cycle_count < n_cycles ) {


        printf("Cycle %i of %i \n", cycle_count + 1, n_cycles);
        print_instructions(cycle_count);
        getchar();

        system(call_shell_script);
        system(call_tail);

        size = BLE_parse(input_file, TRAIN_MODE);
        if(size == 0){
            printf("ERROR (stream_parser): BLE Data formatted incorrectly.\n");
            return 0;
        }

        printf(" Number of samples acquired =  %i \n", size);
        total_num_samples += size;
        
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
        
        rms_comp(SIGNAL_AX,size,&t_start, &t_stop, &rms_signal);

        printf(" RMS signal amplitude over time window t_start %f to t_step %f = %f\n", t_start, t_stop, rms_signal);

        make_train_file(size, cycle_count, n_cycles);
        
        printf(" Filtered motion data for cycle written to output data file\n");
        
        cycle_count++;
        
        cleanup();

	}
    // Modify train file with number of samples actually collected
    sprintf(buf, "sed -i 's/placeholder/%d/g' motion_data_output.csv", total_num_samples);
    system(buf);

    return 0;
}
