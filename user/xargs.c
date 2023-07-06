#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
mysleep(int n)
{
    sleep(n);
}

int
main(int argc, char *argv[])
{
    if(argc <= 1){
        exit(1);
    }

    char c;
    char line[512] = {0};
    int len=0;
    while(read(0, &c, 1) > 0){
        line[len] = c;
        len++;
    }
    
    char args[10][100] = {0};
    int args_index = 0;
    for(;args_index < argc - 1; args_index++)
        for(int i =0; i<strlen(argv[args_index+1]); i++)
            args[args_index][i] = argv[args_index+1][i];
    
    int token_index = 0;
    for(int i = 0; i < len; i++){
        if(line[i] == '\n'){
            token_index = 0;
            args_index++;
            continue;
        }
        args[args_index][token_index] = line[i];
        token_index++;
    }

    char * call_args[10] = {0};
    for(int i=0; i<args_index; i++)
        call_args[i] = args[i];

    int rc = fork();
    if(rc == 0){
        exec(call_args[0], call_args);
        printf("execution failed\n");
        exit(1);
    }else{
        wait(0);
    }

    exit(0);
}
