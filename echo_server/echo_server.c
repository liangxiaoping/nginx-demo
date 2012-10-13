#include <sys/socket.h>  
#include <sys/epoll.h> 
#include <sys/poll.h> 
#include <netinet/in.h> 
#include <netinet/tcp.h> 
#include <arpa/inet.h>
#include <signal.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> 


#define kMaxEventSize        500
#define kMaxBufferLen        128
#define kLengthOfListenQueue 5
#define kEpollTimetOut       1000

/* status in epoll wait list */
#define kNew     -1
#define kAdded   1
#define kDeleted 2

typedef struct epoll_event epoll_event_t;
typedef struct channel_s channel_t;
typedef struct event_loop_s event_loop_t;
typedef void (*event_callback)(channel_t* channel);

struct event_loop_s
{
  int epollfd;
  int nevents;
  epoll_event_t* epoll_events; 
  channel_t* channel_list;
};

struct channel_s 
{
  event_loop_t*  loop;
  int            socketfd;
  int            events;    /* for epoll_ctl */
  int            revents;   /* from epoll_wait */
  int            status;
  char           inbuf[kMaxBufferLen];
  char           outbuf[kMaxBufferLen];
  channel_t*     next;
  event_callback read_callback;
  event_callback write_callback;
  event_callback close_callback;
  event_callback error_callback;
};

channel_t* channel_create(event_loop_t* loop, int sfd, int events, int revents, int status)
{
  channel_t* channel = (channel_t*)malloc(sizeof(channel_t));
  if(channel == NULL)
  {
    printf( "channel_create failed at : %s line %u:.\r\n", __func__, __LINE__ );
    return NULL;
  }

  memset(channel, 0x0, sizeof(channel_t));

  channel->loop      = loop;
  channel->socketfd  = sfd;
  channel->events    = events;
  channel->revents   = revents;
  channel->status    = status;

  return channel;
}

void channel_dump(const channel_t* channel)
{
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 

  printf("socketfd       = %d . \r\n", channel->socketfd);
  printf("events         = %d . \r\n", channel->events);
  printf("status         = %d . \r\n", channel->status);

  if (channel->revents & EPOLLIN)
    printf("revents        = %s . \r\n", "EPOLLIN");
  if (channel->revents & EPOLLPRI)
    printf("revents        = %s . \r\n", "EPOLLPRI");
  if (channel->revents & EPOLLOUT)
    printf("revents        = %s . \r\n", "EPOLLOUT");
  if (channel->revents & EPOLLHUP)
    printf("revents        = %s . \r\n", "EPOLLHUP");
  if (channel->revents & EPOLLRDHUP)
    printf("revents        = %s . \r\n", "EPOLLRDHUP");
  if (channel->revents & EPOLLERR)
    printf("revents        = %s . \r\n", "EPOLLERR");
  if (channel->revents & POLLNVAL)
    printf("revents        = %s . \r\n", "POLLNVAL");
}

void channel_set_readcallback(channel_t* channel, event_callback cb)
{
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 
  channel->read_callback = cb;
}

void channel_set_writecallback(channel_t* channel, event_callback cb)
{
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 
  channel->write_callback = cb;
}

void channel_set_closecallback(channel_t* channel, event_callback cb)
{
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 
  channel->close_callback = cb;
}

void channel_set_errorcallback(channel_t* channel, event_callback cb)
{
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 
  channel->error_callback = cb;
}

void channel_event_handle(channel_t* channel)
{
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  }  

  if (channel->revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
  {
    if (channel->read_callback) channel->read_callback(channel);
  }
  
  if (channel->revents & EPOLLOUT)
  {
    if (channel->write_callback) channel->write_callback(channel);
  }
  
  if ((channel->revents & EPOLLHUP) && !(channel->revents & EPOLLIN))
  {
    if (channel->close_callback) channel->close_callback(channel);
  }

  if (channel->revents & (EPOLLERR | POLLNVAL))
  {
    if (channel->error_callback) channel->error_callback(channel);
  }
}

void channel_epollctl(int op, channel_t* channel)
{
  printf("Enter function: %s.\r\n", __func__);

  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 
  
  struct epoll_event epv = {0, {0}};   
  epv.data.ptr = channel;
  epv.events   = channel->events;

  if(epoll_ctl(channel->loop->epollfd, op, channel->socketfd, &epv) < 0)
  {
    perror("epoll_ctl");
    printf("epoll_ctl failed[epollfd = %d, socketfd = %d]\n", channel->loop->epollfd, channel->socketfd); 
  }
}

// EPOLL_CTL_ADD/MOD
void channel_epollctl_update(channel_t* channel)
{
  printf("Enter function: %s.\r\n", __func__);
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  }
  
  if(channel->status == kAdded)
  {
    channel_epollctl(EPOLL_CTL_MOD, channel);
  }
  else
  {
    channel_epollctl(EPOLL_CTL_ADD, channel);
    channel->status = kAdded;
  }
}

