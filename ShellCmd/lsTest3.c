#include <getopt.h>

#include <dirent.h>
#include <unistd.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>


#define BUF_SIZE 1024

/*Structure for directory entry*/
struct linux_dirent {
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name[];
};


int main(int argc, char *argv[]){
/*Test main */
	//flags -a -l -R
	int flag_all = 0;
	int flag_long = 0;
	int flag_recursive = 0;
	//! Testing gcc lstest printed all in disorder
	int disorder_flag = 0;
	//Hidden files . ..
	int f_hidden;
	//
	int c;
	//Buffer size 1024
	char buf[BUF_SIZE];
	//Struct of directory entries
	struct linux_dirent *dent;
	//Directory
	char **d_names
	int path;
	int d_index;
	//For error on cmd calling the name used
	char *cmd_name;
	cmd_name = argv[0];
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
	
	//Check if current directory or subdirectory using optindex
	/*optind = 1, So if there is argc = 1, there are no options and
	 arguments/path */
	if (argc - optind == 0){
		path = 1;
		d_names[0] = ".";
	} else {
		//Not current directory argv[2] and more /sub/sub ...
		path = argc - optind;
		int i = optind;
		
		while(i<argc){
			int index_path = i - optind;
			d_names[index_path] = argv[i];
			i++;
		}
	}
	
	//Main for open/read etc.
	for(d_index = 0; d_index<path; d_index++){
		if ((dir = open(path[d_index], O_RDONLY))<0){
			perror(cmd_name);
			exit(EXIT_FAILURE);
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
	}
	exit(EXIT_SUCCESS);*/
}
