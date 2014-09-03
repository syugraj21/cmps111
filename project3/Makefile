# Justin Kwok, Ian Hamilton, Yugraj Singh, Ben Lieu, Greg Arnheiter
# Created 5/20/14

all: slug_test

slug_test: slug_malloc.o linked_list.o slug_test.o
	$(CC) slug_malloc.o linked_list.o slug_test.o -o slug_test
slug_malloc.o: slug_malloc.c linked_list.c
	$(CC) -c -w slug_malloc.c linked_list.c
linked_list.o: linked_list.c 
	$(CC) -c -w linked_list.c 
slug_test.o: slug_test.c
	$(CC) -c -w slug_test.c
clean:
	rm -f *.o *.gch slug_test
