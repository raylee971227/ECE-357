
//MINICAT SOURCE CODE
//WRITTEN BY RAYMOND LEE
//ECE-357
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    int option, bufferSize, fdInput, fdOutput, inputCount, reader, writer;
    char* infile;
    char* outfile = "";

    //find flags (-b & -o both optional)
    while((option = getopt(argc, argv, "b:o:")) != -1) {
        switch(option) {
            case 'b':
                bufferSize = atoi(optarg);
                break;
            case 'o':
                outfile = optarg;
                break;
            default:
                printf("Error: flags -b and -o must be followed by an argmument.");
                return -1;
        }
    }
    //set up buffer
    if(!bufferSize) {
        bufferSize = 4096;
    }

    char *buffer = malloc((sizeof(char)) * bufferSize);
    if(buffer == NULL) {
        fprintf(stderr, "Error: Malloc error!\n%s", strerror(errno));
    }

    if(strcmp(outfile,"")) {
        //open output file
        fdOutput = open(outfile,O_WRONLY|O_CREAT|O_TRUNC,0666);
        //handle error
        if(fdOutput < 0) {
            fprintf(stderr, "Error: Failed to open output file %s for writing\n%s\n", outfile, strerror(errno));
            return -1;
        }
    }
    else {
        fdOutput = 1;
    }

    //set up number of inputs
    inputCount = argc - optind;


    //iterate through each input
    for(int i = 0; i < inputCount; i++) {
        infile = argv[optind + i];


        if(strcmp(infile, "-")) {
            //open file
            fdInput = open(infile, O_RDONLY);
        }
        else {
            fdInput = 0;
            infile = "stdin";
        }
        //check for error while opening input
        if(fdInput < 0) {
            fprintf(stderr, "Error: Failed to open input %s for reading\n%s\n", infile, strerror(errno));
            return -1;
        }


        //read file
        while((reader = read(fdInput, buffer, bufferSize)) != 0) {
            if(reader < 0) {
                fprintf(stderr, "Error: Failed to read %s\n%s\n", infile, strerror(errno));
                return -1;
            }
            else {
                //write file
                writer = write(fdOutput, buffer, reader);
                if(reader > writer) {
                    //partial write
                    fprintf(stderr, "Error: Failed to write %s to its entirety onto %s\nAttempting to recover partial write.\n", infile, outfile);
                    /* RECOVERY! */
                }
            }
        }
        //close input file
        if (fdInput != 0) {
            if(close(fdInput) == -1) {
                fprintf(stderr, "Error: Failed to close %s\n%s\n", infile, strerror(errno));
                return -1;
            }
        }
    }
    //close output file
    if(close(fdOutput) == -1) {
        fprintf(stderr, "Error: Failed to close %s\n%s\n", outfile, strerror(errno));
        return -1;
    }

    return 0;
}