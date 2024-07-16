#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXN 35

void prime(int fd0, int sz)
{
    int pre[sz];
    int p[sz];
    read(fd0, pre, sz * sizeof(int));
    close(fd0);
    printf("prime %d\n", pre[0]);
    int nsz = 0;
    for (int i = 1; i < sz; i++)
    {
        if (pre[i] % pre[0] > 0)
            p[nsz++] = pre[i];
    }    
    if (nsz == 0) // 没有数据了就停止
        return;
    int fd[2];//创建新的管道
    if (pipe(fd) < 0)
    {
        printf("pipe failed\n");
        exit(1);
    }
    int pid = fork(); // 创建子进程
    if (pid > 0)
    {
        write(fd[1], p, (nsz) * sizeof(int));
        close(fd[1]);
        wait(0);
    }
    else
    {
        prime(fd[0], nsz);
        exit(0);
    }
}
int main(int argc, int *argv[])
{
    int p[MAXN - 1];
    for (int i = 2; i <= MAXN; i++) // 初始化传入p的数据
    {
        p[i - 2] = i;
    }
    int fd[2];
    if (pipe(fd) < 0) // 创建管道
    {
        printf("pipe failed");
        exit(1);
    }
    write(fd[1], p, (MAXN - 1) * sizeof(int));
    prime(fd[0], MAXN - 1);
    exit(0);
}