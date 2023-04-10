CC = g++ -std=c++17
CFLAGS = -Wall

a.out: main.o guest.o cleaning.o
	$(CC) $(CFLAGS) main.o guest.o cleaning.o -lpthread

main.o: main.cpp main.hpp
	$(CC) $(CFLAGS) -c main.cpp

guest.o: guest.cpp guest.hpp
	$(CC) $(CFLAGS) -c guest.cpp

cleaning.o: cleaning.cpp cleaning.hpp
	$(CC) $(CFLAGS) -c cleaning.cpp

clean:
	rm -f *.o a.out