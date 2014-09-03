/* Yugraj Singh
 * CS111 asg1: Shell
 * 
 *
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define _POSIX_SOURCE

/*MARCOS used specifiy the command '>, <, &' to run*/
#define DEFAULT          1
#define REDIRECT_TO_FILE 2
#define REDIRECT_OF_INPT 3
#define RUN_IN_BACKGRD   4

extern char **get_line(void);

/* execute_cmd() function creates a child process and uses the system
   call to execute the shell command. The function takes three inputs
   for the shell mode, the cammand to be run and filename for file 
   redirection
*/
void execute_cmd(int cur_mode, char **cmd, char **file_name, char **file2_name,
                 int file_flag, int input_flag, int backgrd){
   /* Creaite a child process and passes the command to excevp().*/
    FILE *file; 
    int w_pid;
    int status;
    int pid = fork();
    if (pid >= 0) {
      if (pid == 0) {
        /*in child*/
        if (cur_mode == REDIRECT_TO_FILE ) {
        /*Change the file descriptor for the file to be stdout.
	  Anything printed to standard out will print to the file 
          for this child process
	*/
	  if (file_flag == 1 && input_flag == 1) {   /*both flags indicate we will write to and read
	                                               to the file names passed into the function*/
	    freopen(*file_name,"w",stdout);
	    freopen(*file2_name,"r",stdin); 
	  } else {
	    file = freopen(*file_name,"w",stdout); /*only reading from one file*/
	  }
	  /*excute command*/
	  execvp (cmd[0],cmd);
	  fprintf (stderr,"error with executing command\n");
	} else if (cur_mode == REDIRECT_OF_INPT) { /*similiar to perivous if block, reading eriting switched*/
	  if (file_flag == 1 && input_flag == 1) {
	    freopen (*file_name,"r",stdin);
	    freopen (*file2_name,"w",stdout);
	  } else { 
	    file = freopen(*file_name,"r",stdin);
	  }
	  execvp (cmd[0],cmd);
	  fprintf (stderr,"error with executing command\n");
	/*
	  Checks to see if the process running in the background will need to read or write
	  to a file. 
	*/
	} else if (cur_mode == RUN_IN_BACKGRD) {
	  /*background process will read or write the file names.
	  */
	  if (file_flag == 1 && input_flag == 1) { 
	    if (backgrd == 1) { 
	    /*background == 1 indicates that file_name is used to read
	      and file2_name is used to write out
	    */
	      freopen(*file_name,"r",stdin);
	      freopen(*file2_name,"w",stdout);
	    } else {
	    /*reading and writing for the files is switched*/
	      freopen (*file_name,"w",stdout);
	      freopen (*file2_name,"r",stdout);
	    }
	  /*only one file read or write is running in the background*/  
	  } else if (file_flag == 1) {
	     freopen(*file_name,"w",stdout);
	    
	  } else if (input_flag == 1) {
	    freopen(*file_name,"r",stdin);
	  }
	  /*sends the ignore signal to the waiting process and excutes
	   the command*/
	  signal (SIGCHLD,SIG_IGN);
	  execvp (cmd[0],cmd);
	  fprintf (stderr,"error with excuting command\n");
	} else {
	  execvp (cmd[0],cmd);
          fprintf (stderr,"error with excuting command\n");
	}
      } else {
        /*in parent*/
	/*if its not a background process then we wait and block */
        if (cur_mode != RUN_IN_BACKGRD){
	  w_pid = waitpid(pid,&status,0);
	}
      }  
    } else {
      fprintf (stderr, "Child process failed to be created");
    }
}

