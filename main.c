/*
 * main.c
 *
 *  Created on: Mar 9, 2018
 *      Author: antonbogovis
 */

#include <stdio.h>

#include "simplextable.h"
#include "parsetable.h"

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
    	result = getNextSimplexTable(&st);
    while(result == 1);
    if(result == 0)
    	printf("Success\n");
    if(result == -1)
    	printf("Fiasco\n");

    freeSimplexTable(&st);
    return 0;
}

