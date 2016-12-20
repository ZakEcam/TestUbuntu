#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
int main(int argc,char *argv[])
{
	int fd=0;
	int r;
	char buffer[100];
	struct stat s;
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
	while((r=read(fd,buffer,100))>0)
		write(1,buffer,r);
		   
	return EXIT_SUCCESS;
}
