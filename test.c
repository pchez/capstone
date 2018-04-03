// Created by Priscilla Cheng 3/26/2018
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <liquid/liquid.h>
#include <unistd.h>
#include "main.h"
#include "dsp.h"
#include "helpers.h"
#include "preprocessing.h"
#include "../fann/src/include/fann.h"
#include "../fann/src/include/floatfann.h"

char SIGNAL_AX[] = "tempfile_ax.txt";
char SIGNAL_AY[] = "tempfile_ay.txt";
char SIGNAL_AZ[] = "tempfile_az.txt";
char SIGNAL_GX[] = "tempfile_gx.txt";
char SIGNAL_GY[] = "tempfile_gy.txt";
char SIGNAL_GZ[] = "tempfile_gz.txt";
char SIGNAL_MX[] = "tempfile_mx.txt";
char SIGNAL_MY[] = "tempfile_my.txt";
char SIGNAL_MZ[] = "tempfile_mz.txt";

static volatile int run = 1;
pid_t child_pid, parent_pid;
uint8_t data_ready = 0;

void exitHandler(char* err_string, int exit_code) {
    fprintf(stderr, "exitHandler: %s", err_string);
    kill(child_pid, SIGTERM);        
    exit(exit_code);
}


void sigHandler(int signum) {
    if (signum == SIGUSR1) {
        data_ready = 1;
    }
    if (signum == SIGINT) {
        exitHandler("Caught SIGINT, exiting child processes\n", 0);    
    }
}

int main(int argc, char *argv[]) {	
    // FANN
    fann_type *calc_out;
    fann_type input[NUM_INPUTS];
    struct fann *ann;
    struct fann_train_data *data;
	
    // DSP
    float norm_cutoff;
	float norm_centerf;
	float t_start, t_stop;
	float rms_signal[3];
	unsigned int size;
	int method = 0;

    // main
    char * input_file = "motion_data.dat";
	float** sensors_buf;
    float complex** fft_buf;
    int n_cycles;
	int cycle_count;
    int num_sensors = 1;
    char call_shell_script[128];
    char call_tail[128];
    float max;
    int i;

    // IPC
    int sigwait_result;
    int sig;
    int nbytes, result;
    sigset_t signal_set;
    char ppid_string[8];
    int pipefd[2];
    short revent[1];
    
    // Set up signal set and signal handling
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &signal_set, NULL);
    signal(SIGSEGV, sigHandler);
    signal(SIGINT, sigHandler);

    // Initialize pipe and polling between parent and child
    pipe(pipefd);
    struct pollfd fds[] = {
        {pipefd[0], POLLIN, revent[1]}
    };

    ann = fann_create_from_file("fann_output.net");
    if (!ann) {
        printf("Failed to create network\n");
        exit(1);
    }

    // Initiate data collection from SensorTile via bluetooth command in shell script
    child_pid = fork();
    if (child_pid == -1) {
        exitHandler("Fork error", -1);
    }
    if (child_pid == 0) { 
        parent_pid = (long) getppid();
        
        sprintf(ppid_string, "%ld", parent_pid);
        //printf("parent_pid: %s\n", ppid_string);
        printf("Data collection process started with PID: %ld\n", (long) getpid());
        
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        execlp("./collect_data.sh", "./collect_data.sh", "-t", "1", "-p", ppid_string, (char*) NULL); //, fann_input_name);
        do {
            exitHandler("execlp error", -2);
        } while(0);
    
    }
    else {
        close(pipefd[1]);
        
        // initialize buffer to hold sensor values
        initSensorsBuf(&sensors_buf, &fft_buf, num_sensors);

        while (run) {
            printf("Collect data...\n");            
            kill(child_pid, SIGUSR1);   
            sigwait_result = sigwait(&signal_set, &sig);

            printf("before clear buf\n");
            clearSensorsBuf(&sensors_buf, &fft_buf, num_sensors);
            printf("before ble parse\n");
            size = BLE_parse(input_file, TEST_MODE, num_sensors, sensors_buf);
            printf("after ble parse\n");
            if(size == 0){
                printf("ERROR (stream_parser): BLE Data formatted incorrectly.\n");
            }
            else {
                // compute avg
                compute_average(size, input);
                
                // compute rms
                t_start = SAMPLE_PERIOD;
                t_stop = size * SAMPLE_PERIOD;
                rms_comp(SIGNAL_AX, size, &t_start, &t_stop, &input[0]);
                rms_comp(SIGNAL_AY, size, &t_start, &t_stop, &input[1]);
                rms_comp(SIGNAL_AZ, size, &t_start, &t_stop, &input[2]);

                // compute fft
                fft_comp(sensors_buf[0], fft_buf[0], WINDOW_SIZE, FFT_SIZE);
                fft_comp(sensors_buf[1], fft_buf[1], WINDOW_SIZE, FFT_SIZE);
                fft_comp(sensors_buf[2], fft_buf[2], WINDOW_SIZE, FFT_SIZE);
                // get freq from fft
                input[3] = getFreq(fft_buf[0], FFT_SIZE);
                input[4] = getFreq(fft_buf[1], FFT_SIZE);
                input[5] = getFreq(fft_buf[2], FFT_SIZE);

                calc_out = fann_run(ann, input);
                printf("%f %f %f %f %f %f\n", input[0], input[1], input[2], input[3], input[4], input[5]);

                max = -5.0;
                result = 0;
                for (i=0; i<NUM_LABELS; i++) {
                    if (calc_out[i] > max) {
                        max = calc_out[i];
                        result = i;
                    }
                }    
                printf("result: %d\n", result);
                printf("result: %f %f %f\n", calc_out[0], calc_out[1], calc_out[2]);
                        
            }

            cleanup();
        }

    }

    fann_destroy(ann);
    system("pkill gatttool");

    return 0;
}
