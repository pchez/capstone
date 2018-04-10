#include "fann.h"
#include "preparetrain.h"
#include "main.h"

int main(int argc, char** argv)
{
    struct fann* ann;
    char* train_file;

    if (argc < 2) {
        train_file = ORIG_TRAIN_FILE;
    }
    else {
        train_file = argv[1];
    }
    train(ann, train_file, ORIG_FANN_FILE, NUM_CLASSES); 

	return 0;
}
