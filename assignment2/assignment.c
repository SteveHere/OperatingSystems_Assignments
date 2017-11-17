/*
	ITS60503 ( Operating Systems ) -- Assignment 2
	Students involved:
	1.	Kwan Juen Wen - 0322448
	2.	Mohammad Ramzan Ashraf - 0323724
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char **argv){
	/*
	Check if there are 5 arguments. 
	If not, skip the main program and just print the usage method.
	*/
	if(argc != 4){
		printf("Usage: ./assignment <input_file> <output_file> <reader_writer_threads>\n");
		printf("reader_writer_threads: number of reader threads & number of writer threads\n");
		printf("Example 1: 2 = 2 reader threads & 2 writer threads\n");
		printf("Example 2: 4 = 4 reader threads & 4 writer threads\n");
	}
	else{
		/* File pointers to each file */
		FILE *inputFile, *outputFile;
		/* Number of reader adn writer threads */
		int reader_writer_threads = 0;
	 
		/* Open the files needed at the beginning */
		if((inputFile = fopen(argv[1], "r")) == NULL){
			printf("Cannot read from %s. Exiting program.\n", argv[1]);
			exit(1);
		}
		else if((outputFile = fopen(argv[2], "w")) == NULL){
			printf("Cannot write to %s. Exiting program.\n", argv[2]);
			fclose(inputFile);
			exit(2);
		}
		/* After successful chain of opening needed files, parse the integer for reader_writer_threads */
		else if(( reader_writer_threads = atoi(argv[3) ) < 1){
			printf("Parse error encountered with reader_writer_threads.\n");
			printf("reader_writer_threads must always be more than 0.\n");
			fclose(inputFile);
			fclose(outputFile);
			exit(3);
		}
		else{
			/* TODO: setup */
			/* Main program starts here -----------------------------------------*/
			/* TODO: main logic */
			/* Main program ends here -------------------------------------------*/
		}
		
		/* Closing all of the file pointers */
		fclose(inputFile);
		fclose(outputFile);
	}
	return 0;
}