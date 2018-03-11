/*
 * simplextable.c
 *
 *  Created on: Mar 11, 2018
 *      Author: antonbogovis
 */

#include "simplextable.h"

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

void freeSimplexTable(struct SimplexTable *st)
{
	for(int i=0; i<st->tables_quan; i++)
	{
		for(int j=0; j<st->rows; j++)
			if(st->tables[i][j])
				free(st->tables[i][j]);
		if(st->tables[i])
			free(st->tables[i]);
	}
	if(st->tables)
		free(st->tables);
	if(st->func_vector)
		free(st->func_vector);
	for(int i=0; i<st->tables_quan; i++)
		if(st->base_indexes_table[i])
			free(st->base_indexes_table[i]);
	if(st->base_indexes_table)
		free(st->base_indexes_table);

	st->tables = NULL;
	st->last_table = NULL;
	st->func_vector = NULL;
	st->base_indexes_table = NULL;
	st->base_indexes = NULL;
}


void expandTables(struct SimplexTable *st)
{
	st->tables_quan *= 2;
	st->tables = (double***)realloc(st->tables, sizeof(double**) * st->tables_quan);
	for(int i=st->last_table_i+1; i<st->tables_quan; i++)
	{
		st->tables[i] = (double**)calloc(st->rows, sizeof(double*));
		for(int j=0; j<st->rows; j++)
			st->tables[i][j] = (double*)calloc(st->cols, sizeof(double));
	}

	st->base_indexes_table = (uint32_t**)realloc(st->base_indexes_table, sizeof(uint32_t*) * st->tables_quan);
	for(int i=st->last_table_i+1; i<st->tables_quan; i++)
		st->base_indexes_table[i] = (uint32_t*)calloc(st->base_rows, sizeof(uint32_t));

}
