all:
	gcc main.c -lpthread -lm -o fil -Wall
san:
	gcc -fsanitize=address main.c sobel.c sio.c common.c -lpthread -lm -o fil -Wall
chf:
	./checkpatch.pl -f -no-tree main.c sobel.c sio.c common.c
chs:
	cppcheck --enable=all --inconclusive --std=posix main.c sobel.c sio.c common.c
