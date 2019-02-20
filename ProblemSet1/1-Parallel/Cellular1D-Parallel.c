/**
 * Copyright 2019 Lucia Fuentes Villodres
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal 
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 *  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 *  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
 *  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 *  OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include "functions.h"

#define MAX_CHAR 1024

/**
 * Function used to free all the memory allocations (if any),
 * close the files used (if any) and call finalize MPI
 * for termination of the program
 * */
void program_destroy(FILE* f1, FILE* f2, char* array1, 
                char* array2, int* count1, int* count2){
    if (f1) fclose(f1);
    if (f2) fclose(f2);
    if (array1) free(array1);
    if (array2) free(array2);
    if (count1) free(count1);
    if (count2) free(count2);
    MPI_Finalize();
}


/**Function that receives an input string and the transformation function
 * file and returns the output according to the file. In case of error
 * returns the string "E"
 * */
char* transform(char* in, FILE * funct){
    char line[MAX_CHAR];
    char *function_input = NULL, *function_output = NULL;
    rewind(funct);
    while(fgets(line, sizeof line, funct)!= NULL){ 
      function_input = strtok(line, " ");
      function_output = strtok(NULL, " ");
      if (strcmp(function_input, in) == 0){
          return function_output;
      }
    }
    return "E";
}


int main(int argc, char *argv[]) {
    FILE * initial_configuration = NULL;
    FILE * transformation_function = NULL;
    
    char size[MAX_CHAR];
    char *input = NULL, *output = NULL;
    char *partial_input = NULL, *partial_output = NULL;
    char current_input[MAX_CHAR];
    char char_act, first_element, last_element;

    int current_id, num_procs;
    int tam = -1;
    int i;
    int number_iterations = -1, rest = 0, total_sum = 0;

    int *sendcounts = NULL, *displacements = NULL;

    //FILE * results = fopen("results.txt", "a");
	//clock_t start = clock();

    //Argument check
    if (argc != 4){
        fprintf(stderr, "Invalid number of arguments. Try ./Cellular1D-Parallel file1 file2 num_iterations");
        return EXIT_FAILURE;
    }

    number_iterations = atoi(argv[3]);
    if (number_iterations<1){
        fprintf(stderr, "Number of iterations has to be > 0\n");
        return EXIT_FAILURE;
    }

    //initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &current_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);


    //initial_configuration = fopen (argv[1], "r");
    initial_configuration = fopen(argv[1], "r");
    transformation_function = fopen(argv[2], "r");
    if (!initial_configuration || !transformation_function){
        fprintf(stderr, "Files do not exist or could not open them\n");
        program_destroy(initial_configuration, transformation_function, input, 
                    partial_input, sendcounts, displacements);
        return EXIT_FAILURE;
    }

    //get size of the matrix
    fgets(size, MAX_CHAR, initial_configuration);
    tam = atoi(size);
    if (tam<1){
        fprintf(stderr, "Size of the matrix should be > 0. Check initial configuration file\n");
        program_destroy(initial_configuration, transformation_function, 
                       input, partial_input, sendcounts, displacements);
        return EXIT_FAILURE;
    }

    //fill in sendcounts and displacements arrays (used in scatterv)
    sendcounts = (int*) calloc (sizeof(int), num_procs);
    displacements = (int*) calloc (sizeof(int), num_procs);
    rest = tam%num_procs;
    for (i=0; i<num_procs; i++){
        sendcounts[i] = tam/(num_procs);
        if (rest != 0){
            sendcounts[i]++;
            rest--;
        }
        displacements[i] = total_sum;
        total_sum += sendcounts[i];
    }
    
    //------------------------boss process: reads input vector
    if (current_id == 0){
        input = (char*)calloc(tam, sizeof(char));

        for (i=0; i<tam; i++){
            char_act = fgetc(initial_configuration);
            if (char_act != '0' && char_act != '1'){
                fprintf(stderr, "Initial configuration contains non-boolean value");
                program_destroy(initial_configuration, transformation_function, input, partial_input, sendcounts, displacements);
                return EXIT_FAILURE;
            }
            input[i] = char_act;
            
            //print for visualization
            if(input[i] == '1') printf("#");
            else printf(" ");
            if (i==tam-1) printf ("\n");
        }
        output = (char*)malloc(tam*sizeof(char));
    }
    //---------------------------------------------------------
    
    /**
     * each process has its local partial input (result of scatter) and creates the partial output
     * for the posterior gather
     * */
    partial_input = (char*) calloc (sizeof(char), sendcounts[current_id]);
    partial_output = (char*) calloc (sizeof(char),sendcounts[current_id]);

    while(number_iterations > 0){
        MPI_Scatterv(input, sendcounts, displacements, MPI_CHAR, partial_input, sendcounts[current_id], 
                                    MPI_CHAR, 0, MPI_COMM_WORLD);

        if(num_procs>1){
            if (current_id == 0){ //case first process
                MPI_Send(&partial_input[sendcounts[current_id]-1], 1, MPI_CHAR, current_id+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&first_element, 1, MPI_CHAR, num_procs-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
                MPI_Send(&partial_input[0], 1, MPI_CHAR, num_procs-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&last_element, 1, MPI_CHAR, current_id+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            } else if (current_id == num_procs-1){ //case last process
                MPI_Recv(&first_element, 1, MPI_CHAR, current_id-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&partial_input[sendcounts[current_id]-1], 1,  MPI_CHAR, 0, 0, MPI_COMM_WORLD);
                MPI_Recv(&last_element, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&partial_input[0], 1,  MPI_CHAR, current_id-1, 0, MPI_COMM_WORLD);

            } else {//case default process
                MPI_Recv(&first_element, 1, MPI_CHAR, current_id-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&partial_input[sendcounts[current_id]-1], 1,  MPI_CHAR, current_id+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&last_element, 1, MPI_CHAR, current_id+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&partial_input[0], 1,  MPI_CHAR, current_id-1, 0, MPI_COMM_WORLD);
            }
        }
        
        current_input[3] = '\0';
        for(i=0; i<sendcounts[current_id]; i++){
            if (sendcounts[current_id] == 1){
                current_input[0] = first_element;
                current_input[1] = partial_input[i];
                current_input[2] = last_element;
            }else if(i==0){
                current_input[0] = first_element;
                current_input[1] = partial_input[i];
                current_input[2] = partial_input[i+1];
            } else if (i == sendcounts[current_id]-1){
                current_input[0] = partial_input[i-1];
                current_input[1] = partial_input[i];
                current_input[2] = last_element;
            } else {
                current_input[0] = partial_input[i-1];
                current_input[1] = partial_input[i];
                current_input[2] = partial_input[i+1];
            }
            partial_output[i] = *transform(current_input, transformation_function);
        }

        MPI_Gatherv(partial_output, sendcounts[current_id],  MPI_CHAR,  output,  
                    sendcounts,  displacements,  MPI_CHAR,  0,  MPI_COMM_WORLD);
        
        if (current_id == 0){
            //print output for visualization purposes
            //the output is the new input for the next iteration
            for(i=0; i<tam; i++){
                if(output[i] == '1') printf("#");
                else printf(" ");
                if (i==tam-1) printf ("\n");
                input[i] = output[i];
            } 
        }
        number_iterations--;
    }
	
	/*FOR EXPERIMENTS
    clock_t end = clock();
    double timedif = (double)(end-start)/CLOCKS_PER_SEC;
	if(current_id == 0)
		fprintf(results, "%d %d %f\n", tam, num_procs, timedif);
    */

    //free resources
    program_destroy(initial_configuration, transformation_function, input, 
                   partial_input, sendcounts, displacements);
    //fclose(results);
    return EXIT_SUCCESS;    
}
