autotrain:
	gcc -o autotrain train.c dsp.c helpers.c preprocessing.c -lm -lc -lliquid

train:
	gcc -o train_fann train_fann.c -lm -lfann

test:
	gcc -o test test.c dsp.c helpers.c preprocessing.c -lm -lc -lfann -lliquid

clean:
	rm autotrain
	rm test
	rm train_fann
