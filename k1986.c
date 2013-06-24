/*
**  Jonathan Salwan - Copyright (C) 2013-26
** 
**  http://twitter.com/JonathanSalwan
**  http://shell-storm.org
** 
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software  Foundation, either  version 3 of  the License, or
**  (at your option) any later version.
**
**  ndh2k13 ctf challenge - The k1986's source code
**
**  $ gcc -W -Wall -Wextra -ansi -pedantic -D_BSD_SOURCE -D_POSIX_SOURCE \
**  -lpthread -std=c99 -o k1986 ./k1986.c
*/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SERV_PORT         1024
#define MAX_CLIENT        256
#define MAX_CLIENT_QUEUE  32

#define TRUE      0
#define FALSE     !TRUE
#define SUCCESS   0
#define ERROR     !SUCCESS

#define RC4_KEY   "\x90\x3f\x8e\x7f\x8a"

#define __PRIVATE__ 
#define __API__ 

#define CALL_PTR(func)                   \
  __asm__("lea 0x0a(%rip), %rax");       \
  __asm__("push %rax");                  \
  __asm__("push %0" :: "l"(func));       \
  __asm__("ret");                        \

#define CALL_FUNC(func)                  \
  __asm__("lea 0x08(%rip), %rax");       \
  __asm__("push %rax");                  \
  __asm__("push %0" :: "l"(func));       \
  __asm__("ret");                        \

#define CALL_ARG_CONST(func, arg1)       \
  __asm__("lea 0x0f(%rip), %rax");       \
  __asm__("push %rax");                  \
  __asm__("mov %0, %%rdi" :: "i"(arg1)); \
  __asm__("push %0" :: "l"(func));       \
  __asm__("ret");

#define BB_BREAK1()                      \
  __asm__("push %rax");                  \
  __asm__("lea 0x02(%rip), %rax");       \
  __asm__("push %rax");                  \
  __asm__("ret");                        \
  __asm__("pop %rax");                    

#define JUNK1()                          \
  __asm__("jmp 2f");                     \
  __asm__("1:");                         \
  __asm__("push %rsi");                  \
  __asm__("mov %rax, %rsi");             \
  __asm__("push %rax");                  \
  __asm__("xor $0x61, %rsi");            \
  __asm__("shr $0x3, %rax");             \
  __asm__("pop %rsi");                   \
  __asm__("pop %rax");                   \
  __asm__("jmp 3f");                     \
  __asm__("2:");                         \
  __asm__("jmp 1b");                     \
  __asm__("3:");

