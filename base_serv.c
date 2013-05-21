/*
**  Copyright (C) 2013 - Jonathan Salwan - http://twitter.com/JonathanSalwan
** 
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
** 
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
**
**  $ gcc -W -Wall -Wextra -ansi -pedantic -D_BSD_SOURCE -D_POSIX_SOURCE \
**  -lpthread -std=c99 -o base_serv ./base_serv.c
**
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERV_PORT         1024
#define MAX_CLIENT        256
#define MAX_CLIENT_QUEUE  32

#define TRUE      0
#define FALSE     !TRUE
#define SUCCESS   0
#define ERROR     !SUCCESS

#define __PRIVATE__ 
#define __API__ 

typedef struct client_s {
  unsigned char       buff[256];
  int32_t             fd;
  struct sockaddr_in  addr;
  struct client_s     *next;
  struct client_s     *prev;
} client_t;

typedef struct api_s {
  void(*addClient)(client_t*);
  void(*delClient)(client_t*);
} api_t;

typedef struct coreServ_s {
  /* Attributs */
  int32_t             fd;
  int32_t             on;
  client_t            *clients;
  client_t            *fclient;
  api_t               api;
  struct sockaddr_in  addr;
  struct timeval      timeout;
  pthread_t           threadConHandler;
  pthread_t           threadMoniClient;
  pthread_mutex_t     mutex;
  fd_set              readfs;
  /* Methodes */
  void(*initSocket)(void);
  void(*startServ)(void);
  void*(*waitingClient)(void*);
  void*(*monitoringClient)(void*);
} coreServ_t;

static void __PRIVATE__  initSockServ(void);
static void __PRIVATE__  startServ(void);
static void __PRIVATE__  *servWaitingClient(void*);
static void __PRIVATE__  *monitoringClient(void*);
static void __API__      addClient(client_t *newCli);
static void __API__      delClient(client_t *newCli);

coreServ_t coreServ = {
  .fd               = 0,
  .clients          = NULL,
  .initSocket       = __PRIVATE__  initSockServ,
  .startServ        = __PRIVATE__  startServ,
  .waitingClient    = __PRIVATE__  servWaitingClient,
  .monitoringClient = __PRIVATE__  monitoringClient,
  .api = {
    .delClient      = __API__      delClient,
    .addClient      = __API__      addClient,
  },
  .timeout = {
    .tv_usec        = 0,
    .tv_sec         = 1,
  }
};

