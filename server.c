#include "kserver.h"



int 
set_nonblock(int fd)
{
    int flags, rc;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        fprintf(stdout, "fcntl(F_GETFL) failed: %s\n", strerror(errno));
        return -1;
    }

    flags |= O_NONBLOCK;
    rc = fcntl(fd, F_SETFL, flags);
    if (rc < 0) {
        fprintf(stdout, "fcntl(F_SETFL) failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}


int 
set_reuseaddr(int fd)
{
    int rc, optv;

    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optv, sizeof(optv));

    if (rc < 0) {
        fprintf(stdout, "SO_REUSEADDR failed: %s\n", strerror(errno));
    }

    return rc;
}


int 
init_server(int port)
{
    int rc = 0;
    struct sockaddr_in saddr;

    memset(&saddr, 0, sizeof(saddr)); 
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(port);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (fd < 0) {
        fprintf(stdout, "socket() failed: %s\n", strerror(errno));
        exit(1);
    }

    rc = set_reuseaddr(fd);//端口重用，不知是否有效

    if (rc < 0) {
        return rc;
    }

    rc = bind(fd, (struct sockaddr *) &saddr, sizeof(saddr));

    if (rc < 0) {
        fprintf(stdout, "bind() failed: %s\n", strerror(errno));
        return rc;
    }
 
    rc = listen(fd, LISTEN_BACKLOG);

    if (rc < 0) {
        fprintf(stdout, "listen() failed: %s\n", strerror(errno));
        return rc;
    }
    
    rc = so_nonblock(fd);

    if (rc < 0) {
        return rc;
    }

	struct rlimit rt;
	rt.rlim_max = rt.rlim_cur = EPOLL_SIZE;
	if(setrlimit(RLIMIT_NOFILE, &rt) == -1) {
		fprintf("setrlimit - %m");
	}
 
    return fd;
}

void init_daemon(void) {
	int pid, i;
	if(pid = fork()) {
		exit(0);//是父进程，结束
	}else if(pid < 0) {
		exit(1);//fork失败，退出
	}
	setsid();
	if(pid = fork()) {
		exit(0);//是第一子进程，结束
	}else if(pid < 0) {
		exit(1);//fork失败，退出
	}
	for(i=0;i<NOFILE;++i) {
		close(i);
	}

	chdir("/tmp");
	umask(0);
	return ;
}

int main(int argc, char **argv) {
	char ch;
	int d = 0;
	while((ch = getopt(argc, argv, "e:d?")) != EOF) {
		switch(ch) {
			case 'e':
			break;
			case 'd':
				printf("run as daemon.");
				d = 1;
			break;
			case '?':
				printf("Useage: -d");
				exit(1);
			break;
			default:
				printf("Not support option :%c\n", ch);
				exit(2);
			break;
		}
	}
	if(d == 1)init_daemon(); //守护进程
}
