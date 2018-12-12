epoll_event_exe:epoll_event.o
	gcc -o epoll_event_exe epoll_event.o -l pthread
epoll_event.o:epoll_event.c epoll_event.h
	gcc -c epoll_event.c
clean:
	rm epoll_event_exe epoll_event.o

