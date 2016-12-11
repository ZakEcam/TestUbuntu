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
#include <utmpx.h>
/* The structure describing an entry in the user accounting database.  */
// struct utmpx
// {
	// short int ut_type;        		/* Type of login.  */
	// __pid_t ut_pid;       			/* Process ID of login process.  */
	// char ut_line[__UT_LINESIZE];    /* Devicename.  */
	// char ut_id[4];        			/* Inittab ID. */
	// char ut_user[__UT_NAMESIZE];  	/* Username.  */
	// char ut_host[__UT_HOSTSIZE];  	/* Hostname for remote login.  */
	// struct __exit_status ut_exit; 	/* Exit status of a process marked as DEAD_PROCESS.  */
// };
/*Public utfuncs
struct utfuncs
{
	int(* 	setutent )(void) 
	int(* 	getutent_r )(struct utmp *, struct utmp **)
	int(* 	getutid_r )(const struct utmp *, struct utmp *, struct utmp **) 
	int(* 	getutline_r )(const struct utmp *, struct utmp *, struct utmp **) 
	struct utmp *(* 	pututline )(const struct utmp *) 
	void(* 	endutent )(void)
	int(* 	updwtmp )(const char *, const struct utmp *)
};
*/
//Main
int main(int argc, char *argv[]) {
	//flags -H -q -r -u
	int flag_heading = 0;
	int flag_count = 0;
	int flag_runlvl = 0;
	int flag_users = 0;
	//Variables
	int c;
	//Needed for -q options number of users
	int nb = 0;
	//Array and structure for TIME
	char time[50];		
	struct tm t;//Unused
	//Structure utmpx
	struct utmpx *ut;
	//For error on cmd 
	char *cmd_name;
	cmd_name = argv[0];
	
	struct option opt[] ={
			{"heading", no_argument, NULL, 'H'},
			{"count", no_argument, NULL, 'q'},
			{"runlevel", no_argument, NULL, 'r'},
			{"users", no_argument, NULL, 'u'},
			{0,0,0,0}
	};
	while ((c = getopt_long(argc, argv, "Hqru",opt, NULL)) != -1){
		switch(c){
				case 'H':
					flag_heading = 1;
					break;
				case 'q':
					flag_count =  1;
					break;
				case 'r':
					flag_runlvl =  1;
					break;
				case 'u':
					flag_users = 1;
					break;
				case '?':
				default :
					fprintf(stderr, "%s: option '-%c' is invalid.\n", cmd_name, optopt);
					abort();
		}
	}
	/*utmp maintains a full accounting of the current status of the system, 
	system boot time (used by uptime),recording user logins at which terminals, 
	logouts, system events etc.*/
	//Path for utmp file /var/run/utmp
	
	//Function resetting user accounting database to first entry
	setutxent();
	//Main while - getutxent() : Read next entry in utmpx database
	while ((ut = getutxent()) != NULL) {
		/* Values for ut_type field

           #define EMPTY         0 /* Record does not contain valid info
                                      (formerly known as UT_UNKNOWN on Linux) 
           #define RUN_LVL       1 /* Change in system run-level (see
                                      init(8)) 
           #define BOOT_TIME     2 /* Time of system boot (in ut_tv) 
           #define NEW_TIME      3 /* Time after system clock change
                                      (in ut_tv) 
           #define OLD_TIME      4 /* Time before system clock change
                                      (in ut_tv) 
           #define INIT_PROCESS  5 /* Process spawned by init(8) 
           #define LOGIN_PROCESS 6 /* Session leader process for user login 
           #define USER_PROCESS  7 /* Normal process 
           #define DEAD_PROCESS  8 /* Terminated process 
           #define ACCOUNTING    9 /* Not implemented */
	
		//Print current runlevel who -r
		char a;
		if (flag_runlvl){
			if (ut->ut_type == 1) {
				a = ut->ut_pid / 256;
				if (a == 0) a = 'R';
				printf("%c %c\n", a, ut->ut_pid % 256);
				//TIME
                /*if (asctime_r(&ut->ut_tv.tv_sec, &time) == NULL) {
                    perror(cmd_name);
                    return EXIT_FAILURE;
                }*/
                printf("%s\t", time);
				//Close the utmpx database
				endutxent();
				return EXIT_SUCCESS;
			}
		}
		//Pint Normal process
        if (ut->ut_type == USER_PROCESS) {
			//Print line of column headings who -H
			if (flag_heading && nb == 0 && !flag_users)
                    printf("NAME\t LINE\t TIME\t \t \t COMMENT\n");	
			if (flag_heading && nb == 0 && flag_users)
                    printf("NAME\t LINE\t TIME\t \t \tIDLE\tPID\t COMMENT\n");	
			//Print all login names and number of users logged on who -q
            if (flag_count) {
				//NAME
                printf("%s ", ut->ut_user);
			//Basic who
            } else {
                //NAME
                printf("%s\t", ut->ut_user);
				//LINE
                printf("%s\t", ut->ut_line);
				//TIME
                /*if (asctime_r(&ut->ut_tv.tv_sec, &time) == NULL) {
                    perror(cmd_name);
                    return EXIT_FAILURE;
                }*/
                printf("%s\t", time);
				// List users logged in who -u
				if (flag_users) {
					//IDLE !!!!!find a way to print idle time ? . recently active etc
					//printf("%s ", ut->ut_pid);
					//PID
					printf("%d ", ut->ut_pid);
				}
				//COMMENT
                printf("(%s)\t", ut->ut_host);
                printf("\n");
            }
            nb++;
        }
    }
	//After the while - getting the number of users from nb value
	if (flag_count)
        printf("\n#users=%d\n", nb);
	//Close the utmpx database
	endutxent();
	return EXIT_SUCCESS;	
}
