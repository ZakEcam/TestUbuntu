
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <glob.h>

#define BUFFER_SIZE 4096//Size of buffer set to 4096

int main()
{
    char buffer[BUFFER_SIZE];

    while(1)//Main while
    {
        printf("sh4 > ");//Shell line
        fgets(buffer,BUFFER_SIZE,stdin);//Read the input and avoid buffer overflow(>< scanf)
        buffer[strlen(buffer)-1]='\0';//Clear the carriage return by setting the next to char
        if (strcmp("quit",buffer)==0)//Quit input
        {
            exit(1);
        }
        char *p=strdup(buffer);//Sending a pointer on a new chains of char duplicate from our buffer
        pid_t process=fork();//Creation of a process son 
		if (pid < 0)//Error
		{
			printf("Failed fork.\n",strerror(errno));
			return;
		}
        if (process==0)//we are in the son process
        {
			char exec[150];//Executable 
			char *tmp=strtok(p," ");//Cut the chains p by using space (each word between space will be separate)
			char *sgn=strtok(NULL," ");
			char *opt=strtok(NULL," ");
			char arg=strtok(NULL," ");
			sprintf(exec, "./%s", tmp);//Sends formatted output to a string pointed to, by exec eg.: ./myls
			execlp(exec, tmp, sgn, opt, arg, NULL);//Execute a file
			printf("%s\n",strerror(errno));//Error handling
            exit(1);
        }
        else//we are in the father process, waiting until the son has finished //waitpid(2) - Linux man page
        {
            int status;
			do {
					w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);//Wait for process to change state
					if (w == -1) {
						perror("waitpid");
						return EXIT_FAILURE;
					}
					if (WIFEXITED(status)) {
						printf("exited, status=%d\n", WEXITSTATUS(status));
					} else if (WIFSIGNALED(status)) {
						printf("killed by signal %d\n", WTERMSIG(status));
					} else if (WIFSTOPPED(status)) {
						printf("stopped by signal %d\n", WSTOPSIG(status));
					} else if (WIFCONTINUED(status)) {
						printf("continued\n");
					}
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
			return EXIT_SUCCESS;
		}
        
        free(p);
    }
	
	return 0;
}
