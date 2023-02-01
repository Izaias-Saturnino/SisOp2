#include <stdio.h>
#include <iostream>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include<fcntl.h> 
#include<string> 

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))


int inotify_fd,watch_dir;

std::string name="";
std::string command ="";
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; 

int action =0;

void *folderchecker(void *arg)
{
    inotify_fd = inotify_init();


    if (fcntl(inotify_fd, F_SETFL, O_NONBLOCK) < 0)  // error checking for fcntl
       exit(2);

    watch_dir = inotify_add_watch(inotify_fd, "./sync_dir", IN_MODIFY | IN_CREATE | IN_DELETE);
    int run;
    if(watch_dir==-1){
        printf("erro");
        run=0;
    }else{
        run=1;
    }

    while (run)
    {
        int length;
        int i = 0;
        char buffer[EVENT_BUF_LEN];
        length = read(inotify_fd, buffer, EVENT_BUF_LEN);
        while (i < length)
        {
            pthread_mutex_lock(&m);
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                if (event->mask & IN_CREATE)
                {
                   name = event->name;
                   action =1;
                }
                else if (event->mask & IN_DELETE)
                {
                   name = event->name;
                   action =2;

                }
                else if (event->mask & IN_MODIFY)
                {
                    name = event->name;
                    action =3;

                }
            }
            i += EVENT_SIZE + event->len;
            pthread_mutex_unlock(&m);
        }
    }
}

void *input(void *arg)
{
    std::cin >> command;
}
