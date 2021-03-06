struct filter_options {
	// options
    	unsigned int order; 					//=   4;       // filter order
    	float        fc;    					//=   0.1f;    // cutoff frequency
    	float        f0;    					//=   0.0f;    // center frequency
    	float        Ap;    					//=   1.0f;    // pass-band ripple
    	float        As;    					//=   40.0f;   // stop-band attenuation
    	liquid_iirdes_filtertype ftype;  		//= LIQUID_IIRDES_ELLIP;
    	liquid_iirdes_bandtype   btype; 		//= LIQUID_IIRDES_LOWPASS;
    	liquid_iirdes_format     format; 		//= LIQUID_IIRDES_SOS;
};

void filter(const char *signal, unsigned int n_samples, struct filter_options options);
float findMax(float* buf, int buf_size, int* index);
void normalize_buf(float* buf, int buf_size);
void rms_comp(float *signal, unsigned int n_samples, float * t_start, float * t_stop, float * rms_signal);
float compute_mean(float* sensors_buf, int size);
void remove_dc(float* sensors_buf);
void fft_comp(float* orig_buf, float complex* fft_buf, int window_size, int fft_size);
float get_freq(float complex* fft_buf, int fft_size);
float compute_std(float* buf, int size);
float compute_corr(float* a, float* b);
float compute_energy(float complex* fft_buf);
