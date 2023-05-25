CC = gcc
FLAGS := -g -Wconversion -Wextra -Wall -Wno-unused
LIBS  := -I/usr/local/include/gtk-4.0/ -I/usr/include/boost -I/usr/local/include/libpng12 -I/usr/include/gio-unix-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/fribidi  -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/graphene-1.0 -I/usr/lib/x86_64-linux-gnu/graphene-1.0/include -I/usr/include/libmount -I/usr/include/ -I/usr/include/blkid -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -L/usr/local/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu/ -lgtk-4 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -lgdk_pixbuf-2.0 -lcairo-gobject -lcairo -lgraphene-1.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lm -pthread -lrt -lcoap-3 -mfpmath=sse -msse -msse2

SRC_DIR:= src/

SRC_LIST = $(wildcard $(SRC_DIR)*.c)


TARGET:= main

.PHONY = all clean

all: $(TARGET)

$(TARGET): $(SRC_LIST)
	export LD_LIBRARY_PATH=/usr/local/lib
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)
clean:
	rm main *.o

