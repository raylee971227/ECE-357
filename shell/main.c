// Raymond Lee
// OS Assignment 3
// Shell Program
// Sunday October 21, 2018


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int redirect (const char * path, int newFd, int options, mode_t mode) {
    int fd;

    if ((fd = open (path, options, mode)) > 0) {
        if (dup2 (fd, newFd) < 0) {
            fprintf (stderr, "ERROR: fd=%d dup2() failed: %s\n", fd, strerror (errno));
            return 1;
        } else if (close (fd) < 0) {
            fprintf (stderr, "ERROR: '%s' close() failed: %s\n", path, strerror (errno));
            return 1;
        }
    } else {
        fprintf (stderr, "ERROR: '%s' open() failure: %s\n", path, strerror (errno));
        return 1;
    }
    return 0;
}

void prompt () {
    char cwdBuff[1024];
    if (!getcwd (cwdBuff, sizeof (cwdBuff)))
        fprintf (stderr, "ERROR: getcwd() failed: %s\n", strerror (errno));
    printf ("%s $ ", cwdBuff);
}


int runCommand (char * command, int * exit_code) {
    int pId, estat, wstat, timediff;
    struct rusage usage;
    struct timeval tBegin, tEnd;
    char * tempArgv[1024];
    char * rIn = NULL, * rOut = NULL, * rErr = NULL,
            * rOutApp = NULL, * rErrApp = NULL, * temp;

    int i = 0;
    // remove \n from end of line
    command[strlen(command) - 1] = 0;
    temp = strtok(command, " ");

    while (temp != NULL) {
        if (strstr (temp, "2>>") - temp == 0){
            rErrApp = temp + 3;
        }
        else if (strstr (temp, ">>") - temp == 0) {
            rOutApp = temp + 2;
        }
        else if (strstr (temp, "2>") - temp == 0) {
            rErr = temp + 2;
        }
        else if (strstr (temp, ">") - temp == 0) {
            rOut = temp + 1;
        }
        else if (strstr (temp, "<") - temp == 0) {
            rIn = temp + 1;
        }
        else {
            tempArgv[i++] = temp;
        }
        temp = strtok (NULL, " ");
    }
    tempArgv[i] = NULL;

    if (!strcmp (tempArgv[0], "cd")) {
        if (tempArgv[1] == NULL && chdir (getenv ("HOME")) < 0 || tempArgv[1] != NULL && chdir (tempArgv[1]) < 0) {
            fprintf (stderr, "ERROR: chdir() failed: %s\n", strerror (errno));
            *exit_code = 1;
            return 1;
        }
        return 0;
    }

    if (gettimeofday(&tBegin, NULL) < 0) {
        fprintf (stderr, "ERROR: gettimeofday() failed: %s\n", strerror (errno));
        *exit_code = 1;
        return 1;
    }

    switch (pId = fork ()) {
        case -1:
            fprintf (stderr, "ERROR: '%s' fork() failed: %s\n", tempArgv[0], strerror (errno));
            *exit_code = 1;
            return 1;
        case 0:
            // handle redirection
            if (rErrApp != NULL) {
                if (redirect (rErrApp, 2, O_RDWR | O_APPEND | O_CREAT, 0666)) {
                    exit (1);
                }
            } else if (rErr != NULL)
                if (redirect (rErr, 2, O_RDWR | O_TRUNC | O_CREAT, 0666)) {
                exit (1);
            }
            if (rOutApp != NULL) {
                if (redirect (rOutApp, 1, O_RDWR | O_APPEND | O_CREAT, 0666)) {
                    exit (1);
                }
            } else if (rOut != NULL)
                if (redirect (rOut, 1, O_RDWR | O_TRUNC | O_CREAT, 0666)) {
                exit (1);
            }
            if (rIn != NULL && redirect (rIn, 0, O_RDONLY, 0666)) {
                exit (1);
            }

            if (execvp (tempArgv[0], tempArgv) < 0) {
                fprintf (stderr, "ERROR: '%s' execvp() failure: %s\n", tempArgv[0], strerror (errno));
                exit (1);
            }
        default:
            if (wait4 (pId, &wstat, 0, &usage) > 0) {
                if ((estat = WEXITSTATUS (wstat)) != 0) {
                    *exit_code = 1;
                }
                if (gettimeofday(&tEnd, NULL) < 0) {
                    fprintf (stderr, "ERROR: gettimeofday() failure: %s\n", strerror (errno));
                    *exit_code = 1;
                }
                timediff = tEnd.tv_sec * 1000000 + tEnd.tv_usec - (tBegin.tv_sec * 1000000 + tBegin.tv_usec);

                fprintf (stderr, "\n[COMMAND INFO]\n   Return Code:\t%d\n", estat);
                fprintf (stderr, "   Real Time:\t%d.%04ds\n", timediff / 1000000, timediff % 1000000);
                fprintf (stderr, "   System Time:\t%d.%04ds\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
                fprintf (stderr, "   User Time:\t%d.%04ds\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
            }
            else {
                fprintf (stderr, "ERROR: pid=%d wait4() failure: %s\n", pId, strerror (errno));
            }
    }

    return 0;
}

int main (int argc, char ** argv) {
    int exit_code, bytesRead;
    size_t commandSize = 1024;
    char * command = NULL;
    FILE * inFile;

    exit_code = 0;

    if (argc > 1 && (inFile = fopen (argv[1], "r")) == NULL) {
        fprintf (stderr, "ERROR: '%s' fopen() failure: %s\n", argv[1], strerror (errno));
        return 1;
    }
    else if (argc == 1) {
        inFile = stdin;
    }

    if (inFile == stdin) {
        prompt();
    }

    while ((bytesRead = getline (&command, &commandSize, inFile)) > 0) {
        if (bytesRead <= 1 || command[0] == '#' || command[bytesRead - 1] != '\n') {
            errno = 0;
            if (inFile == stdin) {
                prompt ();
            }
            continue;
        }
        else {
            runCommand (command, &exit_code);
        }

        if (inFile == stdin) {
            prompt ();
        }
        errno = 0;
    }

    if (errno != 0) {
        fprintf (stderr, "\nERROR: getline() failed: %s\n", strerror (errno));
        return 1;
    }
    else {
        printf ("\nEOF reached. Exitting shell.!\n");
    }

    return exit_code;
}