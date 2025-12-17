CC = gcc
CFLAGS = -I../include -Wall
LIBS = -lsqlite3

# Biên dịch tất cả các file cần thiết
SRCS = database.c main.c game.c room.c
TARGET = game_app

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)