/*
 * main.c
 *
 *  Created on: Mar 9, 2018
 *      Author: antonbogovis
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

struct SymplexTable{
	uint32_t rows;
	uint32_t cols;
	uint32_t base_cols;
	uint32_t base_rows;
	uint8_t mode;
	uint32_t simplex_diference; // row index
	uint8_t ready_to_use;


	double M;
	double **whole_matrix;
	double *func_vector;
	uint32_t *base_indexes;
}st;

int parseSimplexFile(struct SymplexTable *st, FILE* simplex_file, char delim);
uint32_t calcDimention(char *line, char delim);

int main(int argn, char *args[])
{
	// Starting up and opening file
	char delim = ',';
	st.ready_to_use = 0;
    if(argn < 2)
    {
        printf("Task doesn't defined\n");
        return 0;
    }
    FILE *simplex_file = fopen(args[1], "r");
    if(simplex_file == NULL)
    {
    	printf("Can't read from %s!", args[1]);
    	return 0;
    }
    if(argn > 2)
    	delim = args[2][0];

    int failed = parseSimplexFile(&st, simplex_file, delim);
    fclose(simplex_file);
    if(failed)
    {
    	printf("Failed to parse %s :(\n", args[1]);
    	return 0;
    }
}


int parseSimplexFile(struct SymplexTable *st, FILE* sf, char delim)
{
	char *line = NULL;
	size_t len = 0;
	st->rows = 0;
	st->base_rows = 0;
	while(getline(&line, &len, sf))
		st->rows++;

	if(!st->rows)
		return 1;
	st->base_rows = st->rows-1;

	fseek(sf, 0, SEEK_SET);
	getdelim(&line, &len, delim, sf);
	line[sizeof(line)-2]='\0'; // cut delimiter
	if(!strcmp(line, "max"))
		st->mode = 0;
	else if(!strcmp(line, "min"))
		st->mode = 1;
	else
		return 1; //failed to parse
	getline(&line, &len, sf);
	st->base_cols = calcDimention(line, delim);
	if(!st->base_cols)
		return 1;










}


uint32_t calcDimention(char *line, char delim)
{
	size_t retsize = 0;
	while(strsep(&line, &delim))
		retsize++; // TODO: might not work
	return (uint32_t)retsize;

}

