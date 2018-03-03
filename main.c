#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#define EXPECTED_NUM_ARGS 2

int data_collected_flag = 0;
pid_t child_pid, parent_pid; 

enum training_mode {
    POSITION  // Basic training using all 3 axes of each sensor, output is a position 
};

void exitHandler(char* err_string, int exit_code) {
    fprintf(stderr, "exitHandler: %s\n", err_string);
    printf("kill %ld\n", child_pid);
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
    if (signum == SIGTERM) {
        exitHandler("Caught SIGTERM, exiting child processes", 0);
    }
}

void trainPosition(char* output_file) {
    char* output_file_name = output_file;
    int num_steps = 6;
    int status;
    int sigwait_result;
    int sig;
    int i, nbytes;
    sigset_t signal_set;
    char ppid_string[8];
    int pipefd[2];
    char buf[1024];

    // Set up signal set and signal handling
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &signal_set, NULL);
    signal(SIGSEGV, sigHandler);

    // Initiate data collection from SensorTile via bluetooth command in shell script
    pipe(pipefd);
    child_pid = fork();
    if (child_pid == -1) {
        exitHandler("Fork error", -1);
    }
    if (child_pid == 0) { 
        parent_pid = (long) getppid();
        
        sprintf(ppid_string, "%ld", parent_pid);
        printf("parent_pid: %ld\n", parent_pid);
        printf("Data collection process started with PID: %ld\n", child_pid);
        
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        execlp("./collect_data.sh",  ppid_string, output_file_name);
        do {
            exitHandler("execlp error", -2);
        } while(0);
    
    }
    else {
        printf("before pipe redirecting\n");
        close(pipefd[1]);
        sleep(0.5);
        for (i=0; i < num_steps; i++) {
            printf("here\n");
            while((nbytes = read(pipefd[0], buf, sizeof(buf))) != 0) {
                printf("%.s", nbytes, buf);
                memset(buf, 0, 1024);
            }

            printf("sending SIGUSR1 to %ld\n", child_pid);
            kill(child_pid, SIGUSR1);    
            printf("before sigwait\n");
            sigwait_result = sigwait(&signal_set, &sig); 

            printf("Got SIGUSR1\n");
             
        }     
    }
/*
    printf("before pipe redirecting\n");
    close(pipefd[1]);
    dup2(pipefd[0], STDOUT_FILENO);
    close(pipefd[0]);

    for (i=0; i < num_steps; i++) {
        kill(child_pid, SIGUSR1);    
        printf("before sigwait\n");
        sigwait_result = sigwait(&signal_set, &sig); 

        printf("Got SIGUSR1\n");
    }
 */   
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
