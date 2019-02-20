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

#define MAX_CHAR 1024

/**
 * Function used to free all the memory allocations (if any)
 * and close the files used (if any)
 * */
void program_destroy(int sz, char** mat1, char** mat2, FILE* f1, FILE* f2){
    int i;
    if(mat1){ for(i=0; i<sz; i++) free(mat1[i]); free(mat1);}
    if(mat2){ for(i=0; i<sz; i++) free(mat2[i]); free(mat2);}
    if(f1) fclose(f1);
    if(f2) fclose(f2);
}

/**Function that receives an input string and the transformation_function
 * file and returns the output according to the file. In case of error
 * returns the string "E"
 * */
char transform(char* input, FILE * funct){
    char line[MAX_CHAR];
    char function_input[MAX_CHAR] = "", function_output[MAX_CHAR] = ""; 
    rewind(funct);
    while(fgets(line, sizeof line, funct)!= NULL){ 
      strcpy(function_input, strtok(line, " "));
      strcpy(function_output, strtok(NULL, " "));
      if (strcmp(function_input, input) == 0){
          return *function_output;
      }
    }
    return 'E';
}

/**
 * Returns the module of the given numbers. Contrary to the
 * operator %, returns the module of negative numbers too
 * */
int module(int num1, int num2){
    if (num1>=0) return num1%num2;
    else return num2+num1;
}


int main(int argc, char const *argv[]) {
    char** matrix = NULL;
    char** result_matrix = NULL;
    FILE * initial_configuration = NULL;
    FILE * transformation_function = NULL;
    int i, j, num_iterations=-1, tam = -1;
    char size[MAX_CHAR];
    char char_act;
    char partial_input[9];
    char partial_output; 
    
	//Argument check
    if (argc != 4){
        fprintf(stderr, "Incorrect number of arguments: try ./Cellular2DSequential "
                        "initial_configuration transformation_function num_iterations\n");
        return EXIT_FAILURE;
    }

    num_iterations = atoi(argv[3]);
    if (num_iterations<0){
        fprintf(stderr, "The number of iterations must be > 0\n");
        return EXIT_FAILURE;
    }
    
    initial_configuration = fopen (argv[1], "r");
    transformation_function = fopen(argv[2], "r");

    if (!initial_configuration || !transformation_function){
        fprintf(stderr, "Files do not exist or could not be opened\n");
        program_destroy(tam, matrix, result_matrix, transformation_function, initial_configuration);
        return EXIT_FAILURE;
    }

    fgets(size, MAX_CHAR, initial_configuration);
    tam = atoi(size);
    if (tam<1){
        fprintf(stderr, "Not valid size of the matrix\n");
        program_destroy(tam, matrix, result_matrix, transformation_function, initial_configuration);
        return EXIT_FAILURE;
    }

    //memory allocation for input matrix
    matrix = (char **) calloc (sizeof(char*), tam);
    for (i=0;i<tam;i++)
        matrix[i] = (char *) calloc (tam, sizeof(char));

    
    //read input matrix from file
    for(i=0; i<tam; i++){
        for(j=0; j<tam; j++){
            do{
                char_act = fgetc(initial_configuration);
            } while (char_act != '0' && char_act != '1' && char_act != EOF);
            
            if(char_act != EOF){
                matrix[i][j] = char_act;
            }

            if (matrix[i][j] != '0' && matrix[i][j] != '1'){
                fprintf(stderr, "Initial configuration contains non-boolean value\n");
                program_destroy(tam, matrix, result_matrix, transformation_function, initial_configuration);
                return EXIT_FAILURE;
            }
        }
    }
    
    //memory allocation for output matrix
    result_matrix = (char **) calloc (tam, sizeof(char*));
    for (i=0;i<tam;i++)
        result_matrix[i] = (char *) calloc (tam, sizeof(char));
    
    /**
     * insert in each cell of result_matrix the result of the function of each cell and its
     * 8 surrounding cells
     * */
    partial_input[9] = '\0';
    while(num_iterations>0){
        for(i=0; i<tam; i++){        
            for(j=0; j<tam; j++){
                partial_input[0] = matrix[module(i-1, tam)][module(j-1, tam)];
                partial_input[1] = matrix[module(i-1, tam)][module(j, tam)];
                partial_input[2] = matrix[module(i-1, tam)][module(j+1, tam)];

                partial_input[3] = matrix[module(i, tam)][module(j-1, tam)];
                partial_input[4] = matrix[module(i, tam)][module(j, tam)];
                partial_input[5] = matrix[module(i, tam)][module(j+1, tam)];

                partial_input[6] = matrix[module(i+1, tam)][module(j-1, tam)];
                partial_input[7] = matrix[module(i+1, tam)][module(j, tam)];
                partial_input[8] = matrix[module(i+1, tam)][module(j+1, tam)];

                result_matrix[i][j] = transform(partial_input, transformation_function);
            }
        }

        //print input matrix (for debugging purposes)
        printf("----->IT %d\nINPUT MATRIX:\n", num_iterations);
        for (i=0; i<tam; i++){
            for(j=0; j<tam; j++) printf("%c", matrix[i][j]);
	printf("\n");
        }
	
        
        //print output matrix (for debugging purposes)
        printf("OUTPUT MATRIX:\n");
        for (i=0; i<tam; i++){
            for(j=0; j<tam; j++) printf("%c", result_matrix[i][j]);
	printf("\n");
        }

        //output matrix becomes the new input matrix for next iteration
        for(i=0; i<tam; i++){
            for(j=0; j<tam; j++){
                matrix[i][j] = result_matrix[i][j];
            }
        }
        num_iterations--;
    }

    //free resouces
    program_destroy(tam, matrix, result_matrix, transformation_function, initial_configuration);

    return EXIT_SUCCESS;
}


