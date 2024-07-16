#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if (argc < 2) {  // 检查是否至少有一个参数
        printf("Usage: sleep number\n");
        exit(1);
    }

    int sleepTime = atoi(argv[1]);  // 将第一个参数转换为整数

    // 调用 sleep 系统调用
    sleep(sleepTime);

    exit(0);  // 正常退出
}