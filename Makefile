autotrain:
	gcc -o autotrain main.c dsp.c helpers.c prep_train_data.c -lm -lc -lliquid

train:
	gcc -o train_fann train_fann.c -lm -lfann

clean:
	rm autotrain
