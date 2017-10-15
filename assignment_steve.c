#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct message{
	int childToSendTo;
	char message[128];
}

int parentPart1();
int parentPart2();
int child1();
int child2();
int child3();

int main(int argc, char **argv){
	/*
	Check if there are 5 arguments. 
	If not, skip the main program and just print the usage method.
	*/
	if(argc != 6){
		printf("Usage: ./assignment <input_file> <parent_output_file> <child1_output_file> <child2_output_file> <child3_output_file>\n");
	}
	else{
		int parentToChild1[2];
		/* Create pipe between Parent and Child1 */
		if(pipe(parentToChild1) == -1){
			perror("Pipe call error for I/O Pipe between Parent and Child 1.\n");
			exit(1);
		}
		
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
			if(pipe(parentToChild2) == -1){
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
				child2(child1ToChild2);
			}
			else{
				/* Child 1 enters here */
				child1(child1ToChild2);
			}
		}
		else{
			/* Parent enters here */
			int child3ToParent[2];
			if(pipe(parentToChild1) == -1){
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
				child3(child3ToParent);
			}
			else{
				/* Parent enters here */
				parentPart1(parentToChild1);
				/* Wait for Child 3 to finish */
				wait(&k);
				parentPart2(child3ToParent);
			}
		}
	}
	return 0;
}

int parentPart1(int pipes[2]){
	printf("Parent part 1\n");
	sleep(1);
	return 0;
}

int parentPart2(int pipes[2]){
	printf("Parent part 2\n");
	sleep(1);
	return 0;
}

int child1(int pipes[2]){
	printf("Child 1\n");
	sleep(1);
	return 0;
}

int child2(int pipes[2]){
	printf("Child 2\n");
	sleep(1);
	return 0;
}

int child3(int pipes[2]){
	printf("Child 3\n");
	sleep(1);
	return 0;
}