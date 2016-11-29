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
/*Functions*/
//Ordering from an old array to a new array and using qsort manpage
/* void qsort(void *base, size_t nmemb, size_t size,
int (*compar)(const void*, const void*));*/
int my_array(int *current_ar_len, char *to_add_elem,int pos_elem, char ***to_update_array){
//Check if current array and new array pointers != NULL
assert(current_ar_len);
assert(to_update_array);
//New and old array and respective indexes
char **old_array;
int old_array_len;
char **new_array;
int old_i = 0;
int new_i = 0;

old_array= *to_update_array;
old_array_len= *current_ar_len;
while(new_i < (old_array_len + 1)){
	if (new_i == pos_elem){
		new_array[new_i] = strdup(to_add_elem);
		new_i++;
	} else {
		new_array[new_i] = strdup(to_add_elem);
		free(old_array[old_i]);
		old_i++;
		new_i++;
	}
}
free(old_array);
*current_array_len = old_array_len + 1;
*to_update_array = new_array;
return EXIT_SUCCESS;
}
//Comparaison manpage strcasecmp and strcmp 
//Hidden files must be ordered when using ls -l
int alphaOrder (constant char *a, cont char *b){
	int r = strcasecmp(a,b);
	//ex. : .folder, .file
	int hidden_a = a[0] == '.';
	int hidden_b = b[0] == '.';
	if(hidden_a)
		hidden_a += sizeof(char);
	if(hidden_b)
		hidden_b += sizeof(char);
	if(r){
		return r;
	} else {
		r= -strcmp(a,b);
		if(r)
		return(r);
		if(hidden_a)
		return 1;
		if(hidden_b)
		return -1;
	}
}

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
	//Details needed for option -l manpage stat
	/*ls -l displays
	for ex. : drwxr-xr-x 1(hard links) user group size(bytes) last modification time name*/
	struct stat stats{
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
		if ((dir = open(d_names[d_index], O_RDONLY))<0){
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
