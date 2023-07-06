#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int pipe_1[2];
    int pipe_2[2];
    int cpid;
    char buf;

    if (argc > 1) {
        printf("args");
        exit(1);
    }

    pipe(pipe_1);
    pipe(pipe_2);

    cpid = fork();
    if (cpid == -1) {
        printf("fork");
        exit(1);
    }

    if (cpid == 0) {   
        close(pipe_1[1]);
        close(pipe_2[0]);

        read(pipe_1[0], &buf, 1);
        printf("%d: received ping\n", getpid());
        write(pipe_2[1], &buf, 1);

        close(pipe_1[0]);
        close(pipe_2[1]);
        exit(0);

    } else {           
        close(pipe_1[0]);       
        close(pipe_2[1]);
        buf = 'f';     
        write(pipe_1[1], &buf, 1);
        read(pipe_2[0], &buf, 1);
        printf("%d: received pong\n", getpid());
        close(pipe_1[1]);
        close(pipe_2[0]);
        exit(0);
    }
}
