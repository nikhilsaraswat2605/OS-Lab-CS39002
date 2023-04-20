all : libgoodmalloc.a merge_sort

libgoodmalloc.a: goodmalloc.cpp goodmalloc.h
	g++ -c goodmalloc.cpp -o goodmalloc.o
	ar rcs libgoodmalloc.a goodmalloc.o
	
merge_sort: libgoodmalloc.a merge_sort.cpp
	g++  -o merge_sort merge_sort.cpp -L. -lgoodmalloc

clean:
	rm -f merge_sort *.o *.a
