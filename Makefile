ifeq ($(OS),Windows_NT)
    LIBS = -lws2_32
else
    LIBS =
endif

all:
	gcc toralize.c -o toralize $(LIBS)