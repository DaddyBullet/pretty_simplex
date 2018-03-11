/*
 * simplextable.h
 *
 *  Created on: Mar 11, 2018
 *      Author: antonbogovis
 */
#ifndef SIMPLEXTABLE_H_
#define SIMPLEXTABLE_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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

int getNextSimplexTable(struct SimplexTable *st);
uint32_t findOutRow(struct SimplexTable *st, uint32_t col);
uint32_t findInCol(struct SimplexTable *st);

void printTables(struct SimplexTable *st);

void expandTables(struct SimplexTable *st);

void freeSimplexTable(struct SimplexTable *st);

#endif /* SIMPLEXTABLE_H_ */
