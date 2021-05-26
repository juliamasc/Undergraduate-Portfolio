/******************************************
/ PROGRAM: Assignment 1 
/ AUTHOR: Julia Masciarelli
/ COURSE: Operating Systems
/ SYNOPSIS: This program shows how to copy 
/ 	     to another using the system calls
/	     for the operating system and 
/	     handles errors. 
/******************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <errno.h>
#include <stdlib.h> 
#include <string.h>

#define BUFFER_SIZE 256 

int main(int argc, char* argv[]) {
    int inFile;
    int outFile; 
    int rcount;
    char buffer[BUFFER_SIZE];

    //Ensures that there are the correct number of command line arguments
    if(argc != 3) {
	printf("Invalid number of arguments\n usage: mycopy <sourcefile> <destinationfile>\n"); 
	exit(1); 
    } 


    //Opens file to be copied and handles errors
    inFile = open(argv[1], O_RDONLY, 0);

    if (inFile < 0) {
	printf("Unable to open %s: %s\n", argv[1], strerror(errno));
	exit(1);
    }

    //Creates the new copy file and handles errors
    outFile = creat(argv[2], S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    if (outFile < 0) { 
	printf("Unable to create %s: %s\n", argv[2], strerror(errno));
	exit(1);
    }

    //Reads orignal file and copied contents to the new file 
    //  while file is not empty and handles errors
    int bytes = 0; 
    rcount = read(inFile, buffer, 256);

    while(rcount != 0) { 
        if(rcount < 0) {
	    printf("Unable to read file %s: %s\n", argv[1], strerror(errno)); 
	    exit(1);
	}
	write(outFile, buffer, rcount);
	bytes += rcount;
	rcount = read(inFile, buffer, 256);  
    } 

    //If program is successful prints numebr of bytes copied
    printf("copied %d bytes from %s to %s\n", bytes, argv[1], argv[2]); 
    close(inFile);
    close(outFile); 

    return 0;

} 



