#include <assert.h>
#include <ctype.h>
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

int main(int argc, char *argv[]) {
	//flags -p -v
	int flag_pid = 0;
	int flag_version = 0;
	//Variables
	int nread, bpos;
	int c;
	char argument;
	int fd, rel_fd, rel_fd2;
	//Buffers size 1024
	char buf[BUF_SIZE];
	//Structs of directory entries
	struct linux_dirent *dent;
	//Directory and files
	char d_type;
	//For error on cmd and ID calling the name used
	char *cmd_name;
	cmd_name = argv[0];
	char *p_id;
	p_id = argv[2];

	struct option opt[] ={
			{"help", no_argument, NULL, 'h'},
			{"pid", required_argument, NULL, 'p'},
			{"version", no_argument, NULL, 'v'},
			{0,0,0,0}
	};
	while ((c = getopt_long(argc, argv, "hpv",opt, NULL)) != -1){
		switch(c){
				case 'h':
					printf("%s", "help");
					break;
				case 'p':
					flag_pid =  1;
					break;
				case 'v':
					flag_version =  1;
					break;
				case '?':
				default :
					fprintf(stderr, "%s: option '-%c' is invalid.\n", cmd_name, opt);
					abort();
		}
	}
	
	// "/proc" process information pseudo-file system
	fd = open("/proc", O_RDONLY | O_DIRECTORY);
	
	if (fd<0){
		fprintf(stderr,"%s: cannot open %s: %s\n", cmd_name, "this directory", strerror(errno));
        return EXIT_FAILURE;
	}
	
	// Read
	while (1) {
		//getdents(2) Manpage
		nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
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
			//Process IDs are numbers
			argument = *d_name;
			if(isdigit(argument)){
				// != open(2) - openat(2) pathname is not taken from root but from cwd here /proc/
				rel_fd = openat(fd, dent->d_name, O_RDONLY | O_DIRECTORY);
				if (rel_fd<0){
					fprintf(stderr,"%s: cannot open %s: %s\n", cmd_name, "this directory", strerror(errno));
					return EXIT_FAILURE;
				}
				/*proc/<pid>/comm
				These files provide a method to access a tasks comm value. It also allows for
				a task to set its own or one of its thread siblings comm value. The comm value
				is limited in size compared to the cmdline value, so writing anything longer
				then the kernel's TASK_COMM_LEN (currently 16 chars) will result in a truncated
				comm value.
				https://www.kernel.org/doc/Documentation/filesystems/proc.txt
				*/
				rel_fd2 = openat(rel_fd, "comm", O_RDONLY);
				if (rel_fd2<0){
					fprintf(stderr,"%s: cannot open %s: %s\n", cmd_name, "this directory", strerror(errno));
					return EXIT_FAILURE;
				}
				// -p
				if (flag_pid) {
					//Check if ID exists
					if (strcmp(dent->d_name, p_id) == 0) {
						int process_found;
						process_found = 1;
						//Read the buffer of the relative file descriptor
						while((read(rel_fd2, buf, BUF_SIZE)) > 0){
							printf("%s %16s", dent->d_name, buf);
						}
					}
					//Wrong ID error 
					if(!process_found){
						printf("Wrong ID %s", p_id);
						return EXIT_FAILURE;
					}
                }
				//Read and print /comm 
				else {
					while((read(rel_fd2, buf, BUF_SIZE)) > 0) {
							printf("%s %16s", dent->d_name, buf);
					}
				}	
			}
			bpos += dent->d_reclen;
		}
	}
	//Close all file descriptors
	close(rel_fd2);
	close(rel_fd);
	close(fd);
	return EXIT_SUCCESS;	
}