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

#include "functions.h"

#define N_GENERATIONS 9

void generate_gameoflife(){
    FILE* generate;
    int i, index = N_GENERATIONS, divisor, alive=0;
    char linea[N_GENERATIONS+2] = "";
    char result;

    generate = fopen("gameOfLife.txt", "w+");
    divisor = 2^index;

    for (i=0; i<pow(2, N_GENERATIONS); i++){
        index = N_GENERATIONS-1;
        divisor = i;
        alive=0;
        while(index>=0){
            linea[index] = divisor%2 + '0';
            if(divisor%2 == 1) alive++;
            divisor/=2;
            index--;
        }
        linea[N_GENERATIONS] = ' ';
        if(alive<3) result = '0';
        else if(alive>4) result = '0';
        else result = '1';
        linea[N_GENERATIONS+1] = result;

        fprintf(generate, "%s\n", linea);
    }
    fclose(generate);
}


void generate_2k(int k){
    FILE *file;
    int i, sum = 0, tam = pow(2, k); 

    sum = 2*pow(2, k);
    
    file = fopen("2kconfig.txt", "w+");
    fprintf(file, "%d\n", sum);
    for(i=0; i<tam; i++){
        fputc('0', file);
    }
    fputc('1', file);
    for(i=0; i<tam-1; i++) fputc('0', file);
    fclose(file);
}

void generate_nxn(int n){
    FILE *file = fopen("nxnconfig.txt", "w+");
    int i, j;
    char *line = (char*) calloc (sizeof(char), n);
    fprintf(file, "%d\n", n);
    srand(time(NULL));
    for(i=0; i<n; i++){
        for(j=0; j<n; j++){
            line[j] = rand()%2 + '0';
        }
        fprintf(file, "%s", line);
	if(i != n-1) fprintf(file, "\n");
    }
    fclose(file);
    free(line);
}
