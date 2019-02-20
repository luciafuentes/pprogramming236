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
 * and close the files used (if any) for termination of the program
 * */
void program_destroy(char* vector1, char* vector2, FILE* f1, FILE* f2){
    if(vector1) free(vector1);
    if(vector2) free(vector2);
    if(f1) fclose(f1);
    if(f2) fclose(f2);
}

/**Function that receives an input string and the transformation function
 * file and returns the output according to the file. In case of error
 * returns the string "E"
 * */
char* transform(char *vector, FILE * funct){
    char line[MAX_CHAR];
    char *function_input = NULL, *function_output = NULL;

    rewind(funct);
    while(fgets(line, sizeof line, funct)!= NULL){ 
      function_input = strtok(line, " ");
      function_output = strtok(NULL, " ");

      if (strcmp(function_input, vector) == 0){
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

/**
 * Prints the vector using spaces for the character 0
 * and using # for the character 1, for visualization
 * purposes
 * */
void pretty_print(char* str){
    int i, sz = strlen(str);
    for(i=0; i<sz; i++){
        if (str[i] == '0') printf (" ");
        else printf ("#");
    }
    printf("\n");
}

int main(int argc, char const *argv[]) {
    FILE * initial_configuration = NULL;
    FILE * transformation_function = NULL;
    char size[MAX_CHAR] = "";
    char *input;
    char *output;
    char *partial_input = (char*) calloc (sizeof(char), 3);
    char char_act;
    int tam = -1, num_iterations = -1, i;

    //Argument check
    if (argc != 4){
        fprintf(stderr, "Incorrect arguments. Try ./Cellular1D-Sequencial initial_configuration "
                        "transformation_function number_iterations\n");
        return EXIT_FAILURE;
    }

    num_iterations = atoi(argv[3]);
    if (num_iterations<1){
        fprintf(stderr, "Number of iterations must be > 0\n");
        return EXIT_FAILURE;
    }


 
    initial_configuration = fopen (argv[1], "r");
    transformation_function = fopen(argv[2], "r");
    if (!initial_configuration || !transformation_function){
        fprintf(stderr, "Files do not exist or could not open them\n");
        program_destroy(input, output, transformation_function, initial_configuration);
        return EXIT_FAILURE;
    }
    
    fgets(size, MAX_CHAR, initial_configuration);
    tam = atoi(size);
    if (tam<1){
        fprintf(stderr, "Error. Size of input vector is < 1\n");
        program_destroy(input, output, transformation_function, initial_configuration);
        return(EXIT_FAILURE);
    }

    //memory allocation for input/output vectors
    input = (char*) calloc (sizeof(char), tam+1);
    output = (char*) calloc (sizeof(char), tam+1);
    
    //reading input from file
    for(i=0; i<tam; i++){
        char_act = fgetc(initial_configuration);
        if(char_act != EOF){
            input[i] = char_act;
            if(input[i] != '0' && input[i] != '1'){
                fprintf(stderr, "Initial configuration file contains non-boolean value\n");
                program_destroy(input, output, transformation_function, initial_configuration);
                return EXIT_FAILURE;
            }
        }
    }
    
    pretty_print(input);

    //start of iterative loop
    while(num_iterations > 0){
        for (i = 0; i<tam; i++){
            partial_input[0] = input[module(i-1, tam)];
            partial_input[1] = input[module(i, tam)];
            partial_input[2] = input[module(i+1, tam)];
            output[i] = *transform(partial_input, transformation_function);
        }

        pretty_print(output);

        //the output becomes the new input for next iteration
        for(i=0; i<tam; i++){
            input[i] = output[i];
        }
        num_iterations--;
    }
    
    //free resources
    program_destroy(input, output, transformation_function, initial_configuration);
    free(partial_input);
    return EXIT_SUCCESS;    
}
