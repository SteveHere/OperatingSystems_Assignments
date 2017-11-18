/*
	ITS60503 ( Operating Systems ) -- Assignment 2
	Students involved:
	1.	Kwan Juen Wen - 0322448
	2.	Mohammad Ramzan Ashraf - 0323724
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFFERSIZE 1024
#define TRUE 1
#define FALSE 0

void *readerFunction();
void *writerFunction();

struct {
	char buffer[16][BUFFERSIZE];
	int bufferSize[16];
	int buffersFilled;
	int endOfFile; /* Flag used to determine if at the end of a file. */
	int readerPointer;
	int writerPointer;
} buffer;

struct {
	pthread_mutex_t readerMutex;
	pthread_mutex_t writerMutex;
	pthread_cond_t waitForReaders;
	pthread_cond_t waitForWriters;
} mutexes;

int main(int argc, char **argv){
	/*
	Check if there are 3 arguments. 
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
		int inputFile, outputFile;
		/* Number of reader and writer threads */
		int reader_writer_threads = 0;
	 
		/* Open the files needed at the beginning */
		if((inputFile = open(argv[1], O_RDONLY)) == -1){
			printf("Cannot read from %s. Exiting program.\n", argv[1]);
			exit(1);
		}
		else if((outputFile = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1){
			printf("Cannot write to %s. Exiting program.\n", argv[2]);
			close(inputFile);
			exit(2);
		}
		/* After successful chain of opening needed files, parse the integer for reader_writer_threads */
		else if(( reader_writer_threads = atoi(argv[3]) ) < 1){
			printf("Parse error encountered with reader_writer_threads.\n");
			printf("reader_writer_threads must always be more than 0.\n");
			close(inputFile);
			close(outputFile);
			exit(3);
		}
		else{
			pthread_t readerThread[reader_writer_threads];
			pthread_t writerThread[reader_writer_threads];
			int i;
			
			buffer.buffersFilled = 0;
			buffer.endOfFile = FALSE;
			buffer.readerPointer = 0;
			buffer.writerPointer = 0;
			
			/* Main program starts here -----------------------------------------*/
			/* Create the reader and writer threads */
			for(i = 0; i < reader_writer_threads; i++) {
				pthread_create(&readerThread[i], NULL, &readerFunction, (void *)(size_t)inputFile);
				pthread_create(&writerThread[i], NULL, &writerFunction, (void *)(size_t)outputFile);
			}
			
			/* Join all the reader threads and the writer threads. */
			for(i = 0; i < reader_writer_threads; i++) {
				pthread_join(readerThread[i], NULL);
				pthread_join(writerThread[i], NULL);
			}
			
			printf("The input file has been fully duplicated.\n");
			/* Main program ends here -------------------------------------------*/
		}
		
		/* Closing all of the file pointers */
		close(inputFile);
		close(outputFile);
	}
	return 0;
}

/* This function is for the reader threads */
void *readerFunction(void *inputFile){
	while(buffer.endOfFile == FALSE){		
		/* Set the lock */
		pthread_mutex_lock(&mutexes.readerMutex);
		
		/* If all the buffers are filled, then wait until 1 one the buffers has been used. */
		while(buffer.buffersFilled == 16){
			pthread_cond_wait(&mutexes.waitForWriters, &mutexes.readerMutex);
		}
		
		if(buffer.endOfFile == FALSE){
			/* Read from the file and get back its return value. */
			buffer.bufferSize[buffer.readerPointer] = read((int)(size_t)inputFile, buffer.buffer[buffer.readerPointer], BUFFERSIZE);
			
			/* 
			If the return value is 0, that means we hit the end. 
			Set the endOfFile flag to TRUE, and quit this thread.
			*/
			if(buffer.bufferSize[buffer.readerPointer] == 0){
				/* Set the flag for endOfFile to TRUE */
				buffer.endOfFile = TRUE;
				
				pthread_cond_signal(&mutexes.waitForReaders); 
				/* Release the lock */
				pthread_mutex_unlock(&mutexes.readerMutex); 
				pthread_exit(0);
				break;
			}
			/* Otherwise, we have 1 more buffer to deal with that isn't the buffer's size. In this case, just set the flag to TRUE. */
			else if(buffer.bufferSize[buffer.readerPointer] < BUFFERSIZE){
				/* Set the flag for endOfFile to TRUE */
				buffer.endOfFile = TRUE;
			}
			
			/* Point to the next buffer to be written to, and increment the number of filled buffers by 1. */
			buffer.readerPointer = (buffer.readerPointer + 1) % 16;
			buffer.buffersFilled++;
		}
		
		/* Signal to the other mutex */
		pthread_cond_signal(&mutexes.waitForReaders);
		
		/* Release the lock */
		pthread_mutex_unlock(&mutexes.readerMutex);
	}
	pthread_exit(0);
}

/* This function is for the writer threads */
void *writerFunction(void *outputFile){
	while(!(buffer.buffersFilled == 0 && buffer.endOfFile == TRUE)){		
		/* Set the lock */
		pthread_mutex_lock(&mutexes.writerMutex);
		
		/* If there's nothing in the buffer, wait until there's something to write about. */
		while(buffer.buffersFilled == 0 && buffer.endOfFile == FALSE){
			pthread_cond_wait(&mutexes.waitForReaders, &mutexes.writerMutex);
		}
				
		/* Another check just to make sure that there's something to write about. */
		if(buffer.buffersFilled > 0){
			/* Write to the output file */
			write((int)(size_t)outputFile, buffer.buffer[buffer.writerPointer], buffer.bufferSize[buffer.writerPointer]);
			
			/* Point to the next buffer to be read from, and decrement the number of filled buffers by 1. */
			buffer.writerPointer = (buffer.writerPointer + 1) % 16;
			buffer.buffersFilled--;
		}
		
		/* Signal to the other mutex */
		pthread_cond_signal(&mutexes.waitForWriters);
		
		/* Release the lock */
		pthread_mutex_unlock(&mutexes.writerMutex);
	}
	pthread_exit(0);
}