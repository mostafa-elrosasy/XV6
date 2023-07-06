#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void find(char *path, char *name){
    int fd;
    struct stat st;
    struct dirent de;
    char buf[512], *p;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }
    switch(st.type){
    // case T_FILE:
    //     printf("%s\n", fmtname(path));
    //     break;

    case T_DIR:

        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){

            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            // printf("%s?\n", buf);
            if(st.type == T_DIR){
                if(strcmp(p,".") != 0 && strcmp(p, "..") != 0)
                    find(buf, name);
            } else if(st.type == T_FILE){
                if(strcmp(p, name) == 0)
                    printf("%s\n", buf);
            }
            fmtname(buf);
        }
        break;
    }
}

int
main(int argc, char *argv[])
{

    if(argc < 3){
        printf("not enough arguments\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}
