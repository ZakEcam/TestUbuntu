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

//Functions

//Comparison Manpage strcasecmp and strcmp 
//Hidden files must be ordered when using ls -l
/*int alphaOrder (const void *a, const void *b){
	//Need to return an integer
	char *acast = *(char**)a;
	char *bcast = *(char**)b;
	//ex. : .folder, .file
	int hidden_a = acast[0] == '.';
	int hidden_b = bcast[0] == '.';
	int r = strcasecmp(acast,bcast);
	if(hidden_a == 1)
		acast += sizeof(char);
	if(hidden_b == 1)
		bcast += sizeof(char);
	if(r == 1){
		return r;
	} else {
		r= -strcmp(acast,bcast);
		if(r == 1)
		return(r);
		if(hidden_a == 1) 
		return 1;
		if(hidden_b == 1)
		return -1;
	}
}*/
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
//Affichage
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

int main(int argc, char *argv[]){
/*Test main */
	printf("%s \n","Variables");
	//flags -a -l -R
	int flag_all = 0;
	int flag_long = 0;
	int flag_recursive = 0;
	// ! Testing gcc lstest printed all in disorder
	int disorder_flag = 0;
	//Hidden files . ..
	int f_hidden;
	//Variables
	int nread, bpos;
	int c,fd;
	//Buffer size 1024
	char buf[BUF_SIZE];
	//Struct of directory entries
	struct linux_dirent *dent;
	//Directory and files
	char **d_names;
	char **files;
	int filelength;
	int path;
	int d_index;
	char *path_all;
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
	//Details needed for option -l manpage stat
	/*ls -l displays
	for ex. : drwxr-xr-x 1(hard links) user group size(bytes) last modification time name*/
	/*struct stat stats{
		dev_t st_dev;	 //ID of device containing file
		ino_t st_ino;	 //inode number
		mode_t st_mode;  //file type or mode
		nlink_t st_nlink;//number of hard links
		uid_t st_uid; 	 //User ID of owner
		gid_t st_gid;    //group ID of owner
		dev_t st_rdev;
		off_t st_size;   //total size in bytes
		blksize_t st_blksize;
		blckcnt_t st_blocks;
		
		struct timespec st_atim;
		struct st_mtim;  //time of last modification
		struct timespec st_ctim;
	}*/
	printf("%s \n","Argc c check");

	//No need for redefining struct stat
	//Directory and file stats
	struct stat d_stats;
	struct stat *f_stats;
	//Check if current directory or subdirectory using optindex
	/*optind = 1, So if there is argc = 1, there are no options and
	 arguments/path */
	extern int optind;
	//Test Segmentation core dumped
	if (argc < 2)
		printf("%s \n", "arg c < 2");
	if (argc - optind == 0){
		path = 1;
		printf("%s \n", "path = 1");
		//Added to avoid segmetation core dumped
		if((d_names = calloc(path, sizeof(char*))) == NULL){
			perror(cmd_name);
			return EXIT_FAILURE;
		}
		printf("%s \n", "After calloc");

		d_names[0] = strdup(".");
		printf("%s \n", "string duplicate ");
		
	} else {
		printf("%s \n", "path !=1");
		
		//Not current directory argv[2] and more /sub/sub ...
		path = argc - optind;
		//int i = optind;
		int i = 1;
		while(i<argc){
			int index_path = i - optind;
			d_names[index_path] = strdup(argv[i]);
			i++;
		printf("%s \n", "while i<argc");
		}
	}
	
	//Main for open/read etc.
	//while(d_index<path){
	for(d_index = 0; d_index<path; d_index++){
		printf("%s \n", "main while");
		
		//Open(2) Manpage
		if ((fd = open(d_names[d_index], O_RDONLY))<0){
			perror(cmd_name);
			return EXIT_FAILURE;
			printf("%s \n", "value return open<0");
		}
		if (stat(d_names[d_index], &d_stats)){
			perror(cmd_name);
			return EXIT_FAILURE;
			printf("%s \n", "stats error");
		}
		
		//If we have a file. S_ISDIR : This macro returns non-zero if the file is a directory
		 if (!S_ISDIR(d_stats.st_mode)) {
			printf("%s \n", "we have a file!");
            		filelength = 1;
			//Allocate memory dynamically
            		files = calloc(filelength, sizeof(char*));
            		files[0] = strdup(d_names[d_index]);
            		close(fd);
			//Below EXIT_FAILURE
            		fd = -1;
        	} else {
			//Last Warning segm core
			size_t length = 0;
			char **array;
			*(&filelength) = 2;
			*(&files) = calloc(*(&filelength),sizeof(char*));
			if (*&(files) == NULL)
				return EXIT_FAILURE;
			//////
			if (fd < 0) {
				printf("%s \n", "fd<0");
				return EXIT_FAILURE;
			} else {
				while ((nread = syscall(SYS_getdents, fd, buf, BUF_SIZE)) > 0) {
					printf("%s \n", "while syscall");
					bpos = 0;
					while (bpos < nread) {
						printf("%s \n", "while bpos");
						//Cast
						dent = (struct linux_dirent *) (buf + bpos);
						///Reallocating memory if needed
						if (length == *(&filelength)){
							printf("%s \n","length ==filelength");
							// x2
							*(&filelength) *=2;
							array = realloc(*(&files),*(&filelength) *sizeof(char*));
							*(&files) = array;
						}
						printf("%s \n","Before strdup dent");
						*(&files[length]) =strdup(dent->d_name);
						printf("%s \n","After strdup dent");
						length++;
						printf("%s \n","increment length");
						printf("nread = %d \n",nread);
						printf("before increment bpos = %d \n",bpos);
						///
						bpos += dent->d_reclen;
						printf("after increment bpos = %d \n",bpos);

					}
				}
			}
		///Segmentation core error again length != filelength
		printf("%s \n","length != filelength");
		array = realloc(*(&files),length *sizeof(char*));
		*(&files) = array;
		*(&filelength) = length;

		///
		printf("%s \n", "exit succes");
		//return EXIT_SUCCESS;
		}
		
		if (flag_recursive == 1){
		//if (flag_recursive==1)
			printf("%s \n", "Recursive flag == 1");
            		printf("%s:\n", d_names[d_index]);
		}
//		if (disorder_flag == 0)
			//Ordering directories and files
   // !segmentation        qsort(files, filelength, sizeof(char*), alphaOrder);
	int a;		
        for(a = 0; a < filelength; a++) {
            printf("%s \n", "for a");
            f_hidden = files[a][0] == '.';
            // Filter hidden files
            if (flag_all == 1) {
		printf("%s \n", "flag all");
                // Get file stats
                file_stats(fd, files[a], &f_stats);

                if (flag_recursive == 1) {
                    path_all = slash(d_names[d_index], files[a]);  
		    printf("%s \n", "free path all");
                    free(path_all);
                }
                // Details
                if (flag_long == 1) {
                    // Permission 
		    file_permission(f_stats, &permission);
                    printf("%s ", permission);
		    printf("%s \n", "free permission");
                    free(permission);
                    // User
           	    file_user(f_stats, &user);
                    printf("%s\t", user);
		    printf("%s \n", "free user");
                    free(user);
                    // Group 
		    file_group(f_stats, &group);
                    printf("%s\t", group);
 		    printf("%s \n", "free group");
                    free(group);
                    // Size
                    printf("%d\t", file_size(f_stats));
                    // Modification time
/*		    struct stat t_stats;
                    printf("%s\t", ctime(&t_stats.st_mtime));*/
					
                } else { // No details
                    printf("%s\n", files[a]);
                }//Forgot to print if not -a
		else{
                    printf("%s\n", files[a]);
		}
      	        printf("%s \n", "free file stats");
                free(f_stats);
            }
	    printf("%s \n", "free files array");
            free(files[a]);
        }
	printf("%s \n", "free files");
        free(files);
        if (fd > -1){
	    printf("%s \n", "close file descriptor/directory");
            close(fd);
	
	}
	}


	/*if argc>1 
		return argv[1]
	  else return "." current directory
	fd= open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
	
	while(1){
		nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
		if (nread ==0)
			break;
		bpos = 0;
		while (bpos<nread){
			d= (struct linux_dirent *) (buf+bpos);
			printf("%4d %s\n", d->d_reclen, d->d_name);
			bpos += d->d_reclen;
		}
	}*/
	
	d_index = 0;
	/*!!!!!!!!!!!!!! ./myls was doing infinite free causing aborted core
	 because missing {} so no increment of directory index*/
	while(d_index<path){
        	printf("%s \n", "free directories array");
		free(d_names[d_index]);
		d_index++;
	}
	printf("%s \n", "free directories");
    	free(d_names);
	printf("%s \n", "End");	
	return EXIT_SUCCESS;
}
