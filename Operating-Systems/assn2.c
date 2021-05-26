/*********************************************************************************
/PROGRAM: Opeartin Systems Programming Assignment 2
/AUTHOR: Julia Masciarelli
/SYNOPSIS:This program uses both signals and pipes to send messages between
/ 	  parent and child processes
**********************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 64

static int signalFound = 0;

void the_handler(int sig_number) {
    if(sig_number == SIGUSR1) {
	signalFound = SIGUSR1;
    }
}


int main(int argc, char** argv) {

    int pipeArray[2];
    int rcount;
    int wcount;
    char buffer[BUFFER_SIZE];
    char message[] = "Hello World!";

    //Register the signal handler and handles potential errors
    if(signal(SIGUSR1, the_handler) == SIG_ERR) {
	fprintf(stderr, "Could not set signal handler %s\n", strerror(errno));
	return 1;
    }

    //Handles errors with pipe
    if(pipe(pipeArray) < 0) {
	fprintf(stderr, "Could not create pipe: %s\n", strerror(errno));
	return 1;
    }

    //Creating a new process
    pid_t pid = fork();

    //Handles errors with fork()
    if(pid < 0) {
	fprintf(stderr, "Could not create process: %s\n", strerror(errno));
	return 1;
    }

    //Child process will wait to recieve signal from signal handler
    if(pid == 0) {
	while (signalFound == 0) {
	    sleep(1);
        }
	fprintf(stdout, "Child process recieved signal from signal handler\n");
        //Child process will read the message fro pipe
	close(pipeArray[1]); //Close write since it is reading
	rcount = read(pipeArray[0], buffer, BUFFER_SIZE);
	fprintf(stdout, "Received message via pipe: \"%s\"\n", buffer);
	fprintf(stdout, "Bytes read: %d\n", rcount);
	close(pipeArray[0]);
	fprintf(stdout, "Child process terminating.\n");
    }
    else {
	//Before signal is recieved, parent will sleep for 3 seconds and then send message to child
	sleep(3);

        //Parent process will write message to pipe
        close(pipeArray[0]); //CLose read since it is writing
        wcount = write(pipeArray[1], message, strlen(message) + 1);
	fprintf(stdout, "Parent process sending message via pipe: \"%s\"\n", message);
	fprintf(stdout, "Bytes written: %d\n", wcount);
        close(pipeArray[1]);
        kill(pid, SIGUSR1);
        fprintf(stdout, "Parent process sent signal to child process (pid: %d)\n", pid);

	//child child process terminates
	wait(NULL);

	printf("Parent process (pid %d) exiting\n", getpid());
    }


    return 0;
}


