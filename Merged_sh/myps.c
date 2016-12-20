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

#define VERSION 1.0

#define BUF_SIZE 1024
//Structure for directory entry
struct linux_dirent {
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name[];
};
//Main
int main(int argc, char *argv[]) {
	//flags -h -p -v
	int flag_help = 0;
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
	//For error on cmd and ID calling the name/numbers used
	char *cmd_name;
	cmd_name = argv[0];
	char *p_id;
	p_id = argv[2];
	int process_found;
	struct option opt[] ={
			{"help", no_argument, NULL, 'h'},
			{"pid", required_argument, NULL, 'p'},
			{"version", no_argument, NULL, 'V'},
			{0,0,0,0}
	};
	while ((c = getopt_long(argc, argv, "hp:V",opt, NULL)) != -1){
		switch(c){
				case 'h':
					flag_help = 1;
					break;
				case 'p':
					flag_pid =  1;
					break;
				case 'V':
					flag_version =  1;
					break;
				case ':':
					fprintf(stderr, "%s: option '-%c' requires argument.\n", cmd_name, optopt);
					break;
				case '?':
				default :
					fprintf(stderr, "%s: option '-%c' is invalid.\n", cmd_name, optopt);
					abort();
		}
	}
	// -h
	if (flag_help){
		printf("%s", "ps -p [p_id] or --pid [p_id] : search process by its ID \n");
		return EXIT_SUCCESS;
	}
	// -V
	if(flag_version){
		printf("my ps version %.1f \n",VERSION);
		return EXIT_SUCCESS;
	}
	//Missing argv[2]--------Avoiding segmentation core fault
	if ((flag_pid) && (p_id == NULL)){
		printf("Need argument for this option");
		return EXIT_FAILURE;
	}
	// "/proc" process information pseudo-file system
	fd = open("/proc", O_RDONLY | O_DIRECTORY);
	//Error
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
			argument = *dent->d_name;
			if(isdigit(argument)){
				// != open(2) - openat(2) pathname is not taken from root but from cwd here /proc/
				rel_fd = openat(fd, dent->d_name, O_RDONLY | O_DIRECTORY);
				//Error
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
					//If name in directory entries = input argument
					if (strcmp(dent->d_name, p_id) == 0) {
						process_found = 1;
						//Read the buffer of the relative file descriptor
						while((read(rel_fd2, buf, BUF_SIZE)) > 0){
							printf("%s %16s", dent->d_name, buf);
						}
					}
					//Wrong process ID - Error 
					if(!process_found){
						printf("Wrong ID %s", p_id);
						return EXIT_FAILURE;
					}
                }
				//Read and print /comm - basic ps 
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
