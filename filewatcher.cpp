#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h> 

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

extern int operation;
extern std::string filename;
void *folderchecker()
{
    int inotify_fd,watch_dir;
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
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                if (event->mask & IN_CREATE)
                {
                   printf("Message: %s ,created\n", event->name);
                   operation =0;
                   filename=event->name
                }
                else if (event->mask & IN_DELETE)
                {
                   printf("Message: %s ,deleted\n", event->name);
                   operation =1;
                   filename=event->name

                }
                else if (event->mask & IN_MODIFY)
                {
                   printf("Message: %s ,modified\n", event->name);
                   operation =2;
                   filename=event->name
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
}