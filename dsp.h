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
void fft_comp(float* orig_buf, float complex* fft_buf, int window_size, int fft_size);
float getFreq(float complex* fft_buf, int fft_size);
