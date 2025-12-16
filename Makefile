CC = gcc
CFLAGS = -Wall -g -Iserver/include
LIBS = -lsqlite3

SERVER_SRC = server/src/main.c server/src/database.c server/src/game.c server/src/network.c server/src/room.c server/src/utils.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_BIN = server_app

TEST_DB_SRC = server/src/test_db.c server/src/database.c server/src/utils.c
TEST_DB_BIN = test_db

all: $(SERVER_BIN) $(TEST_DB_BIN)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(TEST_DB_BIN): $(TEST_DB_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(SERVER_BIN) $(TEST_DB_BIN) server/src/*.o database/*.db

test: $(TEST_DB_BIN)
	./$(TEST_DB_BIN)

run: $(SERVER_BIN)
	./$(SERVER_BIN)