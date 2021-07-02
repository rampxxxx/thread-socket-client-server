#define _GNU_SOURCE // gettid
#include <unistd.h> // gettid

#include <sys/ioctl.h>
#include <net/if.h>

#include <stdio.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h> // uint32_t
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h> 
#include <unistd.h>

#define SRV_PORT 6666
#define SRV_ADDR "192.168.1.43"
#define MAX_THREADS 64 //4
#define MAX_BACKLOG MAX_THREADS
#define MAX_SRV_THREADS 3
#define MAX_MSG 5
#define MAX_SRV_RESPONSE 10
#define MAX_MSG_SIZE 8*1024 //10
#define SEND_MSG_CHUNK 10
#define OK_SRV_RESPONSE "ok"
#define FILE_BUFFER_4k 4096
#define FILE_BUFFER_8k 8192
#define FILE_BUFFER_16k 16384
#define FILE_BUFFER_128k 131072
#define FILE_BUFFER_256k 262144
#define FILE_BUFFER_SIZE FILE_BUFFER_4k
#define MAX_NUM_MSGS 5*1000//*1000

struct client_data{
	pthread_t thread;
	int thread_id;
	bool sent;
	bool received;
	char data[MAX_MSG];
	int fd;
	struct sockaddr_in server_addr;
	uint64_t cnt;
};

struct server_data{
	pthread_t thread;
	pid_t thread_id;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int fd;
};

struct msg{
	int size;
	char data[MAX_MSG_SIZE];

};
