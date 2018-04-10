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
    if (signum == SIGSEGV) {
        exitHandler("Segmentation fault, exiting\n", -2);
    }
}

void train(struct fann* ann, char* train_file, char* output_net, int num_classes) {

	const unsigned int num_input = 12;
	const unsigned int num_output = num_classes;
	const unsigned int num_layers = 3;
	const unsigned int num_neurons_hidden = 10;
	const float desired_error = (const float) 0.001;
	const unsigned int max_epochs = 50000;
	const unsigned int epochs_between_reports = 1000;
	
    ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);

	fann_set_activation_function_hidden(ann, FANN_SIGMOID);
	fann_set_activation_function_output(ann, FANN_SIGMOID);
    fann_train_on_file(ann, train_file, max_epochs, epochs_between_reports, desired_error);

	fann_save(ann, output_net);

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
    char* input_file = RAW_INPUT_FILE;
	FILE* curr_train_file;
	char* curr_train_filename = ORIG_TRAIN_FILE;
    float** sensors_buf;
    float** freq_buf;
    float* results_buf;
    float complex** fft_buf;
	int cycle_count = 0;
    int num_classes = NUM_CLASSES;
    float mse_threshold = MSE_THRESHOLD;
    int num_sensors = 1;
    char call_shell_script[128];
    char call_tail[128];
    char cmd_buf[128];
    float max;
    int i;
    uint8_t mode = RUN_MODE;
    int new_gesture = 0;
    int hist_buf_full = 0;
    int hist_buf_index = 0;
    int new_gesture_detected = 0;
    int total_num_samples = 0;

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

    ann = fann_create_from_file(ORIG_FANN_FILE);
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

        execlp("./collect_data.sh", "./collect_data.sh", "-t", "2", "-p", ppid_string, (char*) NULL); //, fann_input_name);
        do {
            exitHandler("execlp error", -2);
        } while(0);
    
    }
    else {
        close(pipefd[1]);
        
        // initialize buffer to hold sensor values
        initSensorsBuf(&sensors_buf, &fft_buf, &freq_buf, &results_buf, num_sensors);

        while (run) {
            kill(child_pid, SIGUSR1);   
            sigwait_result = sigwait(&signal_set, &sig);
            
            clearSensorsBuf(sensors_buf, fft_buf, num_sensors);
           
            size = BLE_parse(input_file, RUN_MODE, num_sensors, sensors_buf);
            
            
            if(size == 0){
                printf("ERROR (stream_parser): BLE Data formatted incorrectly.\n");
                continue;
            }
                
            t_start = SAMPLE_PERIOD;
            t_stop = size * SAMPLE_PERIOD;

            if ((mode & RUN_MODE) == 0x01) {
                printf("-------------\n");
                
                // collect data and extract features
                get_all_features(sensors_buf, fft_buf, input, NUM_SENSORS, &t_start, &t_stop);
                // feed into FANN network
                // format: rms(3) corrs(3):xy,xz,yz energy(3) stddev(3)
                fann_reset_MSE(ann);
                calc_out = fann_run(ann, input);
                printf("fann inputs: ");
                for (i=0; i<NUM_INPUTS; i++) {
                    printf("%f ", input[i]);
                }
                printf("\n");
                
                max = -5.0;
                result = 0;
                printf("fann outputs: ");
                for (i=0; i<num_classes; i++) {
                    printf("%f ", calc_out[i]);
                    if (calc_out[i] > max) {
                        max = calc_out[i];
                        result = i;
                    }
                }   
                printf("\n");
                
                // print and compare to expected output
                float test_output[num_classes];
                memset(test_output, 0, sizeof(float)*num_classes);
                test_output[result] = 1.0;
                fann_test(ann, input, test_output); 
                printf("expected: ");
                for (i=0; i<num_classes; i++) {
                    printf("%f ", test_output[i]);
                }   
                printf("\n");
                printf("result: %d, MSE: %f\n", result, fann_get_MSE(ann));
                
                // fill history buffers and detect gestures
                
                // fill the buffers at the index above
                printf("%d\n", hist_buf_index);
                freq_buf[0][hist_buf_index] = get_freq(fft_buf[0], FFT_SIZE);
                freq_buf[1][hist_buf_index] = get_freq(fft_buf[1], FFT_SIZE);  
                freq_buf[2][hist_buf_index] = get_freq(fft_buf[2], FFT_SIZE);  
                results_buf[hist_buf_index] = fann_get_MSE(ann);

                // start detecting new gestures once buffer full
                if (hist_buf_full) {
                    new_gesture_detected = detect_new_gesture(freq_buf, results_buf, hist_buf_index, mse_threshold);
                }
                
                // order not important, so just need to keep track 
                // of index where new frames are logged
                if (hist_buf_index >= HISTORY_SIZE-1) {
                    hist_buf_index = 0;
                    hist_buf_full = 1; // buffer full flag will stay high once buffer full
                }
                else {
                    hist_buf_index++; 
                }
                
                // transition to PROMPT_MODE if necessary
                if (new_gesture_detected & DETECT_NEW_GESTURES_ON) {
                    mode |= PROMPT_MODE;
                }
                
            }
            if ((mode & PROMPT_MODE) == 0x02) {
                printf("New gesture detected.\n");
                // button press handled in interrupt 
                 mode = TRAIN_MODE;
                // set new_gesture_detected to 0 if transition to train mode
            } 
            if ((mode & TRAIN_MODE) == 0x04) {
                new_gesture_detected = 0;
                if (cycle_count == 0) {    
                    mode = TRAIN_MODE;
                    num_classes++;
                    printf("Collect data for new gesture. Press ENTER when ready.\n");
                    getchar();
                    total_num_samples = prepare_train_file(curr_train_filename, num_classes);
                    curr_train_file = fopen(NEW_TRAIN_FILE, "a");
                    curr_train_filename = NEW_TRAIN_FILE;
                }
                if (cycle_count < NUM_CYCLES) {
                    
                    printf("Cycle %i of %i \n", cycle_count + 1, NUM_CYCLES);
                    get_all_features(sensors_buf, fft_buf, input, NUM_SENSORS, &t_start, &t_stop);
                    update_train_file(curr_train_file, input, num_classes-1, num_classes);
                    cycle_count++;
                }
                else {
                    // train network, update fann file
                    fclose(curr_train_file);
                    total_num_samples += cycle_count;
                    sprintf(cmd_buf, "sed -i 's/nsamples/%d/g' %s", total_num_samples, NEW_TRAIN_FILE);
                    system(cmd_buf);
                    sprintf(cmd_buf, "sed -i 's/nclasses/%d/g' %s", num_classes, NEW_TRAIN_FILE);
                    system(cmd_buf);
                    fann_destroy(ann);
                    train(ann, NEW_TRAIN_FILE, NEW_FANN_FILE, num_classes); 
                    printf("num fann output in main: %d\n", fann_get_num_output(ann)); 
                    ann = fann_create_from_file(NEW_FANN_FILE);
                    
                    // reset/update program states
                    cycle_count = 0;
                    memset(results_buf, 0, HISTORY_SIZE * sizeof(float));
                    for (i=0; i<NUM_SENSORS * 3; i++) {
                        memset(freq_buf[i], 0, HISTORY_SIZE * sizeof(float));
                    }
                    mode = RUN_MODE; //return to regular run mode
                }

            }        
        }

    }

    fann_destroy(ann);
    system("pkill gatttool");

    return 0;
}
