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
    max = findMax(buf, buf_size, &index);
    for (i=0; i<buf_size; i++) {
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
    float signal_val;
    for (i=0; i<sample_count; i++) {
		signal_val = signal[i] - signal_mean;
		*rms_signal = *rms_signal + signal_val*signal_val;
    }

	*rms_signal = (float)(*rms_signal)/sample_count;
	*rms_signal = (float)sqrt(*rms_signal);
    //fclose(fp);
}

float compute_mean(float* sensors_buf, int size) {
    int i;
    float mean = 0.0;
    for (i=0; i<size; i++) {
        mean += sensors_buf[i];
    }
    return mean / (float)size;
}

void remove_dc(float* sensors_buf) {
    int i;
    float mean = compute_mean(sensors_buf, WINDOW_SIZE);
    for (i=0; i<WINDOW_SIZE; i++) {
        sensors_buf[i] = sensors_buf[i] - mean;    
    }
}

void fft_comp(float* orig_buf, float complex* fft_buf, int window_size, int fft_size) {
    int i; 
    float complex * fc_buf = (float complex*) malloc(FFT_SIZE*sizeof(float complex));
    memset(fc_buf, 0, FFT_SIZE * sizeof(float complex));

    for (i=0; i<window_size; i++) {
        fc_buf[i] = orig_buf[i];
    }
    fftplan pf = fft_create_plan(FFT_SIZE, fc_buf, fft_buf, LIQUID_FFT_FORWARD, 0); 
    
    //fft_print_plan(pf); // print fft plans
   
    fft_execute(pf); //execute fft plans

    // scale/normalize fft
    for (i=0; i<fft_size; i++) {
        fft_buf[i] = fft_buf[i] / (float)FFT_SIZE;
    }
    fft_destroy_plan(pf); //destroy fft plans

}

float get_freq(float complex* fft_buf, int fft_size) {
    float max = -5.0;
    int max_index = 0;
    float abs_fft = 0.0;
    int i;   

    // only need to search half of the fft bc symmetric 
    for (i=0; i<fft_size/2; i++) {
   //     abs_fft = sqrt(pow(crealf(fft_buf[i]),2) + pow(cimagf(fft_buf[i]),2));
        abs_fft = cabsf(fft_buf[i]);
        if (abs_fft > max) {
            max = abs_fft;
            max_index = i;
        }
    }
    float freq = (float)max_index * (1.0/SAMPLE_PERIOD) / (float)FFT_SIZE;

    return freq; 
}

float compute_std(float* buf, int size) {
    int i;
    float std = 0.0;
    float mean_buf = compute_mean(buf, size);
    for (i=0; i<size; i++) {
        std += (buf[i] - mean_buf) * (buf[i] - mean_buf);
    }
    std = sqrt(std / (float)size);
    return std;
}

float compute_corr(float* a, float* b) {
    //cov(a,b)/(std(a)*std(b))
    //compute covariance
    float ab[WINDOW_SIZE];
    int i;
    
    for (i=0; i<WINDOW_SIZE; i++) {
        ab[i] = a[i] * b[i];
    }

    float mean_a = compute_mean(a, WINDOW_SIZE);
    float mean_b = compute_mean(b, WINDOW_SIZE);
    float mean_ab = compute_mean(ab, WINDOW_SIZE);
    float cov = mean_ab - mean_a*mean_b;
    
    //compute standard deviation
    float std_a = compute_std(a, WINDOW_SIZE);
    float std_b = compute_std(b, WINDOW_SIZE);
    
    //prevent division by zero
    if (std_a == 0 || std_b == 0) {
        return 0.0;
    }
    return cov / (std_a * std_b);
}

float compute_energy(float complex* fft_buf) {
    int i;
    float energy = 0;
    for (i=0; i<WINDOW_SIZE; i++) {
        energy += pow(cabsf(fft_buf[i]), 2);         
    }
    return energy / 10000.0;
}

