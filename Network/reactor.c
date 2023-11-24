#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_LENGTH       4096
#define MAX_EPOLL_EVENTS    1024
#define SERVER_PORT         8888

typedef int (*NCALLBACK)(int, int, void*);  //注意与用 NCALLBACK() 的区别

/*描述就绪文件描述符的相关信息*/
struct ccevent {
    int fd;
    int events;
    void* arg;
    int (*callback)(int fd, int events, void* arg);

    int status;
    char buffer[BUFFER_LENGTH];
    int length;
    long last_active;
};

struct ccreactor {
    int epfd;
    struct ccevent* events;
};


int recv_cb(int fd, int events, void* arg);
int send_cb(int fd, int events, void* arg);

/*****************************************************************************
 * 函 数 名 : cc_event_set
 * 函数功能 : 自定义事件
 * 输入参数 : struct ccevent* ev 
 int fd 
 NCALLBACK callback fd的回调函数
 void* arg  额外参数项

 * 输出参数 : 无
 * 其   它  : 在封装这个事件的时候，为这个事件指明了回调函数，一般来说，一个fd只对
 *            一个特定的事件感兴趣，当这个事件发生的时候，就调用这个回调函数。
 * 
 *****************************************************************************/
void cc_event_set(struct ccevent* ev, int fd, NCALLBACK callback, void* arg) {

    ev->fd = fd;
    ev->callback = callback;
    ev->events = 0;
    ev->arg = arg;
    ev->last_active = time(NULL);  //调用eventset函数的时间

    return;
}

/*****************************************************************************
 * 函 数 名 : cc_event_add
 * 函数功能 : 向 epoll监听的红黑树 添加一个文件描述符
 * 输入参数 : int epfd  epoll实例对应的文件描述符
 int events  
 struct ccevent* ev

 * 输出参数 : 
 * 其   它  : 

 *****************************************************************************/
int cc_event_add(int epfd, int events, struct ccevent* ev) {

    struct epoll_event ep_ev = {0, {0}};  //??
    ep_ev.data.ptr = ev;  //ptr指向一个结构体（之前的epoll模型红黑树上挂载的是文件描述符cfd和lfd，现在是ptr指针）
    ep_ev.events = ev->events = events;  //EPOLLIN 或 EPOLLOUT

    int op;
    if (ev->status == 1) {  //status 说明文件描述符是否在红黑树上 0不在，1 在
        op = EPOLL_CTL_MOD;
    } else {                    
        op = EPOLL_CTL_ADD;  //将其加入红黑树 g_efd, 并将status置1
        ev->status = 1;
    }

    /*epfd:epoll实例对应的文件描述符; op:操作; fd:要检测的文件描述符; fd什么事件*/
    if (epoll_ctl(epfd, op, ev->fd, &ep_ev) < 0) {
        printf("event add failed [fd=%d], events[%d]\n\n", ev->fd, events);
        return -1;
    }

    return 0;
}

/*****************************************************************************
 * 函 数 名 : cc_event_del
 * 函数功能 : 从epoll 监听的 红黑树中删除一个文件描述符
 * 输入参数 : int epfd  epoll实例对应的文件描述符
 struct ccevent* ev

 * 输出参数 : 
 * 其   它  : 

 *****************************************************************************/
int cc_event_del(int epfd, struct ccevent* ev) {

    struct epoll_event ep_ev = {0, {0}};

    if (ev->status != 1) {  //如果fd没有添加到监听树上，就不用删除，直接返回
        return -1;
    }

    ep_ev.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &ep_ev);

    return 0;
}


/*****************************************************************************
 * 函 数 名 : recv_cb 接收事件回调函数
 * 函数功能 : 读取客户端发过来的数据
 * 输入参数 : int fd  
 int events
 void *arg

 * 输出参数 : 
 * 其   它  : 

 *****************************************************************************/
