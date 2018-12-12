#include"epoll_event.h"

int efd = -1;

void *read_thread(void *dummy){
    printf("thread callback begin!\n");

    int ret=0;
    int ep_fd=-1;
    uint64_t count = 0;

    struct epoll_event events[10];    

    if(efd<0)
    {
	printf("efd should inited!\n");
    	goto Done;
    }
    
    ep_fd=epoll_create(1024);
    if(ep_fd<0){
	perror("epoll_create fail!");
	goto Done;
    }else{
	struct epoll_event read_event;
	read_event.events=EPOLLHUP|EPOLLERR|EPOLLIN;
	read_event.data.fd=efd;
	ret=epoll_ctl(ep_fd,EPOLL_CTL_ADD,efd,&read_event);
	if(ret<0){
	    perror("epoll ctl fialed!");
	    goto Done;	
	}
    }
    
    while(1){
	ret=epoll_wait(ep_fd,&events[0],10,-1);
	if(ret>0){
            int i=0;
	    for(;i<ret;i++){
		if(events[i].events&EPOLLHUP){
		    printf("epoll eventfd  epoll hup %d\n",events[i].events);			
		    goto Done;		
		}
		else if(events[i].events&EPOLLERR){
		    printf("epoll eventfd has epoll error %d\n",events[i].events);
		    goto Done;		
		}
		else if(events[i].events&EPOLLIN){
		    printf("epoll eventfd has event in %d\n",events[i].events);
		    int event_fd = events[i].data.fd;
		    int ret = read(event_fd,&count,sizeof(count));
		    if(ret<0){
			perror("read fail!");
			goto Done;
		    }
		    else{
			struct timeval tv;
			gettimeofday(&tv,NULL);
			printf("success read from efd,read %d bytes(%llu) at %lds %ldus\n",
				ret,count,tv.tv_sec,tv.tv_usec);
		    }

		}		
		
	    }
	}else if(ret==0){
	    printf("wait timeout,ret=%d\n",ret);
            break;
	}else{
	    perror("epoll wait error:");
	    goto Done;
	}
    }
Done:
    printf("thread end!\n");
    if(ep_fd>=0){
	close(ep_fd);
	ep_fd=-1;
    }
    return NULL;
    
}

int main(int argc,char*argv[]){
    pthread_t pid =0;
    int ret=0;
    uint64_t count=0;
    efd=eventfd(0,0);
    if(efd<0){
	perror("eventfd failed!");
	goto Done;
    }
    ret = pthread_create(&pid,NULL,read_thread,NULL);
    if(ret<0){
	perror("pthread create:");
        goto Done;
    }

    count=10;
    for(int i=0;i<5;i++){
	;
	ret=write(efd,&count,sizeof(count));
	if(ret<0){
	    perror("write event fd fail!");
	    goto Done;	
	}else{
	    struct timeval tv;
	    gettimeofday(&tv,NULL);
	    printf("success write to efd,write %d bytes(%llu) at %lds %ldus\n",
		ret,count,tv.tv_sec,tv.tv_usec);
	}
	count++;
        sleep(4);
    }

    printf("main thread will end!\n");


Done:
    if(pid!=0){
        pthread_join(pid,NULL);
        pid=0;
    }
    if(efd>=0){
    	close(efd);
	efd=-1;
    }
    return ret;
}

