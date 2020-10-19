all:
	gcc -g -Wall -Wno-deprecated -o memory_allocator.o memory_allocator.c

clean:
	@echo "Cleaning up..."
	rm *.o