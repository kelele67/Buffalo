#include <stdint.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "bf_utils.h"
#include "bf_timer.h"
#include "bf_http.h"
#include "bf_epoll.h"
#include "bf_threadpool.h"

#define CONF "buffalo.conf"
#define PROGRAM_VERSION "0.0.1"

extern struct epoll_event *events;

static const struct option long_options[] = {
        {"help", no_argument, NULL, '?'},
        {"version", no_argument, NULL, 'V'},
        {"conf", required_argument, NULL, 'c'},
        {NULL, 0, NULL, 0}
};

static void usage() {
    fprintf(stderr,
            "buffalo [option]... \n"
            "  -c|--conf <config file>  Specify config file. Default ./buffalo.conf.\n"
            "  -?|-h|--help             This information.\n"
            "  -V|--version             Display program version.\n"
    );

}

int main(int argc, char* argv[]) {
    int ret;
    int opt = 0;
    int options_index = 0;
    char *conf_file = CONF;

    /*
    * parse argv
    * more detail visit: http://www.gnu.org/software/libc/manual/html_node/Getopt.html
    */

    if (argc == 1) {
        usage();
        return 0;
    }

    while ((opt = getopt_long(argc, argv, "Vc:?h", long_options, &options_index)) != EOF) {
        switch (opt) {
            case  0 :
                break;
            case 'c':
                conf_file = optarg;
                break;
            case 'V':
                printf(PROGRAM_VERSION"\n");
                return 0;
            case ':':
            case 'h':
            case '?':
                usage();
                return 0;
        }
    }

    bf_debug("conffile = %s", conf_file);

    if (optind < argc) {
        log_err("non-option ARGV-elements:");
        while (optind < argc) {
            log_err("%s ", argv[optind++]);
        }
        return 0;
    }

    // 读confile文件
    char conf_buf[BUFLEN];
    bf_conf_t cf;
    ret = read_conf(conf_file, &cf, conf_buf, BUFLEN);
    check(ret == BF_CONF_OK, "read conf err");

    /*
    *   install signal handle for SIGPIPE
    *   when a fd is closed by remote, writing to this fd will cause system send
    *   SIGPIPE to this process, which exit the program
    */
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) {
        log_err("install signal handler for SIGPIPE failed");
        return 0;
    }

    //初始化监听socket
    int listenfd;
    struct sockaddr_in clientaddr;
    socklen_t inlen = 1;
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));

    listenfd = open_listenfd(cf.port);
    ret = make_socket_non_blocking(listenfd);
    check(ret == 0, "make_socket_non_blocking");

    //创建epoll并添加描述符到文件集
    int epfd = bf_epoll_create(0);
    struct epoll_event event;

    bf_request_t *request = (bf_request_t *)malloc(sizeof(bf_request_t));
    bf_init_request_t(request, listenfd, epfd, &cf);

    event.data.ptr = (void *)request;
    event.events = EPOLLIN | EPOLLET;
    bf_epoll_add(epfd, listenfd, &event);

    bf_timer_init();

    log_info("buffalo started.");
    int n;
    int i, fd;
    int time;

    // 后台程序
    while (1) {
        time = bf_find_timer();
        bf_debug("wait time = %d", time);
        n = bf_epoll_wait(epfd, events, MAXEVENTS, time);
        bf_handle_expire_timers();

        for (i = 0; i < n; ++i) {
            bf_request_t *r = (bf_request_t *)events[i].data.ptr;
            fd = r->fd;

            if (listenfd == fd) {
                //如果收到一个或者多个连接
                int infd;
                while (1) {
                    infd = accept(listenfd, (struct sockaddr *) &clientaddr, &inlen);
                    if (infd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            //连接已被处理完毕
                            break;
                        } else {
                            log_err("accept");
                            break;
                        }
                    }

                    ret = make_socket_non_blocking(infd);
                    check(ret == 0, "make_socket_non_blocking");
                    log_info("new connection fd %d", infd);

                    bf_request_t *request = (bf_request_t *)malloc(sizeof(bf_request_t));
                    if (request == NULL) {
                        log_err("malloc(sizeof(bf_request_t))");
                        break;
                    }

                    bf_init_request_t(request, infd, epfd, &cf);
                    event.data.ptr = (void *)request;
                    event.events = EPOLLIN | EPOLLET |EPOLLONESHOT;

                    bf_epoll_add(epfd, infd, &event);
                    bf_add_timer(request, TIMEOUT_DEFAULT, bf_close_conn);
                }
            } else {
                if ((events[i].events & EPOLLERR) ||
                        (events[i].events & EPOLLHUP) ||
                        (!(events[i].events & EPOLLIN))) {
                    log_err("epoll error fd : %d", r->fd);
                    close(fd);
                    continue;
                }

                log_info("new data from fd %d", fd);

                do_request(events[i].data.ptr);
            }
        }
    }
    return 0;

}