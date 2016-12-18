#include<fcntl.h>
#include <getopt.h>
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
int main(int argc,char *argv[])
{	
	//flags -E -n
	int flag_E = 0;
	int flag_n = 0;
	int fd=0;
	int line = 0;
	int r;
	char buffer[100];
	struct stat s;
	struct option opt[] ={
		{"number", no_argument, NULL, 'n'},
		{"show-ends", no_argument, NULL, 'E'},
		{0,0,0,0}
	};
	while ((c = getopt_long(argc, argv, "En",opt, NULL)) != -1){
		switch(c){
			case 'E':
				flag_E = 1;
				break;
			case 'n':
				flag_n =  1;
				break;
			case '?':
			default :
				abort();
		}
	}
	//Missing or overflowed argument
	//Basic cat with one argument only
	if(argc!=2)
	{
		printf("Mismatch argument");
		//exit(1);
		return EXIT_FAILURE;
	}
	//Check real user's permissions for a file
	if(access(argv[1],F_OK))
	{
		printf("File Exist");
		return EXIT_FAILURE;
	}
	//Get file status
	//argv[1] for file name eg.: cat folder1
	if(stat(argv[1],&s)<0)
	{
		printf("Stat ERROR");
		return EXIT_FAILURE;
	}
	//This macro returns non-zero if the file is a regular file
	if(S_ISREG(s.st_mode)<0)
	{
		printf("Not a Regular FILE");
	   	return EXIT_FAILURE;
	}
	//Get user identity and check if it matches 
	if(geteuid()==s.st_uid)
		if(s.st_mode & S_IRUSR)
			fd=1;
	//Get group identity and check if it matches 
	else if(getegid()==s.st_gid)
		if(s.st_mode & S_IRGRP)
			fd=1;
	//Get others identity and check if it matches 
	else if(s.st_mode & S_IROTH)
		fd=1;
	//No user/group/others permission
	if(!fd)
	{
		printf("Permission denied");
		return EXIT_FAILURE;
	}
	//Open
	fd=open(argv[1],O_RDONLY);
	//Read / Write
	while((r=read(fd,buffer,100))>0){
		//Print line number
		if (flag_n) {
            fprintf(stdout, "%4d  ", ++line);
            fflush(stdout);
        }
		//Basic cat
		write(1,buffer,r);
	}	   
	return EXIT_SUCCESS;
}