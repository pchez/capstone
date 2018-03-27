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
#include "fann/src/include/fann.h"
#include "fann/src/include/floatfann.h"

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
    fann_type *calc_out;
    fann_type input[3];
    struct fann *ann;
    struct fann_train_data *data;
	
    float norm_cutoff;
	float norm_centerf;
	float t_start, t_stop;
	float rms_signal;
	unsigned int size;
	char * input_file = "motion_data.dat";
	int n_cycles;
	int cycle_count;
    char call_shell_script[128];
    char call_tail[128];

    int sigwait_result;
    int sig;
    int i, nbytes, result;
    float max;
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

    signal(SIGINT, sigHandler);

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
        
        while (run) {
            
            kill(child_pid, SIGUSR1);   
            sigwait_result = sigwait(&signal_set, &sig);

            size = BLE_parse(input_file, TEST_MODE);
            if(size == 0){
                printf("ERROR (stream_parser): BLE Data formatted incorrectly.\n");
            }
            else {
                compute_average(size, input);
                calc_out = fann_run(ann, input);
                printf("%f %f %f\n", input[0], input[1], input[2]);
                max = -5.0;
                result = 0;
                for (i=0; i<6; i++) {
                    if (calc_out[i] > max) {
                        max = calc_out[i];
                        result = i;
                    }
                }    
                printf("result: %d\n", result);
                printf("result: %f %f %f %f %f %f\n", calc_out[0], calc_out[1], calc_out[2], calc_out[3], calc_out[4], calc_out[5]);
                        
            }

            cleanup();
        }

    }

    fann_destroy(ann);
    system("pkill gatttool");

    return 0;
}
