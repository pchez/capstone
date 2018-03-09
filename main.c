#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include "fann/src/include/fann.h"
#include "fann/src/include/floatfann.h"
#define EXPECTED_NUM_ARGS 2
#define DEBUG 1
#define NUM_MODES 1

int data_collected_flag = 0;
pid_t child_pid, parent_pid; 
char** trainingPrompts[NUM_MODES];
int numTrainingSteps[NUM_MODES] = {6};

enum train_mode {
    POSITION  // Basic training using all 3 axes of each sensor, output is a position 
};


void exitHandler(char* err_string, int exit_code) {
    fprintf(stderr, "exitHandler: %s\n", err_string);
    kill(child_pid, SIGTERM);        
    exit(exit_code);
}

void sigHandler(int signum) {
    if (signum == SIGUSR1) {
        data_collected_flag = 1;
    }
    if (signum == SIGSEGV) {
        exitHandler("Caught SIGSEGV, exiting child processes", -2);
    }
    if (signum == SIGINT) {
        exitHandler("Caught SIGINT, exiting child processes", 0);
    }
}


void train(int train_mode, char* fann_input, char* fann_output) {
    int num_steps = numTrainingSteps[train_mode];
    int status;
    int sigwait_result;
    int sig;
    int i, nbytes;
    sigset_t signal_set;
    char ppid_string[8];
    int pipefd[2];
    char buf[1024];
    int revent[1];

    // Fann variables initialization
    const unsigned int num_input = 6;
    const unsigned int num_output = 1;
    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 3;
    const float desired_error = (const float) 0.001;
    const unsigned int max_epochs = 500000;
    const unsigned int epochs_between_reports = 1000;
    struct fann *ann;
   
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

        execlp("./collect_data.sh", "./collect_data.sh",  ppid_string, (char*) NULL); //, fann_input_name);
        do {
            exitHandler("execlp error", -2);
        } while(0);
    
    }
    else {
        close(pipefd[1]);
        //printf("Poll for child input\n");
        sleep(1);
        for (i=0; i < num_steps; i++) {
            //printf("sending SIGUSR1 to %ld\n", child_pid);
            printf("%s %s\n", trainingPrompts[train_mode][i], "Press ENTER to continue.");
            getchar();
            kill(child_pid, SIGUSR1);    
            //printf("before sigwait\n");
            sigwait_result = sigwait(&signal_set, &sig); 
            //printf("Got SIGUSR1\n");

        }     
    }
    // Add first line according to fann training file format: [#samples][#inputs][#outputs]
    // TODO count the number of samples
    int num_samples = 482;
    sprintf(buf, "sed -i '1i %d %d %d' %s", num_samples, num_input, num_output, fann_input);
    system(buf);
    printf("Finished collecting and converting data.\n");
    
    printf("Training...\n");
    ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);
    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
//    fann_train_on_file(ann, fann_input, max_epochs, epochs_between_reports, desired_error);
  //  fann_save(ann, fann_output);
   // fann_destroy(ann);

    printf("Finished training.\n"); 
    
}

void initPrompts(char*** trainingPrompts) {
    int mode, step;

    // Allocate mem for strings
    for (mode=0; mode < NUM_MODES; mode++) {
        trainingPrompts[mode] = (char**) malloc(numTrainingSteps[mode] * sizeof(char)); 
        for (step=0; step < numTrainingSteps[mode]; step++) {
            trainingPrompts[mode][step] = (char*) malloc(1024);
        }
    }
    trainingPrompts[POSITION][0] = "Place sensor face up.";
    trainingPrompts[POSITION][1] = "Place sensor face down.";
    trainingPrompts[POSITION][2] = "Tilt sensor 90deg upwards.";
    trainingPrompts[POSITION][3] = "Tilt sensor 90deg downwards.";
    trainingPrompts[POSITION][4] = "Tilt sensor 90deg to the right.";
    trainingPrompts[POSITION][5] = "Tilt sensor 90deg to the left.";
    
}

void test(int mode, struct fann* ann, fann_type* output_label) {
    kill(child_pid, SIGUSR2);
    
    //output_label = fann_run(ann, input);
    
}

int main(int argc, char** argv) {
    // TODO: use optarg
    int mode = 0;
    char* fann_input = NULL;
    char* fann_output = NULL;
    fann_type* output_label;
    fann_type input[2];
    struct fann* ann;
    
    if (argc < EXPECTED_NUM_ARGS) {
        exitHandler("Usage: ./auto_fann_train --[training mode]", -1);
    }
    else {
        mode = atoi(argv[1]);
        fann_input = "motion_converted.txt";
        fann_output = "fann_output.net";
    }

    // Initialize training prompt texts
    initPrompts(trainingPrompts);

    // Get mode we are trying to train for, collect
    train(mode, fann_input, fann_output);
   
    // Validate learning
    ann = fann_create_from_file(fann_output);
    printf("Test system:\n");
    sleep(1);
    while(1) {
        test(mode, ann, output_label);
    }

    return 0;
}
