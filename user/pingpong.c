#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void) {
    int fds[2]; // 文件描述符数组，用于管道
    char c;
    int pid;

    // 创建管道
    if (pipe(fds) < 0) {
        printf("pipe failed\n");
        exit(1);
    }

    // fork 子进程
    pid = fork();

    if (pid < 0) {
        printf("fork failed\n");
        exit(1);
    }

    if (pid > 0) {
        // 父进程
        // 向子进程发送一个字节
        c = 'p'; // 这里使用 'p' 表示 ping
        if (write(fds[1], &c, 1) != 1) {
            printf("write failed\n");
            exit(1);
        }
        // 读取子进程发送回的字节
        if (read(fds[0], &c, 1) != 1) {
            printf("read failed\n");
            exit(1);
        }
        printf("%d: received pong\n", getpid());
    } else {
        // 子进程
        // 读取父进程发送的字节
        if (read(fds[0], &c, 1) != 1) {
            printf("read failed\n");
            exit(1);
        }
        printf("%d: received ping\n", getpid());
        // 向父进程发送一个字节
        if (write(fds[1], &c, 1) != 1) {
            printf("write failed\n");
            exit(1);
        }
        exit(0);
    }

    exit(0);
}