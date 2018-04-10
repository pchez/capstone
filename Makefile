CC=gcc
CFLAGS=-c
SOURCES=dsp.c helpers.c preparetrain.c 
OBJECTS=$(SOURCES:.c=.o)

autotrain: autotrain.o $(OBJECTS)
	$(CC) -o autotrain autotrain.o $(OBJECTS) -lm -lc -lliquid

collect: collect.o $(OBJECTS)
	$(CC) -o collect collect.o $(OBJECTS) -lm -lc -lliquid

train: train.o preparetrain.o
	$(CC) -o train train.o preparetrain.o -lm -lfann

test: test.o $(OBJECTS) 
	$(CC) -o test test.o $(OBJECTS) -lm -lc -lfann -lliquid

main: main.o $(OBJECTS) 
	$(CC) -o main main.o $(OBJECTS) -lm -lc -lfann -lliquid

main.o: main.c
	$(CC) $(CFLAGS) main.c

autotrain.o: autotrain.c
	$(CC) $(CFLAGS) autotrain.c

collect.o: collect.c
	$(CC) $(CFLAGS) collect.c

test.o: test.c
	$(CC) $(CFLAGS) test.c

train.o: train.c
	$(CC) $(CFLAGS) train.c

dsp.o: dsp.c
	$(CC) $(CFLAGS) dsp.c

helpers.o: helpers.c
	$(CC) $(CFLAGS) helpers.c

preparetrain.o: preparetrain.c
	$(CC) $(CFLAGS) preparetrain.c

clean:
	rm *.o autotrain test train
