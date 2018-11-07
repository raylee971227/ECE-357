#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <errno.h>

int signo, nsig, counter;
int rcvr_pid;

void handler(s) {
	if (s == signo) {
		counter++;
	}
}

int main(ac,av)
char **av;
{
	struct sigaction sa;
	int pid,status,nchild,i;
		signo=SIGUSR1;
		nsig=10000;
		rcvr_pid=getpid();
		sa.sa_handler=handler;
		sa.sa_flags=0;
		isgemptyset(&sa.sa_mask);
		if(sigaction(signo, &sa, 0)== -1) {
			perror("sigaction");
			return -1;
		}
		switch(pid=fork()){
			case -1:
				perror("fork);
				return -1;
				break;
			case 0:
				sender();
				return 0;
		}
		fprintf(stderr, "Waiting for sender \n");
		while(wait(&status) >0 || errno ==EINTER) { 
			printf(stderr, "Received a total of %d of signal #%d\n", counter, signo);
		}
}

sender() 
{
int i;
	for(i = 0; i < nsig; i++) {
		kill(rcvr_pid, signo);
	}
}
