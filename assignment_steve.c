#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>

struct message{
	int childToSendTo;
	char content[128];
};

int parentPart1(int parentToChild1, FILE *inputFile, FILE *parentLogFile);
int parentPart2(int child3ToParent, FILE *parentLogFile);
int child1(int parentToChild1, int child1ToChild2, FILE *child1LogFile);
int child2(int child1ToChild2, char *fifoPipeLocation, FILE *child2LogFile);
int child3(char *fifoPipeLocation, int child3ToParent, FILE *child3LogFile);

int main(int argc, char **argv){
	/*
	Check if there are 5 arguments. 
	If not, skip the main program and just print the usage method.
	*/
	if(argc != 6){
		printf("Usage: ./assignment <input_file> <parent_output_file> <child1_output_file> <child2_output_file> <child3_output_file>\n");
	}
	else{
		/* File pointers to each file */
		FILE *inputFile, *parentLogFile, *child1LogFile, *child2LogFile, *child3LogFile;
		
		/* FIFO pipe file path */
		char *fifoPipeLocation = "/tmp/fifoPipe";
	 
		/* Open the files needed at the beginning */
		if((inputFile = fopen(argv[1], "r")) == NULL){
			printf("Cannot read from %s. Exiting program.\n", argv[1]);
			exit(1);
		}
		else if((parentLogFile = fopen(argv[2], "w")) == NULL){
			printf("Cannot write to %s for Parent. Exiting program.\n", argv[2]);
			exit(1);
		}
		else if((child1LogFile = fopen(argv[3], "w")) == NULL){
			printf("Cannot write to %s for Child 1. Exiting program.\n", argv[3]);
			exit(1);
		}
		else if((child2LogFile = fopen(argv[4], "w")) == NULL){
			printf("Cannot write to %s for Child 2. Exiting program.\n", argv[4]);
			exit(1);
		}
		else if((child3LogFile = fopen(argv[5], "w")) == NULL){
			printf("Cannot write to %s for Child 3. Exiting program.\n", argv[5]);
			exit(1);
		}
		else if(mkfifo(fifoPipeLocation, 0666) == -1){
			printf("Cannot FIFO pipe between Child 2 and Child 3. Exiting program.\n");
			exit(1);
		}
		/* After successful chain of opening needed files, create the pipes */
		else{
			int parentToChild1[2];
			/* Create pipe between Parent and Child 1 */
			if(pipe(parentToChild1) == -1){
				perror("Pipe call error for I/O Pipe between Parent and Child 1.\n");
				exit(1);
			}
			
			/* Main program begins here ----------------------------*/
			
			/* Fork to create Child 1 from Parent */
			int i = fork();
			
			/* Exit program if fork wasn't a success */
			if(i == -1){
				perror("There was a problem with forking the Parent. Exiting program.\n");
				exit(1);
			}
			else if(i == 0){
				/* Child 1 enters here */
				/* Create pipe between Child 1 and Child 2 */ 
				int child1ToChild2[2];
				if(pipe(child1ToChild2) == -1){
					perror("Pipe call error for I/O Pipe between Child 1 and Child 2. Exiting child.\n");
					_exit(1);
				}
				
				/* Fork to create Child 2 from Child 1 */
				int j = fork();
				if(j == -1){
					perror("There was a problem with forking in Child 1. Exiting child.\n");
					_exit(1);
				}
				else if(j == 0){
					/* Child 2 enters here */
					close(child1ToChild2[1]); /* Close write part, Child2 only needs read */
					close(parentToChild1[0]); /* Close read part, Child2 doesn't need this */
					close(parentToChild1[1]); /* Close write part, Child2 doesn't need this */
					
					/* Child 2 is given read part of child1ToChild2 pipe, and child2LogFile */
					child2(child1ToChild2[0], fifoPipeLocation, child2LogFile);
					close(child1ToChild2[0]); /* Close read part, don't need it now */
				}
				else{
					/* Child 1 enters here */
					close(child1ToChild2[0]); /* Close read part, Child 1 only needs write */
					close(parentToChild1[1]); /* Close write part, Child 1 only needs read */
					
					/* 
					Child 2 is given read part of parentToChild1, write part of 
					child1ToChild2 pipe, and child2LogFile 
					*/
					child1(parentToChild1[0], child1ToChild2[1], child1LogFile);
					close(parentToChild1[0]); /* Close read part, don't need it now */
					close(child1ToChild2[1]); /* Close write part, don't need it now */
				}
			}
			else{
				/* Parent enters here */
				/* Create pipe between Child 3 and Parent */
				int child3ToParent[2];
				if(pipe(child3ToParent) == -1){
					perror("Pipe call error for I/O Pipe between Parent and Child 3. Exiting program.\n");
					exit(1);
				}
				
				/* Fork to create Child 3 from Parent */
				int k = fork();
				if(k == -1){
					perror("There was a problem with forking the Parent again. Exiting program.\n");
					exit(1);
				}
				else if(k == 0){
					/* Child 3 enters here */
					
					close(parentToChild1[0]); /* Close read part, Child 3 only needs write */
					close(parentToChild1[1]); /* Close write part, Child 3 only needs write */
					close(child3ToParent[0]); /* Close read part, Child 3 only needs write */
					
					/* Child 3 is given write part of child3ToParent, and child3LogFile */
					child3(fifoPipeLocation, child3ToParent[1], child3LogFile);
					close(child3ToParent[1]); /* Close write part, don't need it now */
				}
				else{
					/* Parent enters here */
					
					close(parentToChild1[0]); /* Close read part, Parent only needs write */
					close(child3ToParent[1]); /* Close write part, Parent only needs read */
					
					/* Parent is first given write part of parentToChild1, inputFile, and parentLogFile */
					parentPart1(parentToChild1[1], inputFile, parentLogFile); 
					close(parentToChild1[1]); /* Close write part, don't need it now */
					
					/* Wait for Child 3 to finish */
					wait(&k);
					
					/* Parent is then given read part of child3ToParent, and parentLogFile */
					parentPart2(child3ToParent[0], parentLogFile); 
					close(child3ToParent[0]); /* Close read part, don't need it now */
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

int parentPart1(int parentToChild1, FILE *inputFile, FILE *parentLogFile){
	/* While we have not reached the end of the file */
	while(!feof(inputFile)){
		struct message line; /* Store each message here */ 
		int keepIt; /* This determines whether we keep the message or not */
		
		/* Read each line from input file */
		fscanf(inputFile, "%d\t%[^\n]\n", &line.childToSendTo, line.content);
		
		/* Since this is the parent, we forward everything to the children */
		keepIt = 0;
		
		/* Write message to Child 1 via pipe */
		write(parentToChild1, &line.childToSendTo, sizeof(line.childToSendTo));
		write(parentToChild1, line.content, sizeof(line.content));
		
		/* Create log entry in Parent log about keeping it or not*/
		fprintf(parentLogFile, "%ld\t%s\t%s\n", time(NULL), line.content, (keepIt)? "KEEP" : "FORWARD");	
	}
	return 0;
}

int parentPart2(int child3ToParent, FILE *parentLogFile){
	struct message line;
	/* While there are still messages in the pipe between Parent and Child 1*/
	while(	(read(child3ToParent, &line.childToSendTo, sizeof(line.childToSendTo)) != 0) 
		&& (read(child3ToParent, line.content, sizeof(line.content)) != 0)
	){
		/* 
			This determines whether we keep the message or not.
			In this case, it's if the childToSendTo is not 1, 2, or 3.
		*/
		int keepIt = (line.childToSendTo > 3) || (line.childToSendTo < 1); 
		
		/* If we keep it... */
		if(keepIt){
			/* ...create log entry in Parent log about keeping it */
			fprintf(parentLogFile, "%ld\t%s\t%s\n", time(NULL), line.content, "KEEP");	
		}
		/* Otherwise... */
		else{
			/* ...and create log entry in Parent log about forwarding it */
			fprintf(parentLogFile, "%ld\t%s\t%s\n", time(NULL), line.content, "FORWARD");	
		}
	}
	return 0;
}

int child1(int parentToChild1, int child1ToChild2, FILE *child1LogFile){
	struct message line;
	/* While there are still messages in the pipe between Parent and Child 1*/
	while(	(read(parentToChild1, &line.childToSendTo, sizeof(line.childToSendTo)) != 0) 
		&& (read(parentToChild1, line.content, sizeof(line.content)) != 0)
	){
		/* 
			This determines whether we keep the message or not.
			In this case, it's if the childToSendTo is 1.
		*/
		int keepIt = (line.childToSendTo == 1); 
		
		/* If we keep it... */
		if(keepIt){
			/* ...create log entry in Child 1 log about keeping it */
			fprintf(child1LogFile, "%ld\t%s\t%s\n", time(NULL), line.content, "KEEP");	
		}
		/* Otherwise... */
		else{
			/* ...send it to Child 2 via pipe... */
			write(child1ToChild2, &line.childToSendTo, sizeof(line.childToSendTo));
			write(child1ToChild2, line.content, sizeof(line.content));
			
			/* ...and create log entry in Child 1 log about forwarding it */
			fprintf(child1LogFile, "%ld\t%s\t%s\n", time(NULL), line.content, "FORWARD");	
		}
	}
	return 0;
}

int child2(int child1ToChild2, char *fifoPipeLocation, FILE *child2LogFile){
	int fifoPipe = open(fifoPipeLocation, O_WRONLY);
	struct message line;
	/* Execute normal behaviour if the open function works as intended */
	if(fifoPipe != -1){
		/* While there are still messages in the pipe between Child 1 and Child 2 */
		while(	(read(child1ToChild2, &line.childToSendTo, sizeof(line.childToSendTo)) != 0) 
			&& (read(child1ToChild2, line.content, sizeof(line.content)) != 0)
		){
			/* 
				This determines whether we keep the message or not.
				In this case, it's if the childToSendTo is 2.
			*/
			int keepIt = (line.childToSendTo == 2); 
			
			/* If we keep it... */
			if(keepIt){
				/* ...create log entry in Child 2 log about keeping it */
				fprintf(child2LogFile, "%ld\t%s\t%s\n", time(NULL), line.content, "KEEP");	
			}
			/* Otherwise... */
			else{
				/* ...send it to Child 3 via FIFO pipe... */
				write(fifoPipe, &line.childToSendTo, sizeof(line.childToSendTo));
				write(fifoPipe, line.content, sizeof(line.content));
				
				/* ...and create log entry in Child 2 log about forwarding it */
				fprintf(child2LogFile, "%ld\t%s\t%s\n", time(NULL), line.content, "FORWARD");	
			}
		}
	}
	/* Execute this if the FIFO pipe encountered an error and returned -1 */
	else{
		perror("Cannot open FIFO pipe for Child 3 for reading. Exiting child.\n");
		_exit(1);
	}
	return 0;
}

int child3(char *fifoPipeLocation, int child3ToParent, FILE *child3LogFile){
	int fifoPipe = open(fifoPipeLocation, O_RDONLY);
	struct message line;
	/* Execute normal behaviour if the open function works as intended */
	if(fifoPipe != -1){
		/* While there are still messages in the FIFO pipe between Child 2 and Child 3 */
		while(	(read(fifoPipe, &line.childToSendTo, sizeof(line.childToSendTo)) != 0) 
			&& (read(fifoPipe, line.content, sizeof(line.content)) != 0)
		){
			/* 
				This determines whether we keep the message or not.
				In this case, it's if the childToSendTo is 3.
			*/
			int keepIt = (line.childToSendTo == 3); 
			
			/* If we keep it... */
			if(keepIt){
				/* ...create log entry in Child 3 log about keeping it */
				fprintf(child3LogFile, "%ld\t%s\t%s\n", time(NULL), line.content, "KEEP");	
			}
			/* Otherwise... */
			else{
				/* ...send it to Parent via pipe... */
				write(child3ToParent, &line.childToSendTo, sizeof(line.childToSendTo));
				write(child3ToParent, line.content, sizeof(line.content));
				
				/* ...and create log entry in Child 3 log about forwarding it */
				fprintf(child3LogFile, "%ld\t%s\t%s\n", time(NULL), line.content, "FORWARD");	
			}
		}
	}
	/* Execute this if the FIFO pipe encountered an error and returned -1 */
	else{
		perror("Cannot open FIFO pipe for Child 3 for reading. Exiting child.\n");
		_exit(1);
	}
	return 0;
}
