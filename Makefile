CC=gcc

target=nig

obj=nig.o  nig_alloc.o nig_string.o nig_progress.o nig_http.o nig_epoll.o nig_fastcgi.o   

$(target):$(obj)
	$(CC) $(obj) -Wall -o $(target)

nig.o:nig.c
	$(CC) -g -c nig_header.h nig_config.h nig.c 
	
nig_alloc.o:nig_alloc.c
	$(CC) -g -c nig_alloc.h nig_alloc.c

nig_string.o:nig_string.c
	$(CC) -g -c nig_string.h nig_string.c
	
nig_progress.o:nig_progress.c
	$(CC) -g -c nig_progress.h nig_progress.c

nig_http.o:nig_http.c
	$(CC) -g -c nig_http.h nig_http.c
	 
nig_epoll.o:nig_epoll.c
	$(CC) -g -c nig_epoll.h nig_epoll.c
	
nig_fastcgi.o:nig_fastcgi.c
	$(CC) -g -c nig_fastcgi.h nig_fastcgi.c
	
.PHONY:clean

clean:
	rm -rf *.o && rm -rf *.gch
