NAAG : a.out
	./a.out

a.out: sns.cpp
	g++ sns.cpp

clean:
	rm -f a.out sns.log