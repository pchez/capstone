/*
Fast Artificial Neural Network Library (fann)
Copyright (C) 2003-2016 Steffen Nissen (steffen.fann@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "fann.h"

int main(int argc, char** argv)
{
	const unsigned int num_input = 12;
	const unsigned int num_output = 3;
	const unsigned int num_layers = 3;
	const unsigned int num_neurons_hidden = 6;
	const float desired_error = (const float) 0.001;
	const unsigned int max_epochs = 500000;
	const unsigned int epochs_between_reports = 1000;
    char* train_file;
	
    struct fann *ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);

	fann_set_activation_function_hidden(ann, FANN_SIGMOID);
	fann_set_activation_function_output(ann, FANN_SIGMOID);

    if (argc < 2) {
        train_file = "motion_data_output.csv";
    }
    else {
        train_file = argv[1];
    }
    fann_train_on_file(ann, train_file, max_epochs, epochs_between_reports, desired_error);

	fann_save(ann, "fann_motion.net");

	fann_destroy(ann);

	return 0;
}
