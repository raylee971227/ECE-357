/*
	ECE357 Prof.Hakner
 	Catgrepmore
	Author:Yejun Iris Huang
    To run the program: gcc -o catgrepmore.exe catgrepmore.c
                    ./catgrepmore.exe pattern infile1 [infile2]
*/
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_INPUT 1024



unsigned long byteCounter = 0;
unsigned int fileCounter = 0;

/*
    This function handles the pipes in grep child
    return 0 for success
    return -1 for failure
 */
int grep_handler(int g_pipe[], int m_pipe[], char* pattern);

/* Write to stderr the total number of files and bytes processed */
void exitNotes();
/* Clean up function which close the pipes and file */
void cleanUp(int m_pipe[], int infile);

/* Set up the structure for ignoring signal*/
const struct sigaction ignore_sig =
        {
                .sa_handler = SIG_IGN,
                .sa_mask = 0,
                .sa_flags = 0
        };

/* Set up the structure for interrupting signal */
const struct sigaction inter_sig =
        {
                .sa_handler = exitNotes,
                .sa_mask = 0,
                .sa_flags = 0
        };


int main(int argc, char* argv[])
{
    int g_pipe[2];
    int m_pipe[2];
    static char usage[] = "./catgrepmore patterm infile1 [...infile2...]\n";
    char *pattern;
    if (!( pattern = malloc(MAX_INPUT)))
        fprintf(stderr, "[Error] Pattern malloc failed: %s\n", strerror(errno));
    if (sigaction(SIGINT, &ignore_sig, 0) == -1)
        fprintf(stderr, "[ERROR] Unable to create handler to ignore signal: %s \n", strerror(errno));
    if (argc < 3 || !argv[1] || strlen(argv[1]) == 0)
    {
        fprintf(stderr, "[Error] Invaild argument. Usage: %s \n", usage);
        return -1;
    }
    pattern = argv[1];
    int infile;
    for (int i = 2; i < argc; i++) //iterate though the argument
    {
        printf("In file: %s\n\n", argv[i]);
        if (access(argv[i], R_OK) != 0)
        {
            fprintf(stderr, "ERROR: Unable to access the file %s: %s \n", argv[i], strerror(errno));
            return -1;
        }
        if (pipe(g_pipe) < 0 || pipe(m_pipe) < 0)
        {
            perror("[Error] Pipe failed:");
            return -1;
        }
        pid_t pid1;
        int flag;
        switch( pid1 = fork())
        {
            case -1: //fork failed
                perror("[Error] Unable to fork:");
                return -1;
            case 0: //in grep
                flag = grep_handler(g_pipe, m_pipe, pattern);
                if (flag < 0)
                    return -1;
                execlp("grep", "grep", pattern, NULL);
                perror("[Error] Grep Failed: ");
                return -1;
            default: //in parent
                if(close(g_pipe[0]) < 0 || close(m_pipe[1]) < 0)
                {
                    perror("[Error] Unable to close unused pipes: ");
                    return -1;
                }
                break;
        }
        char *buffer = malloc(MAX_INPUT);
        int more = fork();
        if(more == 0) {
            //dup pipe2 read to stdin
            if(close(g_pipe[1]) < 0) {
                perror("Pipes are leaking everywhere");
                exit(-1);
            }
            if(dup2(m_pipe[0], STDIN_FILENO) < 0) {
                perror("Could not redirect stdin for more");
                exit(-1);
            }
            execlp("more", "more", NULL);
            fprintf(stderr, "You must halt. And catch fire.\n");
            exit(-1);
        } else if(more > 0) {
            if(close(m_pipe[0]) < 0) {
                perror("Cannot close pipe");
                exit(-1);
            }
            if((infile = open(argv[i], O_RDONLY)) < 0) {
                perror("Cannot open input file");
                fprintf(stderr, "File: %s\n", argv[i]);
                exit(-1);
            }
            fileCounter++;
            int bytes, grepStat, moreStat;
            while(1) {
                byteCounter += bytes = read(infile, buffer, MAX_INPUT);
                if(bytes < 0) {
                    perror("Error reading input");
                    exit(-1);
                } else {
                    int wbytes = write(g_pipe[1], buffer, bytes);
                    if(wbytes < 0) {
                        perror("Error writing to output");
                        exit(-1);
                    }
                    if(bytes < MAX_INPUT) { //reached EOF
                        if(close(g_pipe[1]) < 0) {
                            perror("Cannot close write pipe");
                        }
                        break;
                    }
                }
            }
            waitpid(pid1, &grepStat, 0);
            waitpid(more, &moreStat, 0);
            if(close(infile) < 0) {
                perror("Could not close input file");
                fprintf(stderr, "File: %s\n", argv[i]);
                exit(-1);
            }
        } else {
            perror("Could not fork process for more");
            exit(-1);
        }
    }
    printf("Program Finished: ");
    exitNotes();
    return 0;
}

int grep_handler(int g_pipe[], int m_pipe[], char* pattern)
{
    if(close(g_pipe[1]) < 0 || close(m_pipe[0]) < 0) {
        perror("[Error] Unable to close file descriptors for pipes:");
        return -1;
    }
    //dup pipe1 read to stdin and pipe2 write to stdout
    if(dup2(g_pipe[0], STDIN_FILENO) < 0 || dup2(m_pipe[1], STDOUT_FILENO) < 0) {
        perror("[Error] The pipes are all mixed up");
        return -1;
    }
    return 0;
}

void exitNotes()
{
    fprintf(stderr, "File processed: %d \t Bytes processed: %lu \n", fileCounter, byteCounter);
    exit(0);
}


