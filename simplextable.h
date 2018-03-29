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


int getNextSimplexTableBy(struct SimplexTable *st, uint32_t out_row, uint32_t in_col);
int getNextSimplexTable(struct SimplexTable *st);
int getOptimalReversSimplex(struct SimplexTable *st);
uint32_t findOutRow(struct SimplexTable *st, uint32_t col);
uint32_t findInCol(struct SimplexTable *st);
uint32_t findInRow(struct SimplexTable *st);
uint32_t findOutCol(struct SimplexTable *st, uint32_t row);

struct SimplexTable* copyTable(struct SimplexTable *st);
struct SimplexTable* copyLastTable(struct SimplexTable *st);
struct SimplexTable* initLimitation(struct SimplexTable *st);
struct SimplexTable* branch(struct SimplexTable *st, uint32_t row, uint32_t last_index);
uint32_t checkBranch(struct SimplexTable *st, uint32_t last_index);
int getIntegerSolution(struct SimplexTable *st);


void printInitFunc(struct SimplexTable *st, FILE* out);
void printTables(struct SimplexTable *st, FILE* out);
void printResults(struct SimplexTable *st, FILE* out);
void printSensitivity(struct SimplexTable *st, FILE* out);

uint32_t findInBasis(struct SimplexTable *st, uint32_t col);
double expenses(struct SimplexTable *st, uint32_t col);

void expandTables(struct SimplexTable *st);

void freeSimplexTable(struct SimplexTable *st);
int resultReal(struct SimplexTable *st);

#endif /* SIMPLEXTABLE_H_ */
