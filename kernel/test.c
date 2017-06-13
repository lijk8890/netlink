#include <linux/mutex.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

#define NETLINK_TEST 31

static struct sock *nlsk = NULL;

DEFINE_MUTEX(lock);
EXPORT_SYMBOL(lock);

static void netlink_rcv(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = nlmsg_hdr(skb);
    struct nlmsghdr *_nlh = NULL;
    struct sk_buff *_skb = NULL;

    int ret = 0;
    char msg[] = "hello, user!";
    int size = sizeof(msg)/sizeof(msg[0]);

//  printk(KERN_INFO "recv data from user succeed - \"%s\"\n", (char*)nlmsg_data(nlh));

    _skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (_skb == NULL)
    {
        printk(KERN_ERR "%s:%d - netlink failed\n", __FILE__, __LINE__);
        return;
    }
    NETLINK_CB(_skb).dst_group = 0;

    _nlh = nlmsg_put(_skb, nlh->nlmsg_pid, 0, 0, size, 0);
    if(_nlh == NULL)
    {
        printk(KERN_ERR "%s:%d - netlink failed\n", __FILE__, __LINE__);
        nlmsg_free(_skb);
        return;
    }
    memcpy(nlmsg_data(_nlh), msg, size);

    nlmsg_end(_skb, _nlh);
    ret = nlmsg_unicast(nlsk, _skb, nlh->nlmsg_pid);
    if(ret != 0)
    {
        printk(KERN_ERR "%s:%d - netlink failed\n", __FILE__, __LINE__);
        nlmsg_cancel(_skb, _nlh);
        nlmsg_free(_skb);
        return;
    }

//  printk(KERN_INFO "send data to user succeed - \"%s\"\n", msg);
    return;
}

static int __init netlink_init(void)
{
    nlsk = netlink_kernel_create(&init_net, NETLINK_TEST, 0, netlink_rcv, NULL, THIS_MODULE);
    if(nlsk == NULL)
    {
        printk(KERN_ERR "%s:%d - netlink failed\n", __FILE__, __LINE__);
        return -ENOMEM;
    }

    printk(KERN_INFO "netlink init\n");
    return 0;
}

static void __exit netlink_exit(void)
{
    printk(KERN_INFO "netlink exit\n");
    netlink_kernel_release(nlsk);
}

module_init(netlink_init);
module_exit(netlink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lijk <lijk@infosec.com.cn>");
MODULE_DESCRIPTION("user and kernel communication with netlink");
MODULE_VERSION("0.0.1");
