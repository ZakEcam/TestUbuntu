#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 1024
//Structure for directory entry
struct linux_dirent {
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name[];
};
//------------Functions---------//
//Checking type
int if_dir(char *file)
{
	struct stat stats;
	if(stat(file, &stats)==-1){
		return -1;
	}
	return S_ISDIR(stats.st_mode);
}

int if_reg(char *file)
{
	struct stat stats;
	if(stat(file,&stats)==-1){
		return -1;
	}
	return S_ISREG(stats.st_mode);
}

//Using struct stats
int file_stats(int fd, char *file, struct stat **stat)
{
    struct stat *f_stats = *stat;
    if (fd < 0) {
        if (lstat(file, f_stats))
            return EXIT_FAILURE;
    } 
    *stat = f_stats;
    return EXIT_SUCCESS;
}

int file_size(struct stat *stats){
	return stats->st_size;
}
//User ID is given with st_uid, using struct passwd to get user information
//mkssoftware.com/docs/man5/struct_passwd.5.asp
int file_user(struct stat *stats, char **user){
	struct passwd *pass;
	pass = getpwuid(stats->st_uid);
	if(pass == NULL)
		return EXIT_FAILURE;
	*user = strdup(pass->pw_name);
	if(*user == NULL)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
//Group ID
int file_group(struct stat *stats, char **group){
	struct group *group_name;
	group_name=getgrgid(stats->st_gid);
	if(group_name ==NULL)
		return EXIT_FAILURE;
	if(*group == NULL)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
//Modification time !!!!! long int or char for time
/*int mtime(struct stat *stats, char **time){
	time=ctime(&stats.st_mtim);
	printf("%s",time); 
}*/
//Permissions d-xr-wr-x
int file_permission(struct stat *stats, char **permission){
	mode_t mode;
	char *str = calloc(11,sizeof(char));
	if (str==NULL)
		return EXIT_FAILURE;
	mode = stats->st_mode;
	str[0]='\0';
	if(S_ISREG(mode)) strcat(str,"-");
	if(S_ISDIR(mode)) strcat(str,"d");
	//User Permission
	//Read
	if(S_IRUSR & mode) strcat(str,"r");
	else strcat(str,"-");
	//Write
	if(S_IWUSR & mode) strcat(str,"w");
	else strcat(str,"-");
	//Execute
	if(S_IXUSR & mode) strcat(str,"x");
	else strcat(str,"-");
	//Group Permission
	//Read
	if(S_IRGRP & mode) strcat(str,"r");
	else strcat(str,"-");
	//Write
	if(S_IWGRP & mode) strcat(str,"w");
	else strcat(str,"-");
	//Execute
	if(S_IXGRP & mode) strcat(str,"x");
	else strcat(str,"-");
	//Others  Permission
	//Read
	if(S_IROTH & mode) strcat(str,"r");
	else strcat(str,"-");
	//Write
	if(S_IWOTH & mode) strcat(str,"w");
	else strcat(str,"-");
	//Execute
	if(S_IXOTH & mode) strcat(str,"x");
	else strcat(str,"-");

	*permission = str;
	return EXIT_SUCCESS;
}
//Display -----------------Unused
char* slash(const char *a, const char *b){
	char *final;
	//Memory allocation for ex. /home/folder 2 char + 2/
	if ((final = malloc(strlen(a) + strlen(b) + (1 + 1) *sizeof(char))) == NULL)
		return final;
	final[0] = '\0';
	strcat(final,a);
	strcat(final,"/");
	strcat(final,b);
	return final;
}
//-------------------------------//
int main(int argc, char *argv[]){
/*Test main */
	printf("%s \n","Variables");
	//flags -a -l -R
	int flag_all = 0;
	int flag_long = 0;
	int flag_recursive = 0;
	// ! Testing gcc lstest printed all in disorder
	int disorder_flag = 0;///! work on qsort
	//Hidden files . .. unused just check [0]
	//int f_hidden;
	//Variables
	int nread, bpos;
	int nread2, bpos2;
	int c,fd1,fd2;
	//Buffers size 1024
	char buf[BUF_SIZE];
	char buf2[BUF_SIZE];
	//Structs of directory entries
	struct linux_dirent *dent;
	struct linux_dirent *dent2;
	//Directory and files
	char d_type;
	//For error on cmd calling the name used
	char *cmd_name;
	cmd_name = argv[0];
	//Details Variables
	char *permission;
	char *user;
	char *group;
	//Options
	struct option opt[] =
	{
		{"all", no_argument, NULL, 'a'},
		{"recursive", no_argument, NULL, 'R'},
		{0,0,0,0}
	};
	while ((c = getopt_long(argc, argv, "alR",opt, NULL)) != -1){
		switch(c){
			case 'a':
				flag_all =  1;
				break;
			case 'l':
				flag_long =  1;
				break;
			case 'R':
				flag_recursive =  1;
				break;
			case '?':
			default :
				abort();
		}
	}
	//No need for redefining struct stat
	//Directory and file stats
	struct stat d_stats;//Unused
	struct stat *f_stats;
	///////////////
	char *argu = argv[optind];
	//Directory on argument
	if ( argu != NULL) {
		char array[70];
		sprintf(array, "%s", argv[optind]);
		fd1 = open(array , O_RDONLY | O_DIRECTORY);
	}
	//"." current directory
	else
		fd1 = open(".", O_RDONLY | O_DIRECTORY);

	if (fd1<0){
		fprintf(stderr,"%s: cannot open %s: %s\n", cmd_name, "this directory", strerror(errno));
        return EXIT_FAILURE;
	}
	//Open/read
	while (1) {
		//getdents(2) Manpage
		nread = syscall(SYS_getdents, fd1, buf, BUF_SIZE);
		//Error 
		if (nread == -1){
			perror("nread == -1");
			return EXIT_FAILURE;
		}
		//Break from main While
		if (nread == 0)
			break;
		//While bpos
		int bpos =0;
		while (bpos < nread) {
			//Cast 
			dent = (struct linux_dirent *)(buf + bpos);
			d_type = *(buf + bpos + dent->d_reclen - 1);
			//Basic ls 
			if ( (!flag_all) && (!flag_long) && (*dent->d_name != '.')) {
				printf("%s\n", dent->d_name);
			}
			//All even hidden files
			if (flag_all) {
				printf("%s\n", dent->d_name);
			}
			//Details
			if ((flag_long) && (*dent->d_name != '.')) {
				///////
				// Get file stats
                // Permission 
				file_permission(f_stats, &permission);
				printf("%s ", permission);
				//printf("%s \n", "free permission");
				free(permission);
				// User
				file_user(f_stats, &user);
				printf("%s\t", user);
				//printf("%s \n", "free user");
				free(user);
				// Group 
				file_group(f_stats, &group);
				printf("%s\t", group);
				//printf("%s \n", "free group");
				free(group);
				// Size
				printf("%d\t", file_size(f_stats));
				// Modification time	
				printf("%s\t", ctime(&f_stats.st_mtime));
				//////
				if (stat(dent->d_name, &f_stats) == -1) {
					fprintf(stderr, "%s: cannot look at %s: %s\n", cmd_name, "this directory stats", strerror(errno));
                    return EXIT_FAILURE;
				}
		}
			//Recursive flag on subdirectories "Whileception"
			if ((flag_recursive) && (d_type == DT_DIR) && (*dent->d_name != '.')){
				if ((d_type == DT_DIR) & (*dent->d_name != '.')){
					printf("Subdirectories/files :\n");
					fd2 = open(dent->d_name, O_RDONLY | O_DIRECTORY);
					nread2 = syscall(SYS_getdents, fd2, buf2, BUF_SIZE);
					int bpos2 = 0;
					while (bpos2 < nread2 ) {
						dent2 = (struct linux_dirent *)(buf2 + bpos2);
						d_type = *(buf2 + bpos2 + dent2->d_reclen - 1);
						if (*dent2->d_name != '.')
							printf("---->%s\n",dent2->d_name);
						bpos2 += dent2->d_reclen;
					}
					close(fd2);
					//Split
					printf("-----subs-----\n");
				}
			}
			bpos += d->d_reclen;
			}
		}
		close(fd);
		exit(EXIT_SUCCESS);		
	}	
	///////////////
	
}