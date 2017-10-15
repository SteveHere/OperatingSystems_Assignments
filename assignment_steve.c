#include <stdio.h>

int parent();
int child1();
int child2();
int child3();

int main(int argc, char **argv){
	/*
	Check if there are 2 arguments. 
	If not, ignore the main program and just print usage method.
	*/
	if(argc != 6){
		printf("Usage: ./assignment <input_file> <parent_output_file> <child1_output_file> <child2_output_file> <child3_output_file>\n");
	}
	else{
		int i = fork();
		if(i == -1){
			perror("There was a problem with forking the parent. Exiting program.");
			exit(1);
		}
		else if(i == 0){
			/* Child 1 enters here */
			int j = fork();
			if(j == -1){
				perror("There was a problem with forking in child1. Exiting program.");
				exit(1);
			}
			else if(j == 0){
				/* Child 2 enters here */
				child2();
			}
			else{
				/* Child 1 enters here */
				child1();
			}
		}
		else{
			/* Parent enters here */
			int k = fork();
			if(k == -1){
				perror("There was a problem with forking the parent again. Exiting program.");
				exit(1);
			}
			else if(k == 0){
				/* Child 3 enters here */
				child3();
			}
			else{
				/* Parent enters here */
				parent();
			}
		}
	}
	return 0;
}

int parent(){
	
	return 0;
}

int child1(){
	
	return 0;
}

int child2(){
	
	return 0;
}

int child3(){
	
	return 0;
}