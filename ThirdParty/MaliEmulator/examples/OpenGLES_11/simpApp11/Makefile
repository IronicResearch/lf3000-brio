NAME = simpApp11

CC = gcc
CFLAGS = -g -c
INCLUDES = -I../../../include
LIBS = -L/usr/X11R6/lib -lX11 -L../../../bin  -lGLESv1_CM -lEGL

all: $(NAME)

$(NAME): simpApp11.c
	$(CC) -g  $(INCLUDES) -o $@ $^ $(LIBS)

clean:
	rm $(NAME)
