#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_TEST 31

int netlink_socket()
{
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFL) | FD_CLOEXEC);

    struct timeval tv = {
        .tv_sec = 3,
        .tv_usec = 0
    };
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    return fd;
}

int netlink_bind(int fd)
{
    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(struct sockaddr_nl));

    addr.nl_family  = AF_NETLINK;
    addr.nl_pad     = 0;
    addr.nl_pid     = getpid();
    addr.nl_groups  = 0;

    return bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_nl));
}

int netlink_send(int fd, void *buf, int len)
{
    int ret = 0;
    struct nlmsghdr *nlh = NULL;

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(struct sockaddr_nl));
    struct iovec iov;
    memset(&iov, 0, sizeof(struct iovec));
    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));

    nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(len));
    if(nlh == NULL)
        return -1;
    memset(nlh, 0, sizeof(NLMSG_SPACE(len)));

    nlh->nlmsg_len      = NLMSG_SPACE(len);
    nlh->nlmsg_type     = 0;
    nlh->nlmsg_flags    = 0;
    nlh->nlmsg_seq      = 0;
    nlh->nlmsg_pid      = getpid();

    memcpy(NLMSG_DATA(nlh), buf, len);

    iov.iov_base    = nlh;
    iov.iov_len     = NLMSG_SPACE(len);

    addr.nl_family  = AF_NETLINK;
    addr.nl_pad     = 0;
    addr.nl_pid     = 0;
    addr.nl_groups  = 0;

    msg.msg_name        = &addr;
    msg.msg_namelen     = sizeof(struct sockaddr_nl);
    msg.msg_iov         = &iov;
    msg.msg_iovlen      = 1;
    msg.msg_control     = NULL;
    msg.msg_controllen  = 0;
    msg.msg_flags       = 0;

    ret = sendmsg(fd, &msg, 0);
    if(nlh) free(nlh);
    return ret;
}

int netlink_recv(int fd, void *buf, int len)
{
    int ret = 0;
    struct nlmsghdr *nlh = NULL;

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(struct sockaddr_nl));
    struct iovec iov;
    memset(&iov, 0, sizeof(struct iovec));
    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));

    nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(len));
    if(nlh == NULL)
        return -1;
    memset(nlh, 0, sizeof(NLMSG_SPACE(len)));

    iov.iov_base    = nlh;
    iov.iov_len     = NLMSG_SPACE(len);

    msg.msg_name        = &addr;
    msg.msg_namelen     = sizeof(struct sockaddr_nl);
    msg.msg_iov         = &iov;
    msg.msg_iovlen      = 1;
    msg.msg_control     = NULL;
    msg.msg_controllen  = 0;
    msg.msg_flags       = 0;

    ret = recvmsg(fd, &msg, 0);
    memcpy(buf, NLMSG_DATA(nlh), len);
    if(nlh) free(nlh);
    return ret;
}

void netlink_close(int fd)
{
    if(fd > 0) close(fd);
}

int main(int argc, char *argv[])
{
    int rv = 0;
    int fd = 0;
    char buf2send[] = "hello, kernel!";
    char buf2recv[2048] = {0};

    fd = netlink_socket();
    if(fd == -1)
    {
        fprintf(stderr, "%s:%d - %d:%s\n", __FILE__, __LINE__, errno, strerror(errno));
        goto ErrP;
    }

    rv = netlink_bind(fd);
    if(rv == -1)
    {
        fprintf(stderr, "%s:%d - %d:%s\n", __FILE__, __LINE__, errno, strerror(errno));
        goto ErrP;
    }

do{
    rv = netlink_send(fd, buf2send, strlen(buf2send));
    if(rv == -1)
    {
        fprintf(stderr, "%s:%d - %d:%s\n", __FILE__, __LINE__, errno, strerror(errno));
        goto ErrP;
    }
    fprintf(stdout, "send data to kernel succeed - \"%s\"\n", buf2send);

    rv = netlink_recv(fd, buf2recv, sizeof(buf2recv)/sizeof(buf2recv[0]));
    if(rv == -1)
    {
        fprintf(stderr, "%s:%d - %d:%s\n", __FILE__, __LINE__, errno, strerror(errno));
        goto ErrP;
    }
    fprintf(stdout, "recv data from kernel succeed - \"%s\"\n", buf2recv);

    usleep(100);
}while(1);

    close(fd);
    return 1;
ErrP:
    close(fd);
    return 0;
}
