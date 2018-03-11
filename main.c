/*
 * main.c
 *
 *  Created on: Mar 9, 2018
 *      Author: antonbogovis
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <float.h>

struct SimplexTable{
	uint32_t rows;
	uint32_t cols;
	uint32_t init_cols;
	uint32_t init_cols_i;
	uint32_t base_cols;
	uint32_t base_cols_i;
	uint32_t x_base_cols;
	uint32_t x_base_cols_i;
	uint32_t base_rows;
	int8_t mode;
	uint32_t simplex_diference; // row index
	uint8_t ready_to_use;
	uint16_t last_table_i;
	uint16_t tables_quan;

	double M;
	double ***tables;
	double **last_table;
	double *func_vector;
	uint32_t **base_indexes_table;
	uint32_t *base_indexes;
}st;

int parseSimplexFile(struct SimplexTable *st, FILE* simplex_file, char delim);
uint32_t calcDimention(char *line, char delim);
char*** separateText(char ***text_p, size_t rows, char delimiter);
char* cutDelim(char **line_p, char delimiter);
double findMax(double* arr, size_t size);

int getNextSimplexTable(struct SimplexTable *st);
uint32_t findOutRow(struct SimplexTable *st, uint32_t col);
uint32_t findInCol(struct SimplexTable *st);


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

    int result = 0;
    do
    {
    	result = getNextSimplexTable(&st);
    }while(result == 1);
    if(result == 0)
    	printf("Success\n");
    if(result == -1)
    	printf("Fiasco\n");

    return 0;
}


int parseSimplexFile(struct SimplexTable *st, FILE* sf, char delim)
{
	char *line = NULL;
	size_t len = 0;
	st->rows = 0;
	st->base_rows = 0;
	while(getline(&line, &len, sf) != -1)
		st->rows++;

	if(!(st->rows))
		return 1;
	st->base_rows = st->rows-1;

	fseek(sf, 0, SEEK_SET);
	getdelim(&line, &len, delim, sf);
	cutDelim(&line, delim);// cut delimiter
	if(!strcmp(line, "max"))
		st->mode = -1;
	else if(!strcmp(line, "min"))
		st->mode = 1;
	else
		return 1; //failed to parse

	getline(&line, &len, sf);
	st->init_cols = calcDimention(line, delim);
	if(!st->init_cols)
		return 1;
	st->init_cols_i = 1;

	char **st_info = NULL;
	st_info = (char**)calloc(st->rows, sizeof(char*));
	fseek(sf, 0, SEEK_SET);
	for(int i=0; i<st->rows; i++)
		getline(&st_info[i], &len, sf);

	char ***sep_text = NULL;
	sep_text = (char***)calloc(st->rows, sizeof(char**));
	for(int i=0; i<st->rows; i++)
		sep_text[i] = (char**)calloc(st->init_cols+2, sizeof(char*));

	for(int i=0; i<st->rows; i++)
		{
			int j = 0;
			int k = 1;
			sep_text[i][0] = st_info[i];
			while(st_info[i][j])
			{
				if(st_info[i][j] == delim)
				{
					st_info[i][j] = '\0';
					sep_text[i][k]= &st_info[i][j+1];
					k++;
				}
				j++;
			}
		}
	st->base_cols = st->base_rows;
	st->base_cols_i = st->init_cols_i+st->init_cols;
	st->x_base_cols = 0;
	for(int i=1; i<st->rows; i++)
		if(!strcmp(sep_text[i][st->init_cols], ">="))
			st->x_base_cols++;
	st->x_base_cols_i = st->base_cols_i+st->base_cols;

	st->cols = st->init_cols+st->base_cols+st->x_base_cols+1; // 1 for definitions

	st->tables = (double***)calloc(st->cols, sizeof(double**));
	for(int i=0; i<st->cols; i++)
	{
		st->tables[i] = (double**)calloc(st->rows, sizeof(double*));
		for(int j=0; j<st->rows; j++)
			st->tables[i][j] = (double*)calloc(st->cols, sizeof(double));
	}
	st->last_table_i = 0;
	st->last_table = st->tables[st->last_table_i];
	st->tables_quan = st->cols;

	st->base_indexes_table = (uint32_t**)calloc(st->rows, sizeof(uint32_t*));
	for(int i=0; i< st->rows; i++)
		st->base_indexes_table[i] = (uint32_t*)calloc(st->base_rows, sizeof(uint32_t));
	st->base_indexes = st->base_indexes_table[st->last_table_i];

	st->func_vector = (double*)calloc(st->cols, sizeof(double));
	st->func_vector[0] = (double)st->mode; // not really used, but it will align data

	for(int i=1; i<st->init_cols+1; i++)
		st->func_vector[i] = atof(sep_text[0][i]);
	st->M = findMax(st->func_vector, st->base_cols)*1e6*st->mode;
	int x_basis_counter = 0;
	for(int i=1; i<st->rows; i++)
	{
		st->last_table[i-1][0] = atof(sep_text[i][st->init_cols+1]);
		for(int j=0; j<st->init_cols; j++)
			st->last_table[i-1][j+1] = atof(sep_text[i][j]);
		st->last_table[i-1][st->base_cols_i+i-1] = 1;
		if(strcmp("<=", sep_text[i][st->init_cols]))
			st->func_vector[st->base_cols_i+i-1] = st->M;
		if(!strcmp(">=", sep_text[i][st->init_cols]))
			st->last_table[i-1][st->x_base_cols_i+x_basis_counter++] = -1;
	}
	for(int i=0; i<st->base_rows; i++)
		st->base_indexes[i] = st->base_cols_i + i;
	st->last_table[st->base_rows][0] = 0;
	for(int j=0; j<st->base_rows; j++)
		st->last_table[st->base_rows][0]+=st->func_vector[st->base_indexes[j]]*st->last_table[j][0];
	for(int i=1; i<st->cols; i++)
	{
		st->last_table[st->base_rows][i] = -st->func_vector[i];
		for(int j=0; j<st->base_rows; j++)
			st->last_table[st->base_rows][i]+=st->func_vector[st->base_indexes[j]]*st->last_table[j][i];
	}

	st->ready_to_use = 1;

	// Free dat space
	if(st_info)
		for(int i=0; i<st->rows; i++)
			if(st_info[i])
				free(st_info[i]);
	free(st_info);
	if(sep_text)
		free(sep_text);
	return 0;
}

int getNextSimplexTable(struct SimplexTable *st)
{
	uint32_t in_col = findInCol(st);
	if(in_col == UINT32_MAX)
		return 0;

	uint32_t out_row = findOutRow(st, in_col);
	if(out_row == UINT32_MAX)
		return -1;

	if(st->last_table_i+1 >= st->tables_quan)
	{
		st->tables = (double***)realloc(st->tables, sizeof(double**) * st->tables_quan * 2);
		st->tables_quan *= 2;
		for(int i=st->last_table_i+1; i<st->tables_quan; i++)
		{
			st->tables[i] = (double**)calloc(st->rows, sizeof(double*));
			for(int j=0; j<st->rows; j++)
				st->tables[i][j] = (double*)calloc(st->cols, sizeof(double));
		}
	}


	for(int i=0; i<st->rows; i++)
		for(int j=0; j<st->cols; j++)
			st->tables[st->last_table_i+1][i][j] = st->last_table[i][j] - (st->last_table[out_row][j]*st->last_table[i][in_col])/st->last_table[out_row][in_col];
	for(int i=0; i<st->cols; i++)
		st->tables[st->last_table_i+1][out_row][i] = st->last_table[out_row][i]/st->last_table[out_row][in_col];
	for(int i=0; i<st->rows; i++)
		st->tables[st->last_table_i+1][i][in_col] = 0;
	st->tables[st->last_table_i+1][out_row][in_col] = 1;
	st->last_table_i++;
	st->last_table = st->tables[st->last_table_i];
	memcpy(st->base_indexes_table[st->last_table_i], st->base_indexes, sizeof(uint32_t)*(st->rows-1));
	st->base_indexes_table[st->last_table_i][out_row] = in_col;
	st->base_indexes = st->base_indexes_table[st->last_table_i];
	return 1;
}



uint32_t calcDimention(char *line, char delim)
{
	size_t retsize = 0;
	do
	{
		if(!isspace(*(strsep(&line, &delim))))
			retsize++;
	}while(line);
	return (uint32_t)retsize;

}


char*** separateText(char ***text_p, size_t rows, char delimiter)
{
	char **text = *text_p;
	char ***result = (char***)(text);
	for(int i=0; i<rows; i++)
	{
		int j = 0;
		int k = 1;
		result[i][0] = &text[i][0];
		while(text[i][j])
		{
			if(text[i][j] == delimiter)
			{
				text[i][j] = '\0';
				result[i][k]= &text[i][j+1];
				k++;
			}
			j++;
		}
	}


	return result;
}

char* cutDelim(char **line_p, char delimiter)
{
	char *line = *line_p;
	int i = 0;
	while(line[i])
	{
		if(line[i] == delimiter)
		{
			line[i] = '\0';
			return line;
		}
	i++;
	}
	return line;
}

double findMax(double* arr, size_t size)
{
	double max = 0;
	for(int i=0; i<size; i++)
		if(abs(arr[i]) > max)
			max = abs(arr[i]);
	return max;
}

uint32_t findInCol(struct SimplexTable *st)
{
	double local_min = 0;
	uint32_t ret_i = UINT32_MAX;
	for(int i=1; i<st->cols; i++)
		if(st->last_table[st->base_rows][i]*st->mode > local_min)
		{
			local_min = st->last_table[st->base_rows][i]*st->mode;
			ret_i = i;
		}
	return ret_i;
}

uint32_t findOutRow(struct SimplexTable *st, uint32_t col)
{
	double local_min = DBL_MAX;
	uint32_t ret_i = UINT32_MAX;
	for(int i=0; i<st->base_rows; i++)
		if(st->last_table[i][col] > 0 && st->last_table[i][0]/st->last_table[i][col] < local_min)
		{
			local_min = st->last_table[i][0]/st->last_table[i][col];
			ret_i = i;
		}
	return ret_i;
}