// EPOLL_CTL_DEL
void channel_epollctl_remove(channel_t* channel)
{
  printf("Enter function: %s.\r\n", __func__);
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  }

  channel_epollctl(EPOLL_CTL_DEL, channel);
  channel->status = kDeleted;
}

void channel_destory(channel_t* channel)
{
  if(channel != NULL)
  {
    channel_epollctl_remove(channel);
    close(channel->socketfd);
    free(channel);
    channel = NULL;
  }
}

void event_loop_init(event_loop_t* loop)
{
  if(loop == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 

  memset(loop, 0x0, sizeof(event_loop_t));

  loop->epollfd   = epoll_create(kMaxEventSize);  
  if(loop->epollfd == -1) 
  {
    perror("epoll_create");
    return;
  }

  loop->nevents         = kMaxEventSize;
  loop->epoll_events    = (epoll_event_t *)malloc(loop->nevents * sizeof(epoll_event_t));
  if(loop->epoll_events == NULL) 
  {
    printf( "malloc failed at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  }
}

void event_loop_destory(event_loop_t* loop)
{
  channel_t* channel = NULL;
  if(loop == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  }

  free(loop->epoll_events);

  while(loop->channel_list)
  {
    channel = loop->channel_list->next;
    channel_destory(loop->channel_list);
    loop->channel_list = channel;
  }
  close(loop->epollfd);
}

void net_fd_close(int fd)
{
  if(fd > 0)
    close(fd);
}

void net_fd_close_onexec(int fd)
{
  int ret   = -1;
  int flags = fcntl(fd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = fcntl(fd, F_SETFD, flags);
  if(ret < 0)
    perror("fcntl error");
}

void net_fd_set_nonblock(int fd)
{
  int ret   = -1;
  int flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  ret = fcntl(fd, F_SETFL, flags);
  if(ret < 0)
    perror("fcntl error");
}

void net_fd_set_reuseaddr(int fd)
{
  int optval = 1;
  int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if(ret < 0)
    perror("setsockopt error");
}

void net_fd_set_tcp_nodelay(int fd)
{
  int optval = 1;
  int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
  if(ret < 0)
    perror("setsockopt error");
}

void net_fd_set_keepalive(int fd)
{
  int optval = 1;
  int ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
  if(ret < 0)
    perror("setsockopt error");
}

void error_cb(channel_t* channel)
{
  int optval = 1;
  int err    = 0;
  printf("Enter function: %s.\r\n", __func__);
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 

  socklen_t optlen = sizeof(optval);
  if(getsockopt(channel->socketfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    err = errno;
  }
  else
  {
    err = optval;
  }

  printf("Connection closed[err = %d, %s].\r\n", err, strerror(errno));
}

void close_cb(channel_t* channel)
{
  printf("Enter function: %s.\r\n", __func__);
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 

  channel->events = 0;
  channel_epollctl_remove(channel);
  net_fd_close(channel->socketfd);
  printf( "Connections closed, socketfd = %d.\r\n",channel->socketfd);
}

void read_cb(channel_t* channel)
{
  char * inputbuf = NULL;
  printf( "Enter function: %s.\r\n", __func__ );
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 
  
  inputbuf = channel->inbuf;  
  int rlen = read(channel->socketfd, inputbuf, kMaxBufferLen);
  
  if(rlen > 0)
  {
    /* echo */
    int wlen = write(channel->socketfd, inputbuf, rlen);
    if(wlen > 0 && wlen < rlen)
    {
      /* TODO: must append to outputbuffer and enable check EPOLLOUT event */
      printf("wlen = %d, rlen = %d \r\n", wlen, rlen);
    }
    else
    {
      if(wlen < 0 && errno != EWOULDBLOCK)
      {
        perror("read_cb write error.");
      }
    }

    /* quit when press 'q' */
    if(inputbuf[0] == 'q' && (inputbuf[1] == 10 || inputbuf[1] == 13))
    {
      event_loop_destory(channel->loop);
      exit(0);
    }
  }
  else if(rlen == 0)  
  {  
    close_cb(channel);
  }  
  else  
  {
    error_cb(channel);
  }  
}

void write_cb(channel_t* channel)  
{
  char * outputbuf = NULL;
  printf("Enter function: %s.\r\n", __func__);
  if(channel == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 
      
  outputbuf = channel->outbuf;
   
  int len = write(channel->socketfd, outputbuf, kMaxBufferLen);
  if(len < 0)
  {  
    error_cb(channel);
  }

  /*
  TODO:
  if(len == outputBuf->size())
  {
    // if outputbuffer have no data, disable check EPOLLOUT
    int disablewrite = channel->events;
    disablewrite &= ~EPOLLOUT;
    channel_update(channel);
  }
  */
}

void connection_cb(channel_t* accept_channel)  
{
  int           conn_fd      = -1;
  channel_t    *conn_channel = NULL;
  event_loop_t *loop         = NULL;
  struct sockaddr_in sin;

  printf("Enter function: %s.\r\n", __func__);
  if(accept_channel == NULL || accept_channel->loop == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  } 

  loop = accept_channel->loop;

  memset(&sin,  0, sizeof(sin));
  socklen_t len = sizeof(struct sockaddr_in);  
   
  if((conn_fd = accept(accept_channel->socketfd, (struct sockaddr*)&sin, &len)) < 0)  
  { 
    switch (errno)
    {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:
        printf("error of accept, errno = %d.\r\n", errno);
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        printf("unexpected error of accept, errno = %d.\r\n", errno);
        perror("accept error");
        break;
      default:
        printf("unknown error of accept, errno = %d.\r\n", errno);
        perror("accept error");
        break;
    }
    return;  
  }
  
  net_fd_set_tcp_nodelay(conn_fd);
  net_fd_set_keepalive(conn_fd);

  /* level-triggered */
  conn_channel = channel_create(loop, conn_fd, EPOLLIN|EPOLLPRI, 0, kNew);
  if(conn_channel == NULL)
  {
    printf( "channel_create failed at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  }
  channel_set_readcallback(conn_channel, read_cb);
  channel_epollctl_update(conn_channel);
  
  conn_channel->next = loop->channel_list;
  loop->channel_list = conn_channel;
  
  channel_dump(conn_channel);
  printf("new conn[fd:%d][%s:%d]\r\n", conn_fd, inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
}  

int tcpserver_init(short port)  
{
  int ret       = -1;
  int listen_fd = -1;
  struct sockaddr_in sin;

  printf("Enter function: %s.\r\n", __func__);

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(listen_fd < 0)
  {
    perror("socket error");
    return -1;
  }

  net_fd_close_onexec(listen_fd);
  net_fd_set_nonblock(listen_fd);
  net_fd_set_reuseaddr(listen_fd);

  memset(&sin,  0, sizeof(sin));  
  sin.sin_family      = AF_INET;  
  sin.sin_addr.s_addr = INADDR_ANY;  
  sin.sin_port        = htons(port);

  ret = bind(listen_fd, (const struct sockaddr*)&sin, sizeof(sin));
  if( ret < 0)
  {
    perror("bind error");
    return -1;
  }

  ret = listen(listen_fd, kLengthOfListenQueue); 
  if(ret < 0)
  {
    perror("listen error");
    return -1;
  }

  return listen_fd;
}

void tcpserver_runloop(event_loop_t *loop)
{
  int   fds       = -1;
  int   i         = 0;
  
  if(loop == NULL)
  {
    printf( "Abnormal return at : %s line %u:.\r\n", __func__, __LINE__ );
    return;
  }

  while(1)
  {
    memset(loop->epoll_events, 0x0, sizeof(struct epoll_event));
    fds = -1;
    fds = epoll_wait(loop->epollfd, loop->epoll_events, kMaxEventSize, kEpollTimetOut);  
    if(fds < 0)
    {
      perror("epoll_wait error");
      break;  
    }
    
    for(i = 0; i < fds; ++i)
    {  
      channel_t* current_channel = (channel_t*)(loop->epoll_events[i].data.ptr);
      if(current_channel != NULL)
      {
        current_channel->revents = loop->epoll_events[i].events;
        channel_dump(current_channel);
        channel_event_handle(current_channel);
      } 
    }  
  }
}

void fatal(const char* msg) __attribute__ ((noreturn));
void fatal(const char* msg)
{
  perror(msg);
  abort();
}

void broken_pipe()
{
  printf("%s\n", "EPIPE");
  write(1, "EPIPE\n", 6);
}

void ctrl_c()
{
  printf("%s\n", "ctrl+c");
  write(1, "\n", 1);
}

void setsignal(int sig, void (*func)(int))
{
  struct sigaction act;
  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (sigaction(sig, &act, NULL) < 0)
    fatal("sigaction");
}

int main(int argc, char **argv)  
{
  int   listen_fd = -1;
  short port      = 12345;
  channel_t    *accept_channel = NULL;
  event_loop_t  loop;
  
  if(argc == 2)
  {  
    port = atoi(argv[1]);  
  }

  /* signal(SIGPIPE, SIG_IGN); */
  setsignal(SIGINT, ctrl_c);
  setsignal(SIGPIPE, broken_pipe);

  event_loop_init(&loop);

  listen_fd = tcpserver_init(port); 
  if(listen_fd < 0)
  {
    return 1;
  }
  
  printf("echo server is running [listen_fd:port = %d:%d].\r\n", listen_fd, port);

  /* level-triggered */
  accept_channel = channel_create(&loop, listen_fd, EPOLLIN|EPOLLPRI, 0, kNew);
  if(accept_channel == NULL)
  {
    printf( "channel_create failed at : %s line %u:.\r\n", __func__, __LINE__ );
    return 1;
  }
  channel_set_readcallback(accept_channel, connection_cb);
  channel_epollctl_update(accept_channel);
  channel_dump(accept_channel);
  loop.channel_list = accept_channel;
  
  tcpserver_runloop(&loop);
  
  /* free resource */
  event_loop_destory(&loop);
  
  return 0;  
}
