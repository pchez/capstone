CC=gcc
CFLAGS=-c
SOURCES=dsp.c helpers.c preprocessing.c
OBJECTS=$(SOURCES:.c=.o)

autotrain: train.o $(OBJECTS)
	$(CC) -o autotrain train.o $(OBJECTS) -lm -lc -lliquid

train:
	$(CC) -o train train_fann.c -lm -lfann

test: test.o $(OBJECTS) 
	$(CC) -o test test.o $(OBJECTS) -lm -lc -lfann -lliquid

test.o: test.c
	$(CC) $(CFLAGS) test.c

train.o: train.c
	$(CC) $(CFLAGS) train.c

dsp.o: dsp.c
	$(CC) $(CFLAGS) dsp.c

helpers.o: helpers.c
	$(CC) $(CFLAGS) helpers.c

preprocessing.o: preprocessing.c
	$(CC) $(CFLAGS) preprocessing.c

clean:
	rm *.o autotrain test train_fann
