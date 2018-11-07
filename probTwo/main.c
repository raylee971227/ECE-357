#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

int signo, nsig, counter;
int rcvr_pid;

void handler(int s) {
    if (s == signo) {
        counter++;
    }
}

char **av;

int main() {
    struct sigaction sa;
    int pid, status, nchild, i;

    signo = SIGUSR1;
    nsig = 10000;
    rcvr_pid = getpid();
    sa.sa_handler = handler;

    /* 2a-1 */
    sa.sa_flags = 0;

    /* 2a-2 */
    sigemptyset(&sa.sa_mask);
    if(sigaction(signo, &sa, 0) == -1) {
        perror("sigaction");
        return -1;
    }

    switch(pid = fork()) {
        case -1:
            perror("fork");
            return -1;
        case 0:
            sender();
            return 0;
    }

    fprintf(stderr, "Waiting for sender\n");
    /* WHY do i have this thing with EINTR below?? */
//    while (wait(&status) > 0  /* || errno == EINTR */){
//        ;
//    }
//    printf("stderr, Received a total of %d of signal #%d\n", counter, signo);

    while (wait(&status) > 0 || errno == EINTR){
        ;
    }
    /**/
    printf("stderr, Received a total of %d of signal #%d\n", counter, signo);.

}

sender() {
    int i;
    for(i=0; i < nsig; i++) {
        kill(rcvr_pid,signo);
    }
}