int recv_cb(int fd, int events, void *arg) {

    struct ccreactor* reactor = (struct ccreactor* )arg;
    struct ccevent* ev = reactor->events + fd;

    int len = recv(fd, ev->buffer, BUFFER_LENGTH, 0);
    cc_event_del(reactor->epfd, ev);

    if (len > 0) {

        ev->length = len;
        ev->buffer[len] = '\0';  //手动添加字符串结束标记

        printf("C[%d]:%s\n", fd, ev->buffer);

        cc_event_set(ev, fd, send_cb, reactor);  //设置该fd对应的回调函数为senddata 
        cc_event_add(reactor->epfd, EPOLLOUT, ev);  //将fd加入红黑树epfd中,监听其写事件    

    } else if (len == 0) {

        close(ev->fd);
        ///* ev-g_events 地址相减得到偏移元素位置 */
        printf("[fd=%d] pos[%ld], closed\n", fd, ev-reactor->events);

    } else {

        close(ev->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));

    }

    return len;
}


/*****************************************************************************
 * 函 数 名 : send_cb 发送事件回调函数
 * 函数功能 : 发送给客户端数据
 * 输入参数 : int fd  
              int events
              void *arg

 * 输出参数 : 
 * 其   它  : 

*****************************************************************************/

int send_cb(int fd, int events, void* arg) {

    struct ccreactor* reactor = (struct ccreactor* )arg;
    struct ccevent* ev = reactor->events + fd;

    int len = send(fd, ev->buffer, ev->length, 0);  //直接将数据回射给客户端

    if (len > 0) {

        printf("send[fd=%d], [%d]%s\n", fd, len, ev->buffer);  

        cc_event_del(reactor->epfd, ev);  //从红黑树epfd中移除
        cc_event_set(ev, fd, recv_cb, reactor);  //将该fd的回调函数改为recv_cb
        cc_event_add(reactor->epfd, EPOLLIN, ev);  //重新添加到红黑树上,设为监听读事件

    } else {

        close(ev->fd);

        cc_event_del(reactor->epfd, ev);
        printf("send[fd=%d] error %s\n", fd, strerror(errno));

    }

    return len;
}


/*****************************************************************************
 * 函 数 名 : accept_cb 
 * 函数功能 : 当有文件描述符listenfd就绪, epoll返回, 调用该函数与客户端建立链接
 * 输入参数 : int fd  
 int events
 void *arg

 * 输出参数 : 
 * 其   它  : listenfd 只能由accept处理

 *****************************************************************************/
int accept_cb(int fd, int events, void* arg) {

    struct ccreactor *reactor = (struct ccreactor*)arg;
    if (reactor == NULL) return -1;

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int clientfd;

    if ((clientfd = accept(fd, (struct sockaddr* )&client_addr, &len)) == -1) {
        if (errno != EAGAIN && errno != EINTR) {
            sleep(1);
        }

        printf("accept: %s\n", strerror(errno));
        return -1;
    }

    int i = 0;
    do {
        for (i = 3; i < MAX_EPOLL_EVENTS; i ++) {  //从全局数组g_events中找一个空闲元素，类似于select中找值为-1的元素
            if (reactor->events[i].status == 0) {
                break;
            }
        }
        if (i == MAX_EPOLL_EVENTS) {   // 超出连接数上限
            printf("%s: max connect limit[%d]\n", __func__, MAX_EPOLL_EVENTS);
            break;
        }

        int flag = 0;
        if ((flag = fcntl(clientfd, F_SETFL, O_NONBLOCK)) < 0) {  //将clientfd也设置为非阻塞
            printf("%s: fcntl nonblocking failed, %d\n", __func__, MAX_EPOLL_EVENTS);
            break;
        }

        cc_event_set(&reactor->events[clientfd], clientfd, recv_cb, reactor);  //找到合适的节点之后，将其添加到监听树中，并监听读事件
        cc_event_add(reactor->epfd, EPOLLIN, &reactor->events[clientfd]);
    } while (0);


    printf("new connect [%s:%d][time:%ld], pos[%d]\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), reactor->events[i].last_active, i);

    return 0;
}


/*****************************************************************************
 * 函 数 名 : init_sock 
 * 函数功能 : 初始化socket， 初始化fd
 * 输入参数 : short port  

 * 输出参数 : 
 * 其   它  : listenfd 只能由accept处理

 *****************************************************************************/
int init_sock(short port) {

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK);  //将socket设为非阻塞

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (listen(fd, 20) < 0) {
        printf("listen failed : %s\n", strerror(errno));
    }

    return fd;
}

