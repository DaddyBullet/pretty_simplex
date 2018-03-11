/*
 * parsetable.c
 *
 *  Created on: Mar 11, 2018
 *      Author: antonbogovis
 */
#include "parsetable.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


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
	if(!getdelim(&line, &len, delim, sf))
		return 1;
	cutDelim(&line, delim);// cut delimiter
	if(!strcmp(line, "max"))
		st->mode = -1;
	else if(!strcmp(line, "min"))
		st->mode = 1;
	else
		return 1; //failed to parse

	if(!getline(&line, &len, sf))
		return 1;
	st->init_cols = calcDimention(line, delim);
	if(!st->init_cols)
		return 1;
	st->init_cols_i = 1;

	char **st_info = NULL;
	st_info = (char**)calloc(st->rows, sizeof(char*));
	fseek(sf, 0, SEEK_SET);
	for(int i=0; i<st->rows; i++)
		if(!getline(&st_info[i], &len, sf))
			return 1;

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
	for(int i=0; i<st->rows; i++)
		if(sep_text[i])
			free(sep_text[i]);
	if(sep_text)
		free(sep_text);
	return 0;
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

