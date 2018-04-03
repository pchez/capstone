//
// Feature extraction using Liquid DSP library
//
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

void filter(const char *signal, unsigned int n_samples, struct filter_options options) {
    	iirfilt_crcf q = iirfilt_crcf_create_prototype(options.ftype, options.btype, options.format, options.order, options.fc, options.f0, options.Ap, options.As);

    	//iirfilt_crcf_print(q);
    
    	float complex x[n_samples];
    	float complex y[n_samples];

    	FILE *fp = fopen(signal,"r");
    	if(fp == 0){
        	printf("INVALID FILE (filter)\n");
        return;
    	}
    
    	unsigned int i;
    	for (i=0; i<n_samples; i++){
        	float num;
        	fscanf(fp,"%f",&num);
        	x[i] = num;
        	iirfilt_crcf_execute(q, x[i], &y[i]);
    	}
    	fclose(fp);
    
    	iirfilt_crcf_destroy(q);
    
    	FILE *frewrite = fopen(signal,"w");
    	for(i=0; i < n_samples; ++i){
    		fprintf(frewrite, "%f\n", crealf(y[i]));
    	}
    	fclose(frewrite);
}

float findMax(float* buf, int buf_size, int* index) {
    int i;
    float max = -500000.0;
    int max_index = 0;
    for (i=0; i<buf_size; i++) {
        if (buf[i] > max) {
            max = buf[i];
            max_index = i;
        }
    }
    *index = max_index;
    return max;
}

void normalize_buf(float* buf, int buf_size) {
    int index = 0;
    float max;
    int i;
    for (i=0; i<buf_size; i++) {
        max = findMax(buf, buf_size, &index);
        if (max != 0) 
           buf[i] = buf[i] / max;
    }
}

void rms_comp(float* signal, unsigned int n_samples, float * t_start, float * t_stop, float * rms_signal) {

	//float x[n_samples];
	float num;
	unsigned int i;
	float sample_time;
	float sample_period;
	float signal_mean;
	int sample_count;

	sample_period = SAMPLE_PERIOD;

//	FILE *fp = fopen(signal,"r");
//
//        if(fp == 0){
//                printf("INVALID FILE (filter)\n");
//                return;
//        }

	sample_time = 0;
	signal_mean = 0;
	sample_count = 0;
	//
	// Compute mean signal
	//
    for (i=0; i<n_samples; i++) {
        sample_time = sample_time + sample_period;
        if (sample_time > *t_start && sample_time < *t_stop) {
            //fscanf(fp,"%f",&num);
            //x[sample_count] = num;
            signal_mean = signal_mean + signal[sample_count];
            sample_count++;
        }
	}

	sample_count--;

	if(sample_count > 0){
		signal_mean = signal_mean / sample_count;
	}

	//
	// Compute rms of zero mean signal within time window
	// 

	*rms_signal = 0;
    for (i=0; i<sample_count; i++) {
		signal[i] = signal[i] - signal_mean;
		*rms_signal = *rms_signal + signal[i]*signal[i];
    }

	*rms_signal = (float)(*rms_signal)/sample_count;
	*rms_signal = (float)sqrt(*rms_signal);
    //fclose(fp);
}

void fft_comp(float* orig_buf, float complex* fft_buf, int window_size, int fft_size) {
    int i; 
    for (i=0; i<FFT_SIZE; i++) {
        fft_buf[i] = (float)i - _Complex_I * (float)i;
    }

    fftplan pf = fft_create_plan(FFT_SIZE, (float complex*) orig_buf, fft_buf, LIQUID_FFT_FORWARD, 0); 
    
    //fft_print_plan(pf); // print fft plans
   
    fft_execute(pf); //execute fft plans

    fft_destroy_plan(pf); //destroy fft plans

}

float getFreq(float complex* fft_buf, int fft_size) {
    float max = -5.0;
    int max_index = 0;
    float abs_fft = 0.0;
    int i;    
    for (i=0; i<fft_size; i++) {
        abs_fft = sqrt(pow(crealf(fft_buf[i]),2) + pow(cimagf(fft_buf[i]),2));
        if (abs_fft > max) {
            max = abs_fft;
            max_index = i;
        }
    }
    float freq = (float)max_index * (1.0/SAMPLE_PERIOD) / (float)FFT_SIZE;
    printf("index of max %d\n", max_index);
    printf("frequency %f\n", freq);

    return freq; 
}
