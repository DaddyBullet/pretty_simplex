/*
 * parsetable.h
 *
 *  Created on: Mar 11, 2018
 *      Author: antonbogovis
 */

#ifndef PARSETABLE_H_
#define PARSETABLE_H_

#include "simplextable.h"

int parseSimplexFile(struct SimplexTable *st, FILE* simplex_file, char delim);
uint32_t calcDimention(char *line, char delim);
char*** separateText(char ***text_p, size_t rows, char delimiter);
char* cutDelim(char **line_p, char delimiter);
double findMax(double* arr, size_t size);


#endif /* PARSETABLE_H_ */
