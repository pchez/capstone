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
	float rms_signal[3];
	unsigned int size, i;
	char * input_file = "motion_data.dat";
	int n_cycles, cycle_count, class_index; 
    int total_num_samples = 0;
    char call_shell_script[128];
    char call_tail[128];
    char buf[128];
    float** sensors_buf;
    float** freq_buf;
    float* results_buf;
    float complex** fft_buf;
    float input[NUM_INPUTS];
    FILE* train_file;

    //

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

    initSensorsBuf(&sensors_buf, &fft_buf, &freq_buf, &results_buf, NUM_SENSORS);

    // Write header to first line
    train_file = fopen(TRAIN_FILE, "w");
    fprintf(train_file, "%s %d %d\n", "placeholder", NUM_INPUTS, NUM_CLASSES);  

    for (class_index=0; class_index<NUM_CLASSES; class_index++) {    
        printf("Collecting data for class %d of %d. Press ENTER when ready.\n", class_index+1, NUM_CLASSES);
        getchar();
        
        for (cycle_count=0; cycle_count<n_cycles; cycle_count++) {    

            clearSensorsBuf(sensors_buf, fft_buf, NUM_SENSORS);

            printf("Cycle %i of %i \n", cycle_count + 1, n_cycles);

            system(call_shell_script);
            system(call_tail);

            size = BLE_parse(input_file, RUN_MODE, NUM_SENSORS, sensors_buf);
        
            if(size == 0){
                printf("ERROR (stream_parser): BLE Data formatted incorrectly.\n");
                return 0;
            }
            
            printf(" Number of samples acquired =  %i \n", size);
            total_num_samples += 1;
        
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

            //filter(SIGNAL_AX,size,ellip);
            //filter(SIGNAL_AY,size,ellip);
            //filter(SIGNAL_AZ,size,ellip);

            t_start = SAMPLE_PERIOD;;
            t_stop  = size*SAMPLE_PERIOD;
            
            get_all_features(sensors_buf, fft_buf, input, NUM_SENSORS, &t_start, &t_stop);
            update_train_file(train_file, input, class_index, NUM_CLASSES);
        }
    }
    fclose(train_file);
    
    // Modify train file with number of samples actually collected
    sprintf(buf, "sed -i 's/placeholder/%d/g' %s", total_num_samples, TRAIN_FILE);
    system(buf);

    return 0;
}
