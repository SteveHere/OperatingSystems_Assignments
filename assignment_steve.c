#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct message{
	int childToSendTo;
	char message[128];
};

int parentPart1(int parentToChild1, FILE *inputFile);
int parentPart2(int child3ToParent, FILE *parentLogFile);
int child1(int parentToChild1, int child1ToChild2, FILE *child1LogFile);
int child2(int child1ToChild2, FILE *child2LogFile);
int child3(int child3ToParent, FILE *child3LogFile);

int main(int argc, char **argv){
	/*
	Check if there are 5 arguments. 
	If not, skip the main program and just print the usage method.
	*/
	if(argc != 6){
		printf("Usage: ./assignment <input_file> <parent_output_file> <child1_output_file> <child2_output_file> <child3_output_file>\n");
	}
	else{
		FILE *inputFile, *parentLogFile, *child1LogFile, *child2LogFile, *child3LogFile;
		
		/* Open the files needed at the beginning */
		if((inputFile = fopen(argv[1], "r")) == NULL){
			printf("Cannot read from %s. Exiting program.", argv[1]);
			exit(1);
		}
		else if((parentLogFile = fopen(argv[2], "w")) == NULL){
			printf("Cannot write to %s for Parent. Exiting program.", argv[2]);
			exit(1);
		}
		else if((child1LogFile = fopen(argv[3], "w")) == NULL){
			printf("Cannot write to %s for Child 1. Exiting program.", argv[3]);
			exit(1);
		}
		else if((child2LogFile = fopen(argv[4], "w")) == NULL){
			printf("Cannot write to %s for Child 2. Exiting program.", argv[4]);
			exit(1);
		}
		else if((child3LogFile = fopen(argv[5], "w")) == NULL){
			printf("Cannot write to %s for Child 3. Exiting program.", argv[5]);
			exit(1);
		}
		/* After successful chain of opening needed files, create the pipes */
		else{
			int parentToChild1[2];
			/* Create pipe between Parent and Child1 */
			if(pipe(parentToChild1) == -1){
				perror("Pipe call error for I/O Pipe between Parent and Child 1.\n");
				exit(1);
			}
			
			/* Main program begins here ----------------------------*/
			
			/* Fork to create Child1 from Parent */
			int i = fork();
			
			/* Exit program if fork wasn't a success */
			if(i == -1){
				perror("There was a problem with forking the Parent. Exiting program.\n");
				exit(1);
			}
			else if(i == 0){
				/* Child 1 enters here */
				int child1ToChild2[2];
				if(pipe(child1ToChild2) == -1){
					perror("Pipe call error for I/O Pipe between Child 1 and Child 2. Exiting program.\n");
					_exit(1);
				}
				
				/* Fork to create Child2 from Child1 */
				int j = fork();
				if(j == -1){
					perror("There was a problem with forking in Child 1. Exiting program.\n");
					_exit(1);
				}
				else if(j == 0){
					/* Child 2 enters here */
					close(child1ToChild2[1]);
					close(parentToChild1[0]);
					close(parentToChild1[1]);
					
					child2(child1ToChild2[0], child2LogFile);
					close(child1ToChild2[0]);
				}
				else{
					/* Child 1 enters here */
					close(child1ToChild2[0]);
					close(parentToChild1[1]);
					
					child1(parentToChild1[0], child1ToChild2[1], child1LogFile);
					close(parentToChild1[0]);
					close(child1ToChild2[1]);
				}
			}
			else{
				/* Parent enters here */
				int child3ToParent[2];
				if(pipe(child3ToParent) == -1){
					perror("Pipe call error for I/O Pipe between Parent and Child 3. Exiting program.\n");
					exit(1);
				}
				
				/* Fork to create Child3 from Parent */
				int k = fork();
				if(k == -1){
					perror("There was a problem with forking the Parent again. Exiting program.\n");
					exit(1);
				}
				else if(k == 0){
					/* Child 3 enters here */
					
					close(parentToChild1[0]);
					close(parentToChild1[1]);
					close(child3ToParent[0]);
					
					child3(child3ToParent[1], child3LogFile);
				}
				else{
					/* Parent enters here */
					
					close(parentToChild1[0]);
					close(child3ToParent[1]);
					
					parentPart1(parentToChild1[1], inputFile);
					close(parentToChild1[1]);
					
					/* Wait for Child 3 to finish */
					wait(&k);
					
					parentPart2(child3ToParent[0], parentLogFile);
					close(child3ToParent[0]);
				}
			}
			/* Main program ends here -------------------------------------------*/
		}
		
		fclose(inputFile);
		fclose(parentLogFile);
		fclose(child1LogFile);
		fclose(child2LogFile);
		fclose(child3LogFile);
	}
	return 0;
}

int parentPart1(int parentToChild1, FILE *inputFile){
	printf("Parent part 1\n");
	sleep(1);
	return 0;
}

int parentPart2(int child3ToParent, FILE *parentLogFile){
	printf("Parent part 2\n");
	sleep(1);
	return 0;
}

int child1(int parentToChild1, int child1ToChild2, FILE *child1LogFile){
	printf("Child 1\n");
	sleep(1);
	return 0;
}

int child2(int child1ToChild2, FILE *child2LogFile){
	printf("Child 2\n");
	sleep(1);
	return 0;
}

int child3(int child3ToParent, FILE *child3LogFile){
	printf("Child 3\n");
	sleep(1);
	return 0;
}
