/******************************************************************************************
/* PROGRAM:     myshell.c
/* AUTHOR:Julia Masciarelli
/* DESCRIPTION: Acts as a very simple command line interpreter.  It reads commands from
/*              standard input entered from the terminal and executes them. The shell does
/*              not include any provisions for control structures, pipes, redirection,
/*              background processes, environmental variables, or other advanced properties
/*              of a modern shell. All commands are implemented internally and do not rely
/*              on external system programs.
/*              **VERSION 2: Extra Credit
/*		Added cp,mv, and head command
/*		cp copies file1 to file2
/*		mv renames file1 to file2
/*		head reads for 5 lines of a file (with standard line size of 80 characters)
/*******************************************************************************************/
#include <fcntl.h>
#include <pwd.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH_LENGTH           256
#define MAX_BUFFER_LENGTH         256
#define MAX_FILENAME_LENGTH       256

char buffer[MAX_BUFFER_LENGTH] = {0};
char filename[MAX_FILENAME_LENGTH] = {0};
char filename2[MAX_FILENAME_LENGTH] = {0};
int n;
unsigned int result;

// Implements various UNIX commands using POSIX system calls
int do_cat(const char* filename);
int do_cd(char* dirname);
int do_cp(const char* originalFile, const char* newFile);
int do_ls(const char* dirname);
int do_mkdir(const char* dirname);
int do_pwd(void);
int head(const char* filename);
int do_rm(const char* filename);
int do_mv(const char* oldName, const char* newName);
int do_rmdir(const char* dirname);
int do_stat(char* filename);
int execute_command(char* buffer);
// Remove extraneous whitespace at the end of a command to avoid parsing problems
void strip_trailing_whitespace(char* string)
{
  int i = strnlen(string, MAX_BUFFER_LENGTH) - 1;

  while(isspace(string[i]))
    string[i--] = 0;
}

// Display a command prompt including the current working directory
void display_prompt(void)
{
  char current_dir[MAX_PATH_LENGTH];

  if (getcwd(current_dir, sizeof(current_dir)) != NULL)
    fprintf(stdout, "%s>", current_dir);
}

int main(int argc, char** argv)
{
  while (1)
    {
      display_prompt();

      // Read a line representing a command to execute from standard input into a character array
      if (fgets(buffer, MAX_BUFFER_LENGTH, stdin) != 0)
	{
	  strip_trailing_whitespace(buffer);            // Clean up sloppy user input
	  memset(filename, 0, MAX_FILENAME_LENGTH);     //Reset filename buffer after each command execution

	  // As in most shells, "cd" and "exit" are special cases that needed to be handled separately
	  if ((sscanf(buffer, "cd %s", filename) == 1) || (!strncmp(buffer, "cd", MAX_BUFFER_LENGTH)))
	    {
	      result = do_cd(filename);
	      continue;
	    }
	  else if (!strncmp(buffer, "exit", MAX_BUFFER_LENGTH))
	    {
	      exit(0);
	    }
	  else
	    {
	      execute_command(buffer);
	    }
	}
    }
  return 0;
}

// changes the current working directory
int do_cd(char* dirname)
{
  struct passwd *p = getpwuid(getuid());
  int result;

  if (strnlen(dirname, MAX_PATH_LENGTH) == 0)
      strncpy(dirname, p->pw_dir, MAX_PATH_LENGTH);

  result = chdir(dirname);
  if (result < 0)
    fprintf(stderr, "cd: %s\n", strerror(errno));

  return result;
}

// lists the contents of a directory
int do_ls(const char* dirname)
{
     DIR* directoryStream;
     struct dirent* d;

    chdir(dirname);
    directoryStream = opendir(dirname);

    if(directoryStream == NULL) {
        fprintf(stderr, "Could not open directory %s: %s\n", dirname, strerror(errno));
        exit(1);
    }

    d = readdir(directoryStream);

    while(d != NULL) {
	printf("%s\n", d->d_name);
	d = readdir(directoryStream);
    }

    chdir("-");
    closedir(directoryStream);
}

// outputs the contents of a single ordinary file
int do_cat(const char* filename)
{
    int inFile;
    int rcount;

    inFile = open(filename, O_RDONLY, 0);

    if (inFile < 0) {
	printf("Unable to open %s: %s\n", filename, strerror(errno));
	return -1;
    }

    rcount = read(inFile, buffer, MAX_BUFFER_LENGTH);

    while(rcount != 0) {
	if(rcount < 0) {
	    printf("Unable to read file %s: %s\n", filename, strerror(errno));
            return -1;
        }
        write(1, buffer, rcount);
        rcount = read(inFile, buffer, MAX_BUFFER_LENGTH);
    }
    close(inFile);
}

