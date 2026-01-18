ifeq ($(OS),Windows_NT)
    LIBS = -lws2_32
    TARGET = toralize.exe
    CFLAGS = -O2 -fstack-protector-strong -Wall
else
    LIBS = -ldl
    TARGET = toralize.so
    CFLAGS = -fPIC -shared -D_GNU_SOURCE -O2 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now -Wall
endif

all:
	gcc toralize.c -o $(TARGET) $(CFLAGS) $(LIBS)