#define JUNK2()                          \
  __asm__("push %rdi");                  \
  __asm__("push %rbx");                  \
  __asm__("xor %rbx, %rdi");             \
  __asm__("shr $0x3, %rbx");             \
  __asm__("pop %rdi");                   \
  __asm__("pop %rbx");

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
  BB_BREAK1();
  JUNK1();
  if ((coreServ.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("socket");
    CALL_ARG_CONST(exit, -1);
  }
  JUNK2();
  BB_BREAK1();
  if (setsockopt(coreServ.fd, SOL_SOCKET, SO_REUSEADDR, (char*)&coreServ.on, sizeof(int)) < 0){
    perror("setsockopt");
    close(coreServ.fd);
    CALL_ARG_CONST(exit, -1);
  }
  BB_BREAK1();
  JUNK1();
  memset(&coreServ.addr, 0x00, sizeof(struct sockaddr_in));
  coreServ.addr.sin_family      = AF_INET;
  coreServ.addr.sin_port        = htons(SERV_PORT);
  JUNK2();
  coreServ.addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

static void __API__ addClient(client_t *newCli){
  pthread_mutex_lock(&coreServ.mutex);
  JUNK1();
  if (coreServ.clients != NULL){
    BB_BREAK1();
    newCli->next = coreServ.clients->next;
    newCli->prev = coreServ.clients; 
    coreServ.clients->next = newCli;
    coreServ.fclient->prev = newCli;
    coreServ.clients = newCli;
  }
  else {
    BB_BREAK1();
    newCli->next = newCli;
    newCli->prev = newCli;
    coreServ.clients = newCli;
    coreServ.fclient = newCli;
  }
  JUNK2();
  pthread_mutex_unlock(&coreServ.mutex);
}

static void __API__ delClient(client_t *cli){
  if (cli->next == cli && cli->prev == cli){
    BB_BREAK1();
    coreServ.clients = NULL;
    coreServ.fclient = NULL;
    JUNK2();
  }
  else {
    BB_BREAK1();
    cli->prev->next = cli->next;
    cli->next->prev = cli->prev;
    if (cli == coreServ.fclient){
      BB_BREAK1();
      coreServ.fclient = cli->next;
    }
    if (cli == coreServ.clients){
      BB_BREAK1();
      JUNK1();
      coreServ.clients = cli->next;
    }
  }
  JUNK1();
  printf("Connexion closed: %s\n", inet_ntoa(cli->addr.sin_addr));
  close(cli->fd);
  JUNK2();
  free(cli);
}

static void __PRIVATE__ *servWaitingClient(void *args){
  struct sockaddr_in  cliAddr;
  int32_t             fdCli;
  uint32_t            sinSize;
  client_t            *newClient; 

  (void)args;
  BB_BREAK1();
  sinSize = sizeof(struct sockaddr_in);
  if (bind(coreServ.fd, (struct sockaddr *)&coreServ.addr, sizeof(coreServ.addr)) < 0){
    BB_BREAK1();
    perror("bind");
    JUNK2();
    close(coreServ.fd);
    CALL_ARG_CONST(exit, -1);
  }
  if (listen(coreServ.fd, MAX_CLIENT_QUEUE) < 0){
    BB_BREAK1();
    perror("listen");
    JUNK1();
    close(coreServ.fd);
    CALL_ARG_CONST(exit, -1);
  }
  while (1){
    if ((fdCli = accept(coreServ.fd, (struct sockaddr *)&cliAddr, &sinSize)) < 0){
      BB_BREAK1();
      perror("accept");
      JUNK1();
      close(coreServ.fd);
      CALL_ARG_CONST(exit, -1);
    }
    printf("Connexion received: %s\n", inet_ntoa(cliAddr.sin_addr));
    if ((newClient = malloc(sizeof(client_t))) == NULL){
      JUNK1();
      BB_BREAK1();
      perror("malloc");
      JUNK1();
      continue;
    }
    BB_BREAK1();
    JUNK2();
    newClient->fd = fdCli;
    newClient->addr = cliAddr;
    JUNK2();
    coreServ.api.addClient(newClient);
  }
  BB_BREAK1();
  JUNK1();
  /* never go here */
  CALL_ARG_CONST(pthread_exit, 0);
}

static void __PRIVATE__ fdSetAll(void){
  client_t *ptr;
  int       stop = 0;

  BB_BREAK1();
  JUNK1();
  pthread_mutex_lock(&coreServ.mutex);
  ptr = coreServ.clients;
  JUNK1();
  FD_ZERO(&coreServ.readfs);
  while (ptr && ptr->fd != stop){
    BB_BREAK1();
    if (stop == 0)
      stop = ptr->fd;
    FD_SET(ptr->fd, &coreServ.readfs);
    ptr = ptr->next;
  }
  JUNK2();
  pthread_mutex_unlock(&coreServ.mutex);
}

static void recvData(int ret, client_t *cli);

static void __PRIVATE__ fdIssetAll(void){
  client_t *ptr;
  int       stop = 0;
  int       ret = 0;

  BB_BREAK1();
  JUNK1();
  pthread_mutex_lock(&coreServ.mutex);
  ptr = coreServ.clients;
  while (ptr && ptr->fd != stop){
    BB_BREAK1();
    if (stop == 0)
      stop = ptr->fd;
    if (FD_ISSET(ptr->fd, &coreServ.readfs)){
      BB_BREAK1();
      memset(ptr->buff, 0x00, sizeof(ptr->buff));
      JUNK2();
      ret = read(ptr->fd, ptr->buff, sizeof(ptr->buff)-1);
      if (ret <= 0){
        BB_BREAK1();
        if (ptr->fd == stop)
          stop = ptr->next->fd;
        coreServ.api.delClient(ptr);
      }
      else {
        BB_BREAK1();
        JUNK2();
        /* Just change that call */
        recvData(ret, ptr);
      }
    }
    ptr = ptr->next;
  }
  JUNK1();
  pthread_mutex_unlock(&coreServ.mutex);
}

static int __PRIVATE__ getMaxFd(void){
  client_t *ptr;
  int       max;

  BB_BREAK1();
  pthread_mutex_lock(&coreServ.mutex);
  JUNK2();
  ptr = coreServ.clients;
  JUNK1();
  max = (ptr == 0) ? coreServ.fd : coreServ.clients->fd;
  JUNK2();
  while (ptr != coreServ.fclient){
    BB_BREAK1();
    if (ptr->fd > max)
      max = ptr->fd;
    ptr = ptr->next;
  }
  pthread_mutex_unlock(&coreServ.mutex);
  JUNK2();
  return max;
}

static void __PRIVATE__ *monitoringClient(void *args){
  client_t *ptr = NULL;
  int      ret  = 0; 
  
  (void)args;
  BB_BREAK1();
  JUNK1();
  while(1){
    BB_BREAK1();
    if (ptr){
      CALL_FUNC(fdSetAll);
      if ((ret = select(getMaxFd() + 1, &coreServ.readfs, NULL, NULL, &coreServ.timeout)) == -1){
        JUNK1();
        BB_BREAK1();
        perror("select");
        JUNK2();
        close(coreServ.fd);
        exit(-1);
      } 
      else if (ret > 0){
        BB_BREAK1();
        JUNK1();
        CALL_FUNC(fdIssetAll);
      }
      ptr = ptr->next;
    }
    else {
      BB_BREAK1();
      JUNK2();
      ptr = coreServ.clients;
    }
    CALL_ARG_CONST(usleep, 500);
  }
  /* never go here */
  CALL_ARG_CONST(pthread_exit, 0);
}

static void __PRIVATE__ startServ(void){
  void *ret;

  BB_BREAK1();
  JUNK1();
  if (pthread_create(&coreServ.threadConHandler, NULL, coreServ.waitingClient, NULL) < 0){
    BB_BREAK1();
    JUNK2();
    perror("pthread_create (1)");
    CALL_ARG_CONST(exit, -1);
  }
  if (pthread_create(&coreServ.threadMoniClient, NULL, coreServ.monitoringClient, NULL) < 0){
    JUNK1();
    BB_BREAK1();
    perror("pthread_create (2)");
    CALL_ARG_CONST(exit, -1);
  }
  BB_BREAK1();
  (void)pthread_join(coreServ.threadConHandler, &ret);
  (void)pthread_join(coreServ.threadMoniClient, &ret);
}

/* EOF API ----------- */


/*
 * RC4 stream cipher
 * Copyright (c) 2002-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 */
#define S_SWAP(a,b) do { uint8_t t = S[a]; S[a] = S[b]; S[b] = t; } while(0)

void rc4_skip(const uint8_t *key, size_t keylen, size_t skip,
        uint8_t *data, size_t data_len)
{
  uint32_t i, j, k;
  uint8_t S[256], *pos;
  int kpos;

  BB_BREAK1();
  /* Setup RC4 state */
  for (i = 0; i < 256; i++)
    S[i] = i;
  j = 0;
  kpos = 0;
  for (i = 0; i < 256; i++) {
    j = (j + S[i] + key[kpos]) & 0xff;
    kpos++;
    if (kpos >= (int)keylen)
      kpos = 0;
    S_SWAP(i, j);
  }
  /* Skip the start of the stream */
  i = j = 0;
  for (k = 0; k < skip; k++) {
    i = (i + 1) & 0xff;
    j = (j + S[i]) & 0xff;
    S_SWAP(i, j);
  }
  /* Apply RC4 to data */
  pos = data;
  for (k = 0; k < data_len; k++) {
    i = (i + 1) & 0xff;
    j = (j + S[i]) & 0xff;
    S_SWAP(i, j);
    *pos++ ^= S[(S[i] + S[j]) & 0xff];
  }
}

void rc4(uint8_t *buf, size_t len, const uint8_t *key, size_t key_len)
{
  BB_BREAK1();
  JUNK2();
  JUNK1();
  JUNK1();
  JUNK2();
  rc4_skip(key, key_len, 0, buf, len);
  JUNK1();
  JUNK1();
  JUNK2();
  JUNK2();
}


struct checkl
{
  unsigned char   *plaintext_old;
  char            entry[40];
  unsigned char   *plaintext;
  unsigned char   *ciphertext;
  int             fd;
  int             i;
  off_t           size;
};


static int checkLicense(int idMember, unsigned char *license){
  struct checkl check;

  if ((check.fd = open("/usr/local/bin/license.db", O_RDONLY)) < 0){
    perror("open");
    return 0; 
  }

  JUNK2();
  check.size = lseek(check.fd, 0, SEEK_END);
  lseek(check.fd, 0, SEEK_SET);
  JUNK1();
  BB_BREAK1();
  if ((check.ciphertext = malloc(check.size+1)) == NULL){
    perror("malloc");
    CALL_ARG_CONST(exit, -1);
  }
  BB_BREAK1();
  JUNK2();
  memset(check.ciphertext, 0x00, check.size+1);
  if ((check.plaintext = malloc(check.size+1)) == NULL){
    perror("malloc");
    CALL_ARG_CONST(exit, -1);
  }
  BB_BREAK1();
  memset(check.plaintext, 0x00, check.size+1);

  read(check.fd, check.ciphertext, check.size);  
  JUNK1();
  close(check.fd);

  BB_BREAK1();
  for (check.i = 0 ; check.i < check.size ; check.i++){
    if (check.i == 0){
      JUNK2();
      check.plaintext[check.i] = check.ciphertext[check.i] ^ 0x12;
      BB_BREAK1();
    }
    else{
      JUNK1();
      check.plaintext[check.i] = check.ciphertext[check.i] ^ check.ciphertext[check.i-1];
      BB_BREAK1();
    }
  }
  JUNK1();
  check.plaintext_old = check.plaintext;
  BB_BREAK1();
  JUNK2();
  sprintf(check.entry, "%02d:%s", idMember, license);
  
  if (strstr((char*)check.plaintext, (char*)check.entry)){
    BB_BREAK1();
    return 1;
  }
    
  return 0;
}

/* proto =  NDH:<id 2 octets en str>:<licence> */
static void recvData(int ret, client_t *cli){
  uint32_t      i;
  int32_t       idMember;
  unsigned char *license;

  BB_BREAK1();
  rc4(cli->buff, sizeof(cli->buff), (uint8_t*)RC4_KEY, strlen(RC4_KEY));
  /* magic number check if buff[0:2] = NDH */
  if ((cli->buff[0] << 1 & 0xff) != 156 || 
      (cli->buff[1] << 2 & 0xff) != 16  ||
      (cli->buff[2] << 3 & 0xff) != 64){
    BB_BREAK1();
    coreServ.api.delClient(cli);
    goto out;
  }

  JUNK1();
  JUNK1();
  JUNK2();
  JUNK1();
  JUNK1();
  JUNK2();

  if ((cli->buff[3] << 4 & 0xff) != 160){
    BB_BREAK1();
    coreServ.api.delClient(cli);
    goto out;
  }
  else
    idMember = atoi((const char *)&cli->buff[4]);
    
  if ((cli->buff[6] << 3 & 0xff) != 208){
    BB_BREAK1();
    JUNK1();
    JUNK2();
    coreServ.api.delClient(cli);
    goto out;
  }
  else
    license = &cli->buff[7];
  
  if (checkLicense(idMember, license) == 1){
    BB_BREAK1();
    write(cli->fd, "True\n", 5);
  }
  else{
    BB_BREAK1();
    write(cli->fd, "False\n", 6);
  }

out:
  return;
}

int main(int ac, const char *av[]){
  (void)ac;
  (void)av;
  
  BB_BREAK1();
  CALL_PTR(coreServ.initSocket);
  CALL_PTR(coreServ.startServ);

  return 0;
}