// outputs the first 5 lines of a file
int do_head(const char* filename)
{
    int inFile;
    int rcount;

    inFile = open(filename, O_RDONLY, 0);

    if (inFile < 0) {
	printf("Unable to open %s: %s\n", filename, strerror(errno));
	return -1;
    }

    rcount = read(inFile, buffer, 80);   //80 for standard line size
    int count = 0;

    while(count < 5 && rcount != 0) {
	if(rcount < 0) {
	    printf("Unable to read file %s: %s\n", filename, strerror(errno));
	    return -1;
    	}
	write(1, buffer, rcount);
	count++;
	rcount = read(inFile, buffer, 80);
    }
    close(inFile);
}

// copies the first file to the second file name
int do_cp(const char* originalFile, const char* newFile)
{
    int inFile;
    int outFile;
    int rcount;

    inFile = open(originalFile, O_RDONLY, 0);

    if (inFile < 0) {
	printf("Unable to open %s: %s\n", originalFile, strerror(errno));
	return 1;
    }

    outFile = creat(newFile, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (outFile < 0) {
	printf("Unable to create %s: %s\n", newFile, strerror(errno));
	return 1;
    }

    rcount = read(inFile, buffer, MAX_BUFFER_LENGTH);

    while(rcount != 0) {
	if(rcount < 0) {
	    printf("Unable to read file %s: %s\n", originalFile, strerror(errno));
	    return 1;
        }
	write(outFile, buffer, rcount);
	rcount = read(inFile, buffer, MAX_BUFFER_LENGTH);
    }

    printf("Copied %s to %s.\n", originalFile, newFile);
    close(inFile);
    close(outFile);
}

// creates a new directory
int do_mkdir(const char* dirname)
{
    int make;
    make = mkdir(dirname, 0755);

    if(make < 0) {
	printf("Unable to make directory %s: %s\n", dirname, strerror(errno));
	return -1;
    }
}

// removes a directory as long as it is empty
int do_rmdir(const char* dirname)
{
    int remove;
    remove = rmdir(dirname);

    if(remove < 0) {
	printf("Unable to remove directory %s: %s\n", dirname, strerror(errno));
	return -1;
    }
}

// renames a file to a new name
int do_mv(const char* oldName, const char* newName)
{
    int ret;
    ret = rename(oldName, newName);

    if(rename < 0) {
 	printf("Unable to rename file %s: %s\n", oldName, strerror(errno));
	return 1;
    }

    printf("Renamed file %s to %s\n", oldName, newName);
}

// outputs the current working directory
int do_pwd(void)
{
    char current_dir[MAX_PATH_LENGTH];
    if(getcwd(current_dir, sizeof(current_dir)) != NULL) {
	fprintf(stdout, "\n%s>\n", current_dir);
    }
}

// removes (unlinks) a file
int do_rm(const char* filename)
{
    int delete;
    delete = unlink(filename);

    if(delete < 0) {
        printf("Unable to remove file %s: %s\n", filename, strerror(errno));
	return -1;
    }
}

// outputs information about a file
int do_stat(char* filename)
{
    struct stat fileInfo;

    if(stat(filename, &fileInfo) < 0) {
	fprintf(stderr, "Could not state file %s: %s\n", filename, strerror(errno));
	return -1;
    }

    printf("File name: %s\n", filename);
    printf("File size: %lu\n", fileInfo.st_size);
    printf("Number of links: %lu\n",fileInfo.st_nlink);
    printf("Inode: %lu\n", fileInfo.st_ino);
    printf("Number of blocks: %lu\n", fileInfo.st_blocks);
}

int execute_command(char* buffer)
 {
   if (sscanf(buffer, "cat %s", filename) == 1)
     {
       result = do_cat(filename);
       return result;
     }
   if (sscanf(buffer, "stat %s", filename) == 1)
     {
       result = do_stat(filename);
       return result;
     }
   if (sscanf(buffer, "mkdir %s", filename) == 1)
     {
       result = do_mkdir(filename);
       return result;
     }
   if (sscanf(buffer, "rmdir %s", filename) == 1)
     {
       result = do_rmdir(filename);
       return result;
     }
   if (sscanf(buffer, "rm %s", filename) == 1)
     {
       result = do_rm(filename);
       return result;
     }
   if (sscanf(buffer, "cp %s %s", filename, filename2) == 2)
     {
       result = do_cp(filename, filename2);
       return result;
     }
   if (sscanf(buffer, "mv %s %s", filename, filename2) == 2)
     {
       result = do_mv(filename, filename2);
       return result;
     }
   if (sscanf(buffer, "head %s", filename) == 1)
     {
      result = do_head(filename);
      return result;
     }

   else if ((sscanf(buffer, "ls %s", filename) == 1) || (!strncmp(buffer, "ls", MAX_BUFFER_LENGTH)))
     {
       if (strnlen(filename, MAX_BUFFER_LENGTH) == 0)
	 sprintf(filename, ".");
       result = do_ls(filename);
       return result;
     }
   else if (!strncmp(buffer, "pwd", MAX_BUFFER_LENGTH))
     {
       result = do_pwd();
       return result;
     }
   else // Invalid command
     {
       if (strnlen(buffer, MAX_BUFFER_LENGTH) != 0)
	 fprintf(stderr, "myshell: %s: No such file or directory\n", buffer);
       return -1;
     }
 }
