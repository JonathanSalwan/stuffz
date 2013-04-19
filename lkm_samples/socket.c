/* 3.2.0 */

#include <linux/net.h>
#include <linux/socket.h>
#include <linux/tcp.h>
#include <net/sock.h>
#include <net/tcp.h>

static struct sockaddr_in       _sin;
static struct msghdr            _msg;
static struct iovec             _iov;
static struct socket            *_sock;

static uint32_t create_address(uint8_t i1, uint8_t i2, uint8_t i3, uint8_t i4)
{
  uint32_t addr = 0;

  addr += i1, addr <<= 8;
  addr += i2, addr <<= 8;
  addr += i3, addr <<= 8;
  addr += i4;

  return (addr);
}

static int connect(void)
{
  int err;

  if (sock_create_kern(PF_INET, SOCK_STREAM, IPPROTO_TCP, &_sock) < 0){
    return -1;
  }

  memset(&_sin, 0, sizeof(_sin));
  _sin.sin_family = AF_INET;
  _sin.sin_addr.s_addr = htonl(create_address(127, 0, 0, 1));
  _sin.sin_port = htons(1234);

  err = _sock->ops->connect(_sock, (struct sockaddr*)&_sin, sizeof(_sin), O_RDWR);
  if (err < 0){
    return -1;
  }
  return 0;
}

static void disconnect(void)
{
  sock_release(_sock);
}

static int send_msg(char *msg, uint32_t len)
{
  int ret;
  mm_segment_t oldfs;

  _msg.msg_name = 0;
  _msg.msg_namelen = 0;
  _msg.msg_iov = &_iov;
  _msg.msg_iovlen = 1;
  _msg.msg_control = NULL;
  _msg.msg_controllen = 0;
  _msg.msg_flags = MSG_DONTWAIT;

  _msg.msg_iov->iov_base = msg;
  _msg.msg_iov->iov_len = len;

  oldfs = get_fs();
  set_fs(KERNEL_DS);
  ret = sock_sendmsg(_sock, &_msg, len);
  if (ret == -1)
    disconnect();
  set_fs(oldfs);

  return ret;
}

static int recv_msg(char *buffer, uint32_t len)
{
  int ret;
  mm_segment_t oldfs;

  _msg.msg_name = 0;
  _msg.msg_namelen = 0;
  _msg.msg_iov = &_iov;
  _msg.msg_iovlen = 1;
  _msg.msg_control = NULL;
  _msg.msg_controllen = 0;
  _msg.msg_flags = MSG_DONTWAIT;

  _msg.msg_iov->iov_base = buffer;
  _msg.msg_iov->iov_len = len;

  oldfs = get_fs();
  set_fs(KERNEL_DS);
  ret = sock_recvmsg(_sock, &_msg, len, 0);
  if (ret == -1)
    disconnect();
  set_fs(oldfs);

 return ret;
}
