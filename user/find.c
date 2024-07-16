#include <kernel/types.h>
#include <kernel/stat.h>
#include <user/user.h>
#include <kernel/fs.h>
#include "kernel/fcntl.h"

void find(char *path, char *desname) // 当前文件夹名字和目标文件名字
{
    //printf("%s %s\n", path, desname);
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    if ((fd = open(path, O_RDONLY)) < 0)
    {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }
    switch (st.type)
    {
    case T_DEVICE:
    case T_FILE:
        break;
    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0||strcmp(de.name,".")==0||strcmp(de.name,"..")==0)//忽略父亲目录和爷爷目录
                continue;
            memmove(p, de.name, DIRSIZ);//将de.name附加到buf后
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0)
            {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            if (st.type == T_DIR)
                find(buf, desname);//递归寻找子目录
            else
            {
                if(strcmp(desname,de.name)==0)//找到了
                    printf("%s\n",buf);
            }
        }
        *(--p) = 0;
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(2, "格式不对\n");
        exit(1);
    }
    char *path = argv[1];
    char *desname = argv[2];

    find(path, desname);
    exit(0);
}