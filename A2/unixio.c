#include <fcntl.h> // open
#include <unistd.h> // read
#include <sys/types.h> // read
#include <sys/uio.h> // read
#include <stdio.h> // fopen, fread
#include <stdlib.h>
#include <sys/time.h> // gettimeofday

struct timeval start, end; // maintain starting and finishing wall time.

void startTimer( ) { // memorize the starting time
        gettimeofday( &start, NULL );
}

void stopTimer( char *str ) { // checking the finishing time and computes the elapsed time
        gettimeofday( &end, NULL );
printf("%s's elapsed time\t= %ld\n",str, ( end.tv_sec - start.tv_sec ) * 1000000 + (end.tv_usec - start.tv_usec ));

}

int main( int argc, char *argv[] ) {

  // validate arguments
  if (argc != 4) {
    printf("Bad command arguments\n");
    exit(1);
  }

  // Parsing the arguments passed to your C program
  // Including the number of bytes to read per read( ) or fread( ), and
  // the type of i/o calls used
  // (“1” stands for Unix system calls, and “0” stands for C functions).
  char* filename = argv[1];
  int bytes = atoi(argv[2]);
  int typeofcalls = atoi(argv[3]);
  printf("Command Args: %s %d %d\n", filename, bytes, typeofcalls);

  if (typeofcalls == 1) {
    // Use unix I/O system calls to
    startTimer();

    char data[10000];
    int file = open(filename, O_RDONLY);
    if (file < 0) {
      printf("Error opening file\n");
      return 1;
    }

    while (read(file, data, bytes) != 0) {}

    if (close(file) < 0) {
        printf("Error closing the file\n");
        exit(1);
    }

    stopTimer("\t\t\t\tUnix");

  } else if (typeofcalls == 0 && bytes != 1) {
    // Use standard I/O
    startTimer();

    FILE * file;
    char buf[bytes];

    file = fopen (filename, "r");
    if (file == NULL) {
     printf("Error opening file\n");
     return 1;
    }

    while (fread(buf, bytes, 1, file) != 0) {}

    fclose(file);
    stopTimer("\t\tC");
  } else if (typeofcalls == 0 && bytes == 1) {
    // Use standard I/O
    startTimer();

    FILE * file;
    char buf[bytes];

    file = fopen (filename, "r");
    if (file == NULL) {
     printf("Error opening file\n");
     return 1;
    }

    while(fgetc(file) != EOF) {}

    fclose(file);
    stopTimer("\t\tC");
  }

  return 0;
}