int main(int argc, char *argv[]) {
  int i;
  int exit_status = EXIT_SUCCESS;
  int cd_flag = 0;             /*flag value for cd*/
  int file_flag = 0;           /*file redirect falg*/
  int input_flag = 0;          /*file input redirect flag*/
  int backgrd = 0;             /*run in background flag hat specifics which file
                                 if value is 1 then file_ptr is read else if 
				 the value is 2 then file_ptr is write */
  int status;                  /*the status of the child process in waitpid*/
  int w_pid;                   /*the value returned from waitpid()*/
  int shell_mode = DEFAULT;
  char **args;
  char *file_ptr = NULL;       /*file name pointer for file redirection*/
  char *file_ptr2 = NULL;      /*file name pointer for second file*/
  
  while(1) {
  
  file_ptr = NULL;
  file_ptr2 = NULL;
  file_flag = 0;
  input_flag = 0;
  backgrd = 0;
  shell_mode = DEFAULT;
  /* Checks and waits for all child processes to complete. If there 
      are no more children processes, the while loop exits and the 
      we retrieve another command from  stdin.
  */
   while ((w_pid = waitpid(-1,&status,WNOHANG)) > 0 ){}

   /*prompt*/
   printf("shell~ ");
   /* gets a comand from terminal*/
   args = get_line();
   if (args[0] != NULL) {
     if(!args) break;
     /*check's for exit command*/
     if (strcmp(args[0],"exit") == 0) {
       exit(exit_status);
     }
     /*sets the cd_flag*/
     if (strcmp(args[0],"cd") == 0) {
       cd_flag = 1;
     }

     /*Check and set the current mode for the shell along with all the flags.
       The file_flag is used to indicate if we have a redirection to a file.
       The input_flag is used to indicate if we have a redirection of input.
       The backgrd flag is used to specifiy which file redirection comes first
       in the args. backgrd = 1 if input redirectin is first. backgrd = 2 if 
       redirection to a file is first.
     */
     for(i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i],"<") == 0) {
          /* sets the file name and removes the file form the command*/
	  if (input_flag == 1) {
            fprintf(stderr,"shell does not support multi file input redirect\n");
	  } else if (file_flag == 0) {
	    if (args[i+1] != NULL) {
	      file_ptr = args[i+1];
	    }
            shell_mode = REDIRECT_OF_INPT;
            backgrd = 1;
          } else { 
            if (args[i+1] != NULL) {
	      file_ptr2 = args[i+1];
	    }  
          }  
	  args[i] = '\0';
	  input_flag = 1;
	  /* check to see if muiltple commands shell commands are in the 
	     args.
	  */
        } else if (strcmp(args[i],">") == 0) {
          if (file_flag == 1) {
	    fprintf(stderr, "shell does not support multi file output redirect\n");
          } else if (input_flag == 0) {
	    if (args[i+1] != NULL) {
              file_ptr = args[i+1];
	    }
            shell_mode = REDIRECT_TO_FILE;
            backgrd = 2;
	  } else {
            if (args[i+1] != NULL) {
	      file_ptr2 = args[i+1];
	    }  
	  }  
          args[i] ='\0';
	  file_flag = 1;
        } else if (strcmp(args[i],"&") == 0) {
          args[i] = '\0';
          shell_mode = RUN_IN_BACKGRD;
	  break;
        } else {
          /* shell_mode = DEFAULT;*/
        }
     }
     /*
       executes the cd command and ignores all other inputs except 
       the first arg[1]
     */
     if (cd_flag) {
       int error;
       cd_flag = 0;
       if ( args[1] == NULL) {
         error = chdir("~");
       } else {
         error = chdir(args[1]);
       }
       if (error == -1) {
         fprintf (stderr,"error during cd command\n");
       }
      /*
         Error handling for file redirection cmd by verifing
         the file_ptr and file_ptr2 is not null and no other shell command mode
	 is specified as the file names. If there are no error thrown then 
	 we call the execute_cmd function
      */
      } else if (shell_mode == REDIRECT_OF_INPT || shell_mode == REDIRECT_TO_FILE 
                || input_flag || file_flag) {
	if (file_flag && input_flag) {
	  if (file_ptr == NULL || file_ptr2 == NULL) {
	     fprintf (stderr, "no file entered\n");
	  } else if (strcmp(file_ptr,"<") == 0 ||
	             strcmp(file_ptr,">") == 0 ||
		     strcmp(file_ptr,"&") == 0 ||
		     strcmp(file_ptr2,"<") == 0 ||
		     strcmp(file_ptr2,">") == 0 ||
		     strcmp(file_ptr2,"&") == 0) {
            fprintf (stderr,"invalid argument for file name \n");
          } else {
            execute_cmd (shell_mode,args, &file_ptr, &file_ptr2,file_flag,input_flag,backgrd);
	  }
	} else {
        if (file_ptr == NULL) { 
	      fprintf (stderr, "no file entered\n");
	     } else if ( strcmp(file_ptr,"<") == 0 ||
	                strcmp(file_ptr,">") == 0 ||
	                strcmp(file_ptr,"&") == 0) {
	       fprintf (stderr, "invalid argument for file name\n");
	     } else {
	       execute_cmd (shell_mode, args, &file_ptr, &file_ptr2, file_flag,input_flag,backgrd);
	     }
        }
      } else { 
        execute_cmd (shell_mode, args, &file_ptr, &file_ptr2, file_flag, input_flag,backgrd);
      }
    }
  }
  return exit_status;
}
