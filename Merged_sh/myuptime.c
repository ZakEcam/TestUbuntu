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
#include <sys/sysinfo.h>

#define BUF_SIZE 1024
//Structure for directory entry
struct linux_dirent {
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	//char d_name[];
};
//Main
int main(int argc, char *argv[]) {
	//flags -p -s
	int flag_pretty = 0;
	int flag_since = 0;
	//Variables
	int c;
	//char argument;
	int fd, fd_uptime;
	//Buffers size 1024
	char buf[BUF_SIZE];
	//For error on cmd calling the name used
	char *cmd_name;
	cmd_name = argv[0];
	struct option opt[] ={
			{"pretty", no_argument, NULL, 'p'},
			{"since", no_argument, NULL, 's'},
			{0,0,0,0}
	};
	while ((c = getopt_long(argc, argv, "ps",opt, NULL)) != -1){
		switch(c){
				case 'p':
					flag_pretty =  1;
					break;
				case 's':
					flag_since =  1;
					break;
				case '?':
				default :
					fprintf(stderr, "%s: option '-%c' is invalid.\n", cmd_name, optopt);
					abort();
		}
	}

	// "/proc" process information pseudo-file system
	fd = open("/proc", O_RDONLY | O_DIRECTORY);
	//Error
	if (fd<0){
		fprintf(stderr,"%s: cannot open %s: %s\n", cmd_name, "this directory", strerror(errno));
        return EXIT_FAILURE;
	}

	// Read
    fd_uptime = openat(fd, "uptime", O_RDONLY);

    //Read uptime in /proc
    read(fd_uptime, buf, BUF_SIZE);
    //cut the buffer in small part
    const char s[2] = " ";
    char *token;
    int sec,min,hours;
    token=strtok(buf, s);

    //Converting second in proper format
    sec = atoi(token)%60;//atoi() convert char in int
    min = atoi(token)/60%60;
    hours = atoi(token)/3600%24;

    //Get current time
   time_t current = time (NULL);
   struct tm current_time = *localtime (&current);
   char s_now[sizeof "HH:MM:SS"];
   strftime (s_now, sizeof s_now, "%H:%M:%S", &current_time);//format date and time manpage strftime


    // -p
	if (flag_pretty){
		printf("up: %d hours %d min\n",hours,min);
		return EXIT_SUCCESS;
	}
	// -s
	if (flag_since){
		struct sysinfo info;//for getting uptime information
        time_t start_time;
        struct tm *elapsed;
        char date[20];
        sysinfo(&info);
        //time elapse with current time
        start_time = current - info.uptime;
        elapsed = localtime(&start_time);
        strftime(date,20 , "%F %T",elapsed);
        printf("%s \n",date);
        return EXIT_SUCCESS;
	}
	printf("Current time:%s System running up: %d hours %d min %d seconds\n",s_now,hours,min,sec );
	//Get system load averages.
    double load[3];
    if (getloadavg(load, 3) != -1)
	{
 		printf("load average : %.2f , %.2f , %.2f\n", load[0],load[1],load[2]);
  	}
  	//Close all file descriptors
	close(fd_uptime);
	close(fd);
	return EXIT_SUCCESS;
}
