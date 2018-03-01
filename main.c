#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#define EXPECTED_NUM_ARGS 2

int data_collected_flag = 0;

enum training_mode {
    POSITION  // Basic training using all 3 axes of each sensor, output is a position 
};

void exitHandler(char* err_string, int exit_code) {
    fprintf(stderr, "exitHandler: %s\n", err_string);
    exit(exit_code);
}

void sigHandler(int signum) {
    if (signum == SIGUSR1) {
        data_collected_flag = 1;
    }
}

void trainPosition(char* output_file) {
    char* output_file_name = output_file;
    pid_t cpid, w;
    pid_t child_pid; 
    int num_steps = 6;
    int status;
    int sigwait_result;
    int sig;
    sigset_t signal_set;

    // Set up signal set
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &signal_set, NULL);

    // Initiate data collection from SensorTile via bluetooth command in shell script
    cpid = fork();
    if (cpid == -1) {
        exitHandler("Fork error", -1);
    }
    
    if (cpid == 0) {
        child_pid = (long) getpid();
        printf("Data collection process for step %d started, with PID: %ld\n", i, child_pid);
        execlp("./collect_data.sh",  getppid(), output_file_name);
    }
    else {
        for (int i=0; i < num_steps; i++) {
            kill(child_pid, SIGUSR1);    
            sigwait_result = sigwait(&signal_set, &sig); 

            if (data_collected_flag) {
                print("Got SIGUSR1\n");
            } 
        }    
    }
    printf("Finished collecting and converting data.\n"); 

}

int main(int argc, char** argv) {
    // TODO: use optarg
    int mode = 0;
    char* output_file_name = NULL;

    if (argc < EXPECTED_NUM_ARGS) {
        exitHandler("Usage: ./auto_fann_train --[training mode]", -1);
    }
    else {
        mode = atoi(argv[1]);
        output_file_name = "train_motion.txt";
    }

    switch(mode) {
        case POSITION:
            trainPosition(output_file_name);
            break;
        default:
            exitHandler("Invalid training mode provided.", -1);
            break;
    }
   
    return 0;
}
