CFLAGS = -Wno-implicit-function-declaration

# all: final

# final: main.o client.o
# 	echo "Linking and producing final"
# 	gcc $(CFLAGS) main.o client.o -o final

# main.o: main.c
# 	gcc $(CFLAGS) -c main.c
# client.o: client.c
# 	gcc $(CFLAGS) -c client.c


server: main.o
	gcc main.o -o server

client: client.o
	gcc client.o -o client


clean:
	rm -rf *.o final server client
