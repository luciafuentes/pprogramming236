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
#include <unistd.h>
#include <mpi.h>
#include <time.h>
#include "functions.h"

#define MAX_CHAR 1024 //default maximum amount of characters


/**Function that receives an input string and the transformation_function
 * file and returns the output according to the file. In case of error
 * returns the string "E"
 * */
char* transform(char* input, FILE * funct){
    char line[MAX_CHAR];
    char *function_input = NULL, *function_output = NULL;
    rewind(funct);
    while(fgets(line, sizeof line, funct)!= NULL){ 
      function_input = strtok(line, " ");
      function_output = strtok(NULL, " ");
      if (strcmp(function_input, input) == 0){
          return function_output;
      }
    }
    return "E";
}

/**
 * Returns the module of the given numbers. Contrary to the
 * operator %, returns the module of negative numbers too
 * */
int module(int num1, int num2){
    if (num1>=0) return num1%num2;
    else return (num2+num1)%num2;
}


int main(int argc, char *argv[]) {
    char** matrix = NULL;
    char** result_matrix = NULL;
    char** scattered_matrix = NULL;
    char** scattered_result = NULL;
    char* first_vector = NULL, *last_vector = NULL;
    FILE * initial_configuration = NULL;
    FILE * transformation_function = NULL;
    int i, j, num_iterations=-1, tam = -1, total_sum = 0;
    int current_id, num_procs, rest;
    char size[MAX_CHAR];
    char char_act;
    char partial_input[9];
    int *sendcounts = NULL, *displacements = NULL;


    //Argument check
    num_iterations = atoi(argv[3]);
    if (num_iterations<1){
        fprintf(stderr, "Non-valid number of iterations\n");
        return EXIT_FAILURE;
    }
    
    /**USED FOR EXPERIMENTS
    int k = atoi(argv[1]);
    generate_nxn(k);
    FILE * results = fopen("results2dparallel.txt", "a");
    clock_t start = clock();
    */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &current_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    initial_configuration = fopen (argv[1], "r");
    transformation_function = fopen(argv[2], "r");
    if (!initial_configuration || !transformation_function){
        fprintf(stderr, "Unable to read specified files\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }


    fgets(size, MAX_CHAR, initial_configuration);
    tam = atoi(size);
    if (tam<1){
        fprintf(stderr, "Matrix size not valid\n");
        return EXIT_FAILURE;
    }
        

    matrix = (char **) calloc (tam, sizeof(char*));

    //fill in sendcounts and displacements array
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

    
    //---------------------boss process: reads input matrix from file
    if(current_id == 0){
        matrix = (char **) calloc (tam, sizeof(char*));
        for(i=0; i<tam; i++){
            matrix[i] = (char *) calloc (tam, sizeof(char));
            for(j=0; j<tam; j++){
                do{
                    char_act = fgetc(initial_configuration);
                } while (char_act != '0' && char_act != '1' && char_act != EOF);
                
                if(char_act != EOF){
                    matrix[i][j] = char_act;
                }

                if (matrix[i][j] != '0' && matrix[i][j] != '1'){
                    fprintf(stderr, "Matrix contains non-boolean value\n");
                    MPI_Finalize();
                    return EXIT_FAILURE;
                }
            }
        }
    }
    //----------------------------------------------------------------

    scattered_matrix = (char**) calloc (sizeof(char*), tam); //each process has its own partial matrix
    scattered_result = (char**) calloc (sizeof(char*), tam); //partial output
    
    first_vector = (char*)calloc(sizeof(char), tam); //vector containing the last elements of the previous column in the matrix
    last_vector = (char*)calloc(sizeof(char), tam); //vector containing the first elements of the next column in the matrix

    result_matrix = (char**) calloc (sizeof(char*), tam);

    for(i=0; i<tam; i++){
        scattered_result[i] = (char*) calloc (sizeof(char), sendcounts[current_id]);
        scattered_matrix[i] = (char*) calloc (sizeof(char), sendcounts[current_id]);
        result_matrix[i] = (char*) calloc (sizeof(char), sendcounts[current_id]);
    }

    //print initial input (for debugging purposes)
    if (current_id == 0){
        for(i=0; i<tam; i++){
            if(i==0) printf("MOTHER MATRIX:\n");
            printf("%s\n", matrix[i]);
        }
    }

    while(num_iterations>0){
        for(i=0; i<tam; i++){
            MPI_Scatterv(matrix[i], sendcounts, displacements, MPI_CHAR, scattered_matrix[i], 
                        sendcounts[current_id], MPI_CHAR, 0, MPI_COMM_WORLD);  

            if (current_id == 0){ //case first process
                MPI_Send(&scattered_matrix[i][sendcounts[current_id]-1], 1, MPI_CHAR, current_id+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&first_vector[i], 1, MPI_CHAR, num_procs-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Send(&scattered_matrix[i][0], 1, MPI_CHAR, num_procs-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&last_vector[i], 1, MPI_CHAR, current_id+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            } else if (current_id == num_procs-1){ //case last process
                MPI_Recv(&first_vector[i], 1, MPI_CHAR, current_id-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&scattered_matrix[i][sendcounts[current_id]-1], 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

                MPI_Recv(&last_vector[i], 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&scattered_matrix[i][0], 1, MPI_CHAR, current_id-1, 0, MPI_COMM_WORLD);
            } else { //case default process
                MPI_Recv(&first_vector[i], 1, MPI_CHAR, current_id-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&scattered_matrix[i][sendcounts[current_id]-1], 1, MPI_CHAR, current_id+1, 0, MPI_COMM_WORLD);

                MPI_Recv(&last_vector[i], 1, MPI_CHAR, current_id+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&scattered_matrix[i][0], 1, MPI_CHAR, current_id-1, 0, MPI_COMM_WORLD);
            }
        }

        partial_input[9] = '\0';
        for(i=0; i<tam; i++){
            for(j=0; j<sendcounts[current_id]; j++){
                if(j==0){ //case first column
                    partial_input[0] = first_vector[module(i-1, tam)];
                    partial_input[3] = first_vector[i];
                    partial_input[6] = first_vector[module(i+1, tam)];
                } else{ 
                    partial_input[0] = scattered_matrix[module(i-1, tam)][module(j-1, tam)];
                    partial_input[3] = scattered_matrix[module(i, tam)][module(j-1,tam)];
                    partial_input[6] = scattered_matrix[module(i+1, tam)][module(j-1, tam)];
                }

                if(j==sendcounts[current_id]-1){ //case last column
                    partial_input[2] = last_vector[module(i-1, tam)];
                    partial_input[5] = last_vector[i];
                    partial_input[8] = last_vector[module(i+1, tam)];
                } else {
                    partial_input[2] = scattered_matrix[module(i-1, tam)][module(j+1, tam)];
                    partial_input[5] = scattered_matrix[module(i, tam)][module(j+1, tam)];
                    partial_input[8] = scattered_matrix[module(i+1, tam)][module(j+1, tam)];
                }

                partial_input[1] = scattered_matrix[module(i-1, tam)][module(j, tam)];
                partial_input[4] = scattered_matrix[module(i, tam)][module(j, tam)];
                partial_input[7] = scattered_matrix[module(i+1, tam)][module(j, tam)];

                scattered_result[i][j] = *transform(partial_input, transformation_function);
            }
        }


        for(i=0; i<tam; i++)
            MPI_Gatherv(scattered_result[i], sendcounts[current_id],  MPI_CHAR,  result_matrix[i],  sendcounts,
                            displacements,  MPI_CHAR,  0,  MPI_COMM_WORLD);    
        
        //print result matrix (for debugging purposes)
        if(current_id == 0){
            printf("---> IT %d\nRESULT MATRIX:\n", num_iterations);
            for(i=0; i<tam; i++){
                printf("%s\n", result_matrix[i]);
            }
        }

        //output becomes new input for next iteration
        if(current_id == 0){
            for(i=0; i<tam; i++){
                for(j=0; j<tam; j++){
                    matrix[i][j] = result_matrix[i][j];
                }
            }
        }
        num_iterations--;
    }

    /*
    double timedif = (double)(clock() - start)/CLOCKS_PER_SEC;
    if(current_id == 0)
        fprintf(results, "%d %d %f\n", tam*tam, num_procs, timedif);
    */
   

    for(i=0; i<tam; i++){
        free(matrix[i]);
        free(result_matrix[i]);
        free(scattered_matrix[i]);
        free(scattered_result[i]);
    }
    free(matrix);
    free(result_matrix);
    free(scattered_matrix);
    free(scattered_result);
    free(sendcounts);
    free(displacements);
    free(first_vector);
    free(last_vector);
    fclose(transformation_function);
    fclose(initial_configuration);
    MPI_Finalize();  

    return EXIT_SUCCESS;
}