int ccreactor_init(struct ccreactor* reactor) {

    if (reactor == NULL) return -1;
    memset(reactor, 0, sizeof(struct ccreactor));

    reactor->epfd = epoll_create(1);  //创建红黑树,返回给全局 epfd
    if (reactor->epfd <= 0) {
        printf("create epfd in %s err %s\n", __func__, strerror(errno));
        return -2;
    }

    reactor->events = (struct ccevent*)malloc((MAX_EPOLL_EVENTS)* sizeof(struct ccevent));
    if (reactor->events == NULL) {
        printf("create epfd in %s err %s\n", __func__, strerror(errno));
        close(reactor->epfd);
        return -3;
    }
}

int ccreactor_destory(struct ccreactor* reactor) {

    close(reactor->epfd);
    free(reactor->events);

}

int ccreactor_addlistener(struct ccreactor *reactor, int sockfd, NCALLBACK acceptor) {

    if (reactor == NULL) return -1;
    if (reactor->events == NULL) return -1;

    cc_event_set(&reactor->events[sockfd], sockfd, acceptor, reactor);
    cc_event_add(reactor->epfd, EPOLLIN, &reactor->events[sockfd]);  //将lfd添加到监听树上，监听读事件

    return 0;
}

int ccreactor_run(struct ccreactor *reactor) {
    if (reactor == NULL) return -1;
    if (reactor->epfd < 0) return -1;
    if (reactor->events == NULL) return -1;

    struct epoll_event events[MAX_EPOLL_EVENTS+1];  //结构体数组，用来接收epoll_wait传出的满足监听事件的fd结构体

    int checkpos = 0, i;

    while (1) {

        long now = time(NULL);
        for (i = 0;i < 100;i ++, checkpos ++) {
            if (checkpos == MAX_EPOLL_EVENTS) {
                checkpos = 0;
            }

            if (reactor->events[checkpos].status != 1) {
                continue;
            }

            long duration = now - reactor->events[checkpos].last_active;

            if (duration >= 60) {
                close(reactor->events[checkpos].fd);
                printf("[fd=%d] timeout\n", reactor->events[checkpos].fd);
                cc_event_del(reactor->epfd, &reactor->events[checkpos]);
            }
        }

        //调用eppoll_wait等待接入的客户端事件,epoll_wait传出的是满足监听条件的那些fd的struct epoll_event类型
        int nready = epoll_wait(reactor->epfd, events, MAX_EPOLL_EVENTS, 1000);
        if (nready < 0) {
            printf("epoll_wait error, exit\n");
            continue;
        }

        for (i = 0;i < nready;i ++) {
            //cc_event_add函数中，添加到监听树中监听事件的时候将ccevent* 结构体类型给了ptr指针
            //这里epoll_wait返回的时候，同样会返回对应fd的ccevent* 类型的指针
            struct ccevent *ev = (struct ccevent*)events[i].data.ptr;
            
            //如果监听的是读事件，并返回的是读事件
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) {
                ev->callback(ev->fd, events[i].events, ev->arg);
            }
            //如果监听的是写事件，并返回的是写事件
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) {
                ev->callback(ev->fd, events[i].events, ev->arg);
            }

        }

    }
}


int main(int argc, char* argv[]) {

    unsigned short port = SERVER_PORT;
    if (argc == 2) {
        port = atoi(argv[1]);
    }

    int sockfd = init_sock(port);

    struct ccreactor* reactor = (struct ccreactor* )malloc(sizeof(struct ccreactor));
    ccreactor_init(reactor);

    ccreactor_addlistener(reactor, sockfd, accept_cb);
    ccreactor_run(reactor);

    ccreactor_destory(reactor);
    close(sockfd);

    return 0;
}


