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
	{
		if(resultReal(st))
			return 0;
		return -1;
	}

	uint32_t out_row = findOutRow(st, in_col);
	if(out_row == UINT32_MAX)
		return -1;

	return getNextSimplexTableBy(st, out_row, in_col);
}
int getNextSimplexTableBy(struct SimplexTable *st, uint32_t out_row, uint32_t in_col)
{
	if(st->last_table_i+1 >= st->tables_quan)
			expandTables(st);

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

void printTables(struct SimplexTable *st, FILE *out)
{
	if(!st->ready_to_use)
		return;
	if(out == NULL)
		out = stdout;
	if(!resultReal(st))
	{
		fprintf(out, "No optimal results!");
		return;
	}

	for(int i=0; i<st->last_table_i+1; i++)
	{
		fprintf(out, "Bx,val,");
		for(int i=1; i<st->cols; i++)
			fprintf(out, "x%d,", i);
		fprintf(out, "\n");
		for(int j=0; j<st->base_rows; j++)
		{
			fprintf(out, "x%d,", st->base_indexes_table[i][j]);
			for(int k=0; k<st->cols; k++)
				fprintf(out, "%g,", st->tables[i][j][k]);
			fprintf(out, "\n");
		}
		fprintf(out, "delta,");
		for(int j=0; j<st->cols; j++)
			fprintf(out, "%g,", st->tables[i][st->base_rows][j]);
		fprintf(out, "\n\n");
	}
}

void printInitFunc(struct SimplexTable *st, FILE *out)
{
	if(!st->ready_to_use)
		return;
	if(!out)
		out = stdout;
	fprintf(out, "f:,");
	fprintf(out, "%g*x%d,", st->func_vector[1], 1);
	for(int i=st->init_cols_i+1; i<st->base_cols_i; i++)
		fprintf(out, "%+g*x%d,", st->func_vector[i], i);
	fprintf(out, "->,%s", st->mode==-1?"max\n\n":"min\n\n");
}

void printResults(struct SimplexTable *st, FILE* out)
{
	if(!st->ready_to_use)
		return;
	if(!out)
		out = stdout;
	fprintf(out, "Optimal function value:,%g\n", st->last_table[st->base_rows][0]);
	uint32_t row=0;
	for(int i=st->init_cols_i; i<st->base_cols_i; i++)
	{
		row = findInBasis(st, i);
		fprintf(out, "x%d:,", i);
		if(row == UINT32_MAX)
			fprintf(out, "0\n");
		else
			fprintf(out, "%g\n", st->last_table[row][0]);
	}
	fprintf(out, "\n");
}

void printSensitivity(struct SimplexTable *st, FILE* out)
{
	if(!st->ready_to_use)
		return;
	if(!out)
		out = stdout;

	fprintf(out, "Sensitivity of the solution:\n");
	for(int i=st->base_cols_i; i<st->x_base_cols_i; i++)
		if(st->func_vector[i] != st->M)
		{
			if(st->last_table[st->base_rows][i] == 0)
				fprintf(out, "A%d:,Can be %s from,%g,to,%g\n", i-st->base_cols_i+1, st->tables[0][i-st->base_cols_i][i] == -1? "increased": "reduced", st->tables[0][i-st->base_cols_i][0], expenses(st, i));
			else
				fprintf(out, "A%d:,Potential %s,%g\n", i-st->base_cols_i+1, (st->tables[0][i-st->base_cols_i][i] == 1?"growth":"reduces"), st->last_table[st->base_rows][i]);
		}
//	for(int i=st->x_base_cols_i; i<st->cols; i++)
//		if(st->func_vector[i] != st->M)
//		{
//			if(st->last_table[st->base_rows][i] == 0)
//				fprintf(out, "A%d:,Can be increased from,%g,to,%g\n", i-st->base_cols_i-st->x_base_cols+1, st->tables[0][i-st->base_cols_i-st->x_base_cols][0], expenses(st, i));
//			else
//				fprintf(out, "A%d:,Potential %s,%g\n", i-st->base_cols_i-st->x_base_cols+1, (st->mode==-1?"losses":"increases"), st->last_table[st->base_rows][i]);
//		}

	fprintf(out, "\n");
}

double expenses(struct SimplexTable *st, uint32_t col)
{
	uint32_t row = findInBasis(st, col);
	return st->last_table[row][0];
}

uint32_t findInBasis(struct SimplexTable *st, uint32_t col)
{
	for(int i=0; i<st->base_rows; i++)
		if(st->base_indexes[i] == col)
			return i;
	return UINT32_MAX;
}

int resultReal(struct SimplexTable *st)
{
	for(int i=0; i<st->base_rows; i++)
		if(st->func_vector[st->base_indexes[i]] == st->M && st->last_table[i][0] > 0.)
			return 0;
	return 1;
}

struct SimplexTable* copyTable(struct SimplexTable *st)
{
	struct SimplexTable *st_c = NULL;
	st_c = (struct SimplexTable *)calloc(1, sizeof(struct SimplexTable));
	st_c = (struct SimplexTable *)memcpy(st_c, st, sizeof(struct SimplexTable));
	st_c->tables = NULL;
	st_c->last_table = NULL;
	st_c->func_vector = NULL;
	st_c->base_indexes_table = NULL;
	st_c->base_indexes = NULL;

	st_c->tables = (double***)calloc(st_c->cols, sizeof(double**));
	for(int i=0; i<st_c->cols; i++)
	{
		st_c->tables[i] = (double**)calloc(st_c->rows, sizeof(double*));
		for(int j=0; j<st_c->rows; j++)
		{
			st_c->tables[i][j] = (double*)calloc(st_c->cols, sizeof(double));
			for(int k=0; k<st_c->cols; k++)
				st_c->tables[i][j][k] = st->tables[i][j][k];
		}
	}
	st_c->last_table = st_c->tables[st_c->last_table_i];
	st_c->base_indexes_table = (uint32_t**)calloc(st_c->cols, sizeof(uint32_t*));
	for(int i=0; i<st_c->cols; i++)
	{
		st_c->base_indexes_table[i] = (uint32_t*)calloc(st_c->base_rows, sizeof(uint32_t));
		for(int j=0; j<st->base_rows; j++)
			st_c->base_indexes_table[i][j] = st->base_indexes_table[i][j];
	}
	st_c->base_indexes = st_c->base_indexes_table[st_c->last_table_i];

	st_c->func_vector = (double*)calloc(st_c->cols, sizeof(double));
	st_c->func_vector = (double*)memcpy(st_c->func_vector, st->func_vector, st->cols*sizeof(double));

	return st_c;
}

//struct SimplexTable* copyLastTable(struct SimplexTable *st)


struct SimplexTable* initLimitation(struct SimplexTable *st)
{
	struct SimplexTable *st_c = NULL;
	st_c = (struct SimplexTable *)calloc(1, sizeof(struct SimplexTable));
	st_c = (struct SimplexTable *)memcpy(st_c, st, sizeof(struct SimplexTable));
	st_c->tables = NULL;
	st_c->last_table = NULL;
	st_c->func_vector = NULL;
	st_c->base_indexes_table = NULL;
	st_c->base_indexes = NULL;

	st_c->last_table_i = 0;
	st_c->cols++;
	st_c->rows++;
	st_c->base_rows++;
	st_c->base_cols++;
	st_c->x_base_cols_i++;

	st_c->tables = (double***)calloc(st_c->tables_quan, sizeof(double**));
	for(int i=0; i<st->tables_quan; i++)
	{
		st_c->tables[i] = (double**)calloc(st_c->rows, sizeof(double*));
		for(int j=0; j<st_c->rows; j++)
			st_c->tables[i][j] = (double*)calloc(st_c->cols, sizeof(double));
	}
	st_c->last_table = st_c->tables[st_c->last_table_i];
	st_c->base_indexes_table = (uint32_t**)calloc(st->tables_quan, sizeof(uint32_t*));
	for(int i=0; i<st->tables_quan; i++)
		st_c->base_indexes_table[i] = (uint32_t*)calloc(st_c->base_rows, sizeof(uint32_t));

	st_c->base_indexes = st_c->base_indexes_table[st_c->last_table_i];

	st_c->func_vector = (double*)calloc(st_c->cols, sizeof(double));

	for(int i=0; i<st->rows; i++)
		for(int j=0; j<st->cols; j++)
			st_c->tables[0][i>=st->base_rows?i+1:i][j>=st->x_base_cols_i?j+1:j] = st->last_table[i][j];
	for(int i=0; i<st->cols; i++)
		st_c->func_vector[i>=st->x_base_cols_i?i+1:i] = st->func_vector[i];

	memcpy(st_c->base_indexes_table[0], st->base_indexes, st->base_rows*sizeof(uint32_t));
	st_c->base_indexes_table[0][st_c->base_rows-1] = st_c->x_base_cols_i-1;

	return st_c;
}

uint32_t checkBranch(struct SimplexTable *st, uint32_t last_index)
{
	for(int i=1; i<last_index+1; i++)
		if(findInBasis(st, i) != UINT32_MAX)
			if((double)((int)st->last_table[findInBasis(st, i)][0]) != ROUND(st->last_table[findInBasis(st, i)][0]))
				return findInBasis(st, i);
	return UINT32_MAX;
}

struct SimplexTable* branch(struct SimplexTable *st, uint32_t row, uint32_t last_index)
{
	struct SimplexTable *gst = initLimitation(st);
	struct SimplexTable *lst = initLimitation(st);

	double lnum = (double)((int)st->last_table[row][0]);
	double gnum = (double)((int)st->last_table[row][0]+1);

	for(int i=0; i<st->cols; i++)
	{
		lst->last_table[lst->base_rows-1][i] = -lst->last_table[row][i];
		gst->last_table[gst->base_rows-1][i] = gst->last_table[row][i];
	}
	gst->last_table[gst->base_rows-1][0] -= gnum;
	lst->last_table[lst->base_rows-1][0] += lnum;
	gst->last_table[gst->base_rows-1][gst->x_base_cols_i-1] += 1;
	gst->last_table[gst->base_rows-1][gst->base_indexes[row]] -= 1;
	lst->last_table[lst->base_rows-1][lst->x_base_cols_i-1] += 1;
	lst->last_table[lst->base_rows-1][lst->base_indexes[row]] += 1;

	freeSimplexTable(st);

	int resultg = getOptimalReversSimplex(gst);
	int resultl = getOptimalReversSimplex(lst);

	if(checkBranch(gst, last_index) == UINT32_MAX)
	{
		freeSimplexTable(lst);
		return gst;
	}
	if(checkBranch(lst, last_index) == UINT32_MAX)
	{
		freeSimplexTable(gst);
		return lst;
	}

	if(resultg && resultl)
	{
		if(gst->last_table[gst->base_rows][0]>lst->last_table[lst->base_rows][0])
		{
			freeSimplexTable(lst);
			return gst;
		}
		else
		{
			freeSimplexTable(gst);
			return lst;
		}
//		return gst->last_table[gst->base_rows][0]>lst->last_table[lst->base_rows][0]? gst: lst;
	}
	if(resultg)
	{
		freeSimplexTable(lst);
		return gst;
	}
	if(resultl)
	{
		freeSimplexTable(gst);
		return lst;
	}
	return NULL;
}

uint32_t findInRow(struct SimplexTable *st)
{
	uint32_t row = UINT32_MAX;
	double local_max = 0;
	for(int i=0; i<st->base_rows; i++)
		if(ROUND(st->last_table[i][0]) < local_max)
		{
			row = i;
			local_max = st->last_table[i][0];
		}
	return row;
}

uint32_t findOutCol(struct SimplexTable *st, uint32_t row)
{
	uint32_t col = UINT32_MAX;
	double delta = -DBL_MAX;
	for(int i=1; i<st->x_base_cols_i; i++)
		if(st->last_table[row][i] < 0 && st->last_table[st->base_rows][i]/st->last_table[row][i] > delta)
		{
			delta = st->last_table[st->base_rows][i]/st->last_table[row][i];
			col = i;
		}
	return col;
}

int getOptimalReversSimplex(struct SimplexTable *st)
{
	int result = 0;

	do{
		uint32_t in_row = findInRow(st);
		if(in_row == UINT32_MAX)
			return 1;
		uint32_t out_col = findOutCol(st, in_row);
		if(out_col == UINT32_MAX)
			return 0;
		getNextSimplexTableBy(st, in_row, out_col);
	}while(1);

	return result;
}

