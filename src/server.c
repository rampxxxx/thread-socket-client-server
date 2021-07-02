


#include "common.h"

struct server_data server_threads[MAX_SRV_THREADS]={};
int file_number=0;

bool is_worker_free(void)
{

  for (int i = 0; i < MAX_SRV_THREADS; i++) {
    if (server_threads[i].fd == -1) {
      printf("(%s) Found free (%d)\n", __func__, i);
      return true;
    }
  }
  return false;
}

void assing_job(int fd)
{

  for (int i = 0; i < MAX_SRV_THREADS; i++) {
    pthread_mutex_trylock(&server_threads[i].lock);
    if (server_threads[i].fd == -1) {
      server_threads[i].fd = fd;
      printf("(%s) A1 Signaling task in i/fd (%d)/(%d) \n", __func__, i,
             server_threads[i].fd);
      pthread_cond_signal(&server_threads[i].cond);
      printf("(%s) A2 Assign task in i/fd (%d)/(%d) \n", __func__, i,
             server_threads[i].fd);
      pthread_mutex_unlock(&server_threads[i].lock);
      return;
    }
    pthread_mutex_unlock(&server_threads[i].lock);
  }
}

void * work_server_fn(void* data)
{
  char file_buffer[FILE_BUFFER_SIZE];
  //  char msg[10];
  int total_writed = 0;
  int received = 0;
  struct msg msg = {};
  struct server_data *srv_data = (struct server_data *)data;
  srv_data->thread_id = gettid();

  printf("(%s)\t W1 Worker running ...t(%d)\n", __func__, srv_data->thread_id);
  while (1) {
    // for (int i = 0; i < MAX_SRV_THREADS; i++) {
    pthread_mutex_trylock(&srv_data->lock);
    printf("(%s)\t W2 Worker waiting, thread/fd (%d)/(%d)\n", __func__,
           srv_data->thread_id, srv_data->fd);
    pthread_cond_wait(&srv_data->cond, &srv_data->lock);
    printf("(%s)\t W3 Worker running, thread/fd (%d)/(%d)received signal\n",
           __func__, srv_data->thread_id, srv_data->fd);
    if (srv_data->fd != -1) {
      char file_name[20];
      sprintf(file_name, "%d_%d.txt", srv_data->thread_id, file_number++);
      // int file = open(file_name, O_CREAT | O_WRONLY, S_IRUSR);
      FILE *file = fopen(file_name, "w");
      setvbuf(file, file_buffer, _IOFBF, FILE_BUFFER_SIZE);
      printf("(%s)\t W4-1 Received in thread/fd (%d)/(%d)\n", __func__,
             srv_data->thread_id, srv_data->fd);
      while ((received = read(srv_data->fd, &msg, sizeof(msg))) > 0) {
        printf("(%s)\t Received/Size (%d)/(%d) file (%s) thread/fd (%d)/(%d)\n",
               __func__, received, sizeof(msg), file_name, srv_data->thread_id,
               srv_data->fd);

        // write(file, msg, sizeof(msg));
        if (msg.size == 0) { // End of message
          printf("(%s)\t Received zero size writed (%d) file (%s) thread/fd "
                 "(%d)/(%d)\n",
                 __func__, total_writed, file_name, srv_data->thread_id,
                 srv_data->fd);
          break;
        }
	int xtra_received=0;
        while (received < sizeof(msg)) { // Msg need extra read
          xtra_received = read(srv_data->fd, ((char*)&msg )+ received,
                           sizeof(msg) - received);
	  if(xtra_received <=0){
          printf("(%s)\t XTRA break Received/Xtra/Size (%d)/(%d)/(%d) file (%s) thread/fd "
                 "(%d)/(%d) - (%s)\n",
                 __func__, received, xtra_received, msg.size, file_name, srv_data->thread_id,
                 srv_data->fd, strerror(errno));
		  break;
	  }
	  received+=xtra_received;
          printf("(%s)\t XTRA Received/Size (%d)/(%d) file (%s) thread/fd "
                 "(%d)/(%d)\n",
                 __func__, received, sizeof(msg), file_name, srv_data->thread_id,
                 srv_data->fd);
        }
        fwrite(&(msg.data), 1, sizeof(msg.data), file);
        total_writed += received;
      }
      printf("(%s)\t Total writed (%d) file (%s) thread/fd (%d)/(%d)\n",
             __func__, total_writed, file_name, srv_data->thread_id,
             srv_data->fd);
      total_writed = 0;
      size_t bufsize;
      bufsize = __fbufsize(file);
      printf("(%s)\t Buffer size (%d) file (%s) thread/fd (%d)/(%d) \n",
             __func__, bufsize, file_name, srv_data->thread_id, srv_data->fd);

      // Send ack response
      char server_response[MAX_SRV_RESPONSE] = OK_SRV_RESPONSE;
      write(srv_data->fd, server_response, MAX_SRV_RESPONSE);
      // close(file);
      fclose(file);        // close disk file
      close(srv_data->fd); // close socket
      srv_data->fd = -1;
      printf("(%s)\t W5 Closing file (%s) thread/fd (%d)/(%d)\n", __func__,
             file_name, srv_data->thread_id, srv_data->fd);
    } else {
      printf("(%s)\t W4-2 Somethin ko thread/fd (%d)/(%d)\n", __func__,
             srv_data->thread_id, srv_data->fd);
    }
    pthread_mutex_unlock(&srv_data->lock);
    //}
  }
}

void create_threads(void)
{

  for (int i = 0; i < MAX_SRV_THREADS; i++) {
    pthread_mutex_init(&server_threads[i].lock, NULL);
    pthread_cond_init(&server_threads[i].cond, NULL);
    server_threads[i].fd = -1;
    pthread_create(&server_threads[i].thread, NULL, work_server_fn,
                   (void *)&server_threads[i]);
  }
}

int main(int argc, char* argv[])
{

  struct sockaddr_in serv_addr;
  int srv_fd;
  int accept_fd;

  create_threads();

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SRV_PORT);

  srv_fd = socket(AF_INET, SOCK_STREAM, 0);
  bind(srv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  listen(srv_fd, MAX_BACKLOG);

  while (1) {
    if (is_worker_free()) {
      accept_fd = accept(srv_fd, (struct sockaddr *)NULL, NULL);
      printf("(%s) Accepting fd (%d) \n", __func__, accept_fd);
      // MTU change
      struct ifreq ifr;
      ifr.ifr_addr.sa_family = AF_INET; // address family
      strncpy(
          ifr.ifr_name, "ens8u2u2",
          sizeof(ifr.ifr_name)); // interface name where you want to set the MTU
      ifr.ifr_mtu = 9*1024;        // your MTU size here
      if (ioctl(accept_fd, SIOCSIFMTU, (caddr_t)&ifr) < 0)
        printf("(%s) Failed to set MTU to (%d) \n", __func__, ifr.ifr_mtu);
      // socket buffers change
      int recvBuff = 512 * 1024;
      int sendBuff = 512 * 1024;
      if (setsockopt(accept_fd, SOL_SOCKET, SO_RCVBUF, &recvBuff,
                     sizeof(recvBuff)) < 0) {
        printf("(%s) Error setsockopt (%s)\n", __func__, strerror(errno));
      }
      if (setsockopt(accept_fd, SOL_SOCKET, SO_SNDBUF, &sendBuff,
                     sizeof(sendBuff)) < 0) {
        printf("(%s) Error setsockopt (%s)\n", __func__, strerror(errno));
      }
      assing_job(accept_fd);
    } else {
      printf("(%s) Waiting for free workers \n", __func__);
      sleep(1);
    }
  }

  return 0;
}
