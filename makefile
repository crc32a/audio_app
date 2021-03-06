BIN=./bin
CF= -O0 -ggdb -I. -I/opt/local/include -L/opt/local/lib
GF=$(CF) `pkg-config --cflags gtk+-3.0`
LIBS= -lm -lsndfile
GL= `pkg-config --libs gtk+-3.0` -lm -lsndfile -lpthread

all: audio_app

dft.o: dft.c dft.h
	gcc $(CF) -c dft.c

audio_app.o: audio_app.c dft.h
	gcc $(GF) -c audio_app.c

gladexml.o: audio_xml.glade
	ld -r -b binary -o gladexml.o audio_xml.glade

audio_app: audio_app.o dft.o gladexml.o 
	gcc $(GF) -o $(BIN)/audio_app audio_app.o gladexml.o dft.o $(GL) -rdynamic

clean:
	rm -rf *.o
	rm ./bin/*
