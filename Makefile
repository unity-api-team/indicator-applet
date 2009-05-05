CC=gcc

all: main.c
	$(CC) `pkg-config glib-2.0 gtk+-2.0 clutter-0.9 clutter-gtk-0.9 indicator indicate --libs --cflags` -o clutter-indicate main.c

clean:
	/bin/rm -f *.o *~

