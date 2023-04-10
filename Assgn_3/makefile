# declare variables for compiler flags
CC = g++
CFLAGS = -Wall -g -std=c++17

a.out : main.o consumer.o producer.o
	$(CC) $(CFLAGS) -o a.out main.o consumer.o producer.o

main.o : main.cpp main.hpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

consumer.o : consumer.cpp consumer.hpp
	$(CC) $(CFLAGS) -c consumer.cpp -o consumer.o

producer.o : producer.cpp producer.hpp
	$(CC) $(CFLAGS) -c producer.cpp -o producer.o

clean :
	rm -f *.o a.out