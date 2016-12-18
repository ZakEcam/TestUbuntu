#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

int main( void )
{
	//Current working directory
    char* cwd;
	//Maximum number of bytes in a pathname, including the terminating null character.
	//Minimum Acceptable Value: {_POSIX_PATH_MAX}
    char buffer[PATH_MAX + 1];
	//Get current working directory
    cwd = getcwd( buffer, PATH_MAX + 1 );
    if( cwd != NULL ) {
        printf( "%s\n", cwd );
    }

    return EXIT_SUCCESS;
}