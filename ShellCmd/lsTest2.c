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
	int flag_all = 0;
	int flag_long = 0;
	int flag_recursive = 0;

	int fd, nread;
	char buf[BUF_SIZE];
	struct linux_dirent *d;
	int bpos;
	char d_type;

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
	exit(EXIT_SUCCESS);
}
