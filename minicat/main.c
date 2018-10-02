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
    int option, bufferSize, fdInput, fdOutput, inputCount, noInput, reader, writer;
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
                printf("outfile placed.\n");
                break;
            default:
                fprintf(stderr, "Error: flags -b and -o must be followed by an argmument.\n%s\n", strerror(errno));
                return -1;
        }
    }
    //set up buffer
    if(!bufferSize) {
        bufferSize = 4096;
    }
    printf("buffer size: %d\n", bufferSize);

    char *buffer = malloc((sizeof(char)) * bufferSize);

    if(strcmp(outfile,"")) {
        //open output file
        printf("opening output file %s\n", outfile);
        fdOutput = open(outfile,O_WRONLY|O_CREAT|O_TRUNC,0666);
        //handle error
        if(fdOutput < 0) {
            fprintf(stderr, "Error: Failed to open output file %s for writing\n%s\n", outfile, strerror(errno));
            return -1;
        }
        else {
            printf("output %s opened\n", outfile);
        }
    }
    else {
        printf("no output file selected\n");
        fdOutput = 1;
    }

    //set up number of inputs
    inputCount = argc - optind;
    if(!inputCount) {
        noInput = 1;
    }
    printf("inputs: %d\n", inputCount);

    //iterate through each input
    for(int i = 0; i < inputCount; i++) {
        infile = argv[optind + i];
        printf("current input file: %s\n", infile);

        if(strcmp(infile, "-")) {
            //open file
            fdInput = open(infile, O_RDONLY, 0666);
        }
        else {
            printf("'-' detected!\n");
            fdInput = 0;
            infile = "stdin";
        }
        //check for error while opening input
        if(fdInput < 0) {
            fprintf(stderr, "Error: Failed to open input %s for reading\n%s\n", infile, strerror(errno));
            return -1;
        }
        else {
            printf("File %s successfully opened\n", infile);
        }

        //read file
        printf("attempting to read fdInput # %i: \n", fdInput);
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
                    fprintf(stderr, "Error: Failed to write %s to its entirety onto %s\n", infile, outfile);
                    return -1;
                }
            }
        }
        //close input file
        if(close(fdInput) == -1) {
            fprintf(stderr, "Error: Failed to close %s\n%s\n", infile, strerror(errno));
            return -1;
        }
        else {
            printf("input %s succesfully closed!\n", infile);
        }
    }
    //close output file
    if(close(fdOutput) == -1) {
        fprintf(stderr, "Error: Failed to close %s\n%s\n", outfile, strerror(errno));
        return -1;
    }
    else {
        printf("output %s succesfully closed!\n", outfile);
    }
    return 0;
}