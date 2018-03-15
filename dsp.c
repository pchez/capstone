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


void rms_comp(const char *signal, unsigned int n_samples, float * t_start, float * t_stop, float * rms_signal) {

	float x[n_samples];
	float num;
	unsigned int i;
	float sample_time;
	float sample_period;
	float signal_mean;
	int sample_count;

	sample_period = SAMPLE_PERIOD;

	FILE *fp = fopen(signal,"r");

        if(fp == 0){
                printf("INVALID FILE (filter)\n");
                return;
        }

	sample_time = 0;
	signal_mean = 0;
	sample_count = 0;
	//
	// Compute mean signal
	//
        for (i=0; i<n_samples; i++){
                sample_time = sample_time + sample_period;
		if (sample_time > * t_start && sample_time < * t_stop){
			fscanf(fp,"%f",&num);
                	x[sample_count] = num;
			signal_mean = signal_mean + x[sample_count];
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
        for (i=0; i<sample_count; i++){
		x[i] = x[i] - signal_mean;
		*rms_signal = *rms_signal + x[i]*x[i];
        }

	*rms_signal = (float)(*rms_signal)/sample_count;
	*rms_signal = (float)sqrt(*rms_signal);
        fclose(fp);
}


