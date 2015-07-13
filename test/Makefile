alloc.o:
	gcc -c ../alloc.c -o alloc.o

test-alloc:
	gcc test-alloc.c -o test-alloc -I../

test-string: alloc.o
	gcc test-string.c alloc.o -o test-string -I../

clean:
	rm -f test-alloc test-string *.o