static void __PRIVATE__ initSockServ(void){
  if ((coreServ.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("initSockServ.socket");
    exit(-1);
  }
  if (setsockopt(coreServ.fd, SOL_SOCKET, SO_REUSEADDR, (char*)&coreServ.on, sizeof(int)) < 0){
    perror("initSockServ.setsockopt");
    close(coreServ.fd);
    exit(-1);
  }
  memset(&coreServ.addr, 0x00, sizeof(struct sockaddr_in));
  coreServ.addr.sin_family      = AF_INET;
  coreServ.addr.sin_port        = htons(SERV_PORT);
  coreServ.addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

static void __API__ addClient(client_t *newCli){
  pthread_mutex_lock(&coreServ.mutex);
  if (coreServ.clients != NULL){
    newCli->next = coreServ.clients->next;
    newCli->prev = coreServ.clients; 
    coreServ.clients->next = newCli;
    coreServ.fclient->prev = newCli;
    coreServ.clients = newCli;
  }
  else {
    newCli->next = newCli;
    newCli->prev = newCli;
    coreServ.clients = newCli;
    coreServ.fclient = newCli;
  }
  pthread_mutex_unlock(&coreServ.mutex);
}

static void __API__ delClient(client_t *cli){
  if (cli->next == cli && cli->prev == cli){
    coreServ.clients = NULL;
    coreServ.fclient = NULL;
  }
  else {
    cli->prev->next = cli->next;
    cli->next->prev = cli->prev;
    if (cli == coreServ.fclient){
      coreServ.fclient = cli->next;
    }
    if (cli == coreServ.clients){
      coreServ.clients = cli->next;
    }
  }
  free(cli);
}

static void __PRIVATE__ *servWaitingClient(void *args){
  struct sockaddr_in  cliAddr;
  int32_t             fdCli;
  uint32_t            sinSize;
  client_t            *newClient; 

  (void)args;
  sinSize = sizeof(struct sockaddr_in);
  if (bind(coreServ.fd, (struct sockaddr *)&coreServ.addr, sizeof(coreServ.addr)) < 0){
    perror("ServWaitingClient.bind");
    close(coreServ.fd);
    exit(-1);
  }
  if (listen(coreServ.fd, MAX_CLIENT_QUEUE) < 0){
    perror("ServWaitingClient.listen");
    close(coreServ.fd);
    exit(-1);
  }
  while (1){
    if ((fdCli = accept(coreServ.fd, (struct sockaddr *)&cliAddr, &sinSize)) < 0){
      perror("ServWaitingClient.accept");
      close(coreServ.fd);
      exit(-1);
    }
    if ((newClient = malloc(sizeof(client_t))) == NULL){
      perror("ServWaitingClient.malloc");
      continue;
    }
    newClient->fd = fdCli;
    newClient->addr = cliAddr;
    coreServ.api.addClient(newClient);
  }
  /* never go here */
  pthread_exit(0);
}

static void __PRIVATE__ fdSetAll(void){
  client_t *ptr;
  int       stop = 0;

  pthread_mutex_lock(&coreServ.mutex);
  ptr = coreServ.clients;
  FD_ZERO(&coreServ.readfs);
  while (ptr && ptr->fd != stop){
    if (stop == 0)
      stop = ptr->fd;
    FD_SET(ptr->fd, &coreServ.readfs);
    ptr = ptr->next;
  }
  pthread_mutex_unlock(&coreServ.mutex);
}

static void __PRIVATE__ fdIssetAll(void){
  client_t *ptr;
  int       stop = 0;
  int       ret = 0;

  pthread_mutex_lock(&coreServ.mutex);
  ptr = coreServ.clients;
  while (ptr && ptr->fd != stop){
    if (stop == 0)
      stop = ptr->fd;
    if (FD_ISSET(ptr->fd, &coreServ.readfs)){
      memset(ptr->buff, 0x00, sizeof(ptr->buff));
      ret = read(ptr->fd, ptr->buff, sizeof(ptr->buff)-1);
      if (ret <= 0){
        printf("Close (%d)\n", ptr->fd);
        if (ptr->fd == stop)
          stop = ptr->next->fd;
        coreServ.api.delClient(ptr);
      }
      else {
        /* Just change that call */
        printf("ret=%d | Client (%d): %s", ret, ptr->fd, ptr->buff);
      }
    }
    ptr = ptr->next;
  }
  pthread_mutex_unlock(&coreServ.mutex);
}

static int __PRIVATE__ getMaxFd(void){
  client_t *ptr;
  int       max;

  pthread_mutex_lock(&coreServ.mutex);
  ptr = coreServ.clients;
  max = (ptr == 0) ? coreServ.fd : coreServ.clients->fd;
  while (ptr != coreServ.fclient){
    if (ptr->fd > max)
      max = ptr->fd;
    ptr = ptr->next;
  }
  pthread_mutex_unlock(&coreServ.mutex);
  return max;
}

static void __PRIVATE__ *monitoringClient(void *args){
  client_t *ptr = NULL;
  int      ret  = 0; 
  
  (void)args;
  while(1){
    if (ptr){
      fdSetAll();
      if ((ret = select(getMaxFd() + 1, &coreServ.readfs, NULL, NULL, &coreServ.timeout)) == -1){
        perror("monitoringClient.select");
        close(coreServ.fd);
        exit(-1);
      } 
      else if (ret > 0){
        fdIssetAll();
      }
      ptr = ptr->next;
    }
    else {
      ptr = coreServ.clients;
    }
    usleep(500);
  }
  /* never go here */
  pthread_exit(0);
}

static void __PRIVATE__ startServ(void){
  void *ret;

  if (pthread_create(&coreServ.threadConHandler, NULL, coreServ.waitingClient, NULL) < 0){
    perror("startServ.pthread_create (1)");
    exit(1);
  }
  if (pthread_create(&coreServ.threadMoniClient, NULL, coreServ.monitoringClient, NULL) < 0){
    perror("startServ.pthread_create (2)");
    exit(1);
  }
  (void)pthread_join(coreServ.threadConHandler, &ret);
  (void)pthread_join(coreServ.threadMoniClient, &ret);
}

int main(int ac, const char *av[]){
  (void)ac;
  (void)av;
  
  coreServ.initSocket();
  coreServ.startServ();

  return 0;
}
