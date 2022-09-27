#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#define BUFLEN 100

int pid,fd;

pthread_t thread1,thread2;
bool quit = true;

void *reader(void *args){

	char out_buff[BUFLEN];
	int retval;
	while(quit){
		retval=read(fd,out_buff,BUFLEN);
			if(retval<0){
				printf("Read Failed: \n");
				exit(1);
			}
		if(retval == 0){
			if(out_buff[0]!='\0')
				printf("%s\n",out_buff);
		}
	}
}

void *writer(void  *args){

	char in_buff[100];
	sprintf(in_buff,"%d joined",pid);

        if(write(fd,in_buff,strlen(in_buff)+1)<0){
                perror("write failed: ");
                exit(2);
        }
	while(quit){
		gets(in_buff);
		int length = strlen(in_buff);
                in_buff[length]='\0';
		if(strcmp(in_buff,"Bye")==0){
                        quit=false;
                        sprintf(in_buff,"%d exit\n",pid);
                        if(write(fd,in_buff,strlen(in_buff)+1)<0){
                                printf("write failed: ");
                                exit(3);
                        }
                        break;
                }
		if(write(fd,in_buff,strlen(in_buff)+1)<0){
                                printf("Write failed: ");
                                exit(4);
                }
	}
}

int main(int argc, char *argv[]) {
	pid=strtol(argv[1],(char **)NULL,10);

	printf("Process joined: %d\n",pid);
	 fd = open("/dev/chatroom", O_RDWR);
	if( fd < 0) {
		perror("Open failed: ");
		exit(5);
	}

	pthread_create(&thread1,NULL,writer,NULL);
	pthread_create(&thread2,NULL,reader,NULL);

	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);

	return 0;
}