autotrain:
	gcc -o autotrain train.c dsp.c helpers.c prep_train_data.c -lm -lc -lliquid

train:
	gcc -o train_fann train_fann.c -lm -lfann

test:
	gcc -o test test.c dsp.c helpers.c prep_train_data.c -lm -lc -lliquid

clean:
	rm autotrain
