#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    pid_t child_pid = fork();
    if(child_pid > 0){
        printf("child pid:%i\n", child_pid);
        sleep(20);
    }
    return 0;
}