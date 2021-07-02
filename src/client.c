#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>



#include "common.h"

struct client_data client_threads[MAX_THREADS]={};

void *work_client_fn(void *data) 
{
  struct client_data *cl_data = (struct client_data *)data;
  printf("(%s) Working...\n", __func__);
  if ((cl_data->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("(%s) Error creating socket...\n", __func__);
    return 0;
  }
  printf("(%s) socket created...\n", __func__);
      // MTU change
      struct ifreq ifr;
      ifr.ifr_addr.sa_family = AF_INET; // address family
      strncpy(
          ifr.ifr_name, "eth0",
          sizeof(ifr.ifr_name)); // interface name where you want to set the MTU
      ifr.ifr_mtu = 9*1024;        // your MTU size here
      if (ioctl(cl_data->fd, SIOCSIFMTU, (caddr_t)&ifr) < 0)
        printf("(%s) Failed to set MTU to (%d) \n", __func__, ifr.ifr_mtu);

      int recvBuff = 512 * 1024;
      int sendBuff = 512 * 1024;
      if (setsockopt(cl_data->fd, SOL_SOCKET, SO_RCVBUF, &recvBuff,
                     sizeof(recvBuff)) < 0) {
        printf("(%s) Error setsockopt (%s)\n", __func__, strerror(errno));
      }
      if (setsockopt(cl_data->fd, SOL_SOCKET, SO_SNDBUF, &sendBuff,
                     sizeof(sendBuff)) < 0) {
        printf("(%s) Error setsockopt (%s)\n", __func__, strerror(errno));
      }

  cl_data->server_addr.sin_family = AF_INET;
  cl_data->server_addr.sin_port = htons(SRV_PORT);
  inet_pton(AF_INET, SRV_ADDR, &cl_data->server_addr.sin_addr);
  if (connect(cl_data->fd, (struct sockaddr *)&cl_data->server_addr,
              sizeof(cl_data->server_addr)) < 0) {
    printf("(%s) Error connecting socket...\n", __func__);
    return 0;
  }
  printf("(%s) socket connected...\n", __func__);
  // char msg[10] = "Hola.";
  int total_writed = 0;
  struct msg msg = {MAX_MSG_SIZE, "\nhola"};
  int status = 0;
  int packet_writed = 0;
  for (cl_data->cnt = 0; cl_data->cnt < MAX_NUM_MSGS; cl_data->cnt++) {
     status = write(cl_data->fd, &msg, sizeof(msg));
	  /*
    for (packet_writed = 0; packet_writed < MAX_MSG_SIZE;
         packet_writed += status){
      if (packet_writed + SEND_MSG_CHUNK <= MAX_MSG_SIZE)
        status = write(cl_data->fd, &msg + packet_writed, SEND_MSG_CHUNK);
      else
        status = write(cl_data->fd, &msg + packet_writed,
                       (MAX_MSG_SIZE - packet_writed));
    }
    */

    total_writed += status;
    //total_writed += packet_writed;
    if (status == 0) {
      printf("(%s) socket write == 0...\n", __func__);
    } else if (status < 0) {
      printf("(%s) socket write < 0..(%s).\n", __func__, strerror(errno));
    }
  }
  printf("(%s) Total writed (%d)\n", __func__, total_writed);
  total_writed = 0;
  // End comunication
  msg.size = 0;
  write(cl_data->fd, &msg, sizeof(msg));

  // Receive a reply from the server
  char server_response[MAX_SRV_RESPONSE];
  if (recv(cl_data->fd, server_response, MAX_SRV_RESPONSE, 0) < 0) {
    printf("(%s) Error receiving response\n", __func__);
  } else {
    if (strcmp(server_response, OK_SRV_RESPONSE) == 0) {
      printf("(%s) SUCCESS receiving response\n", __func__);
    } else {
      printf("(%s) ERROR receiving response\n", __func__);
    }
  }
  close(cl_data->fd);
  printf("(%s) socket close...\n", __func__);
  return 0;
}

int main(int argc, char* argv[])
{

    printf("(%s) Creating threads ....\n", __func__);
  for (int i = 0; i < MAX_THREADS; i++) {
    client_threads[i].thread_id = pthread_create(
        &client_threads[i].thread, NULL, work_client_fn, (void *)&client_threads[i]);
  }


    printf("(%s) Join threads ......\n", __func__);
  for (int i = 0; i < MAX_THREADS; i++) {
      pthread_join( client_threads[i].thread, NULL);
  }
    printf("(%s) Finish ...sleep ...\n", __func__);
  // Wait to check
  //sleep(60);
  return 0;
}
