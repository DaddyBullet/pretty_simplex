/*
 * main.c
 *
 *  Created on: Mar 9, 2018
 *      Author: antonbogovis
 */

#include <stdio.h>
#include <time.h>

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
    if(argn < 3)
    {
    	printf("No destination file defined\n");
		return 0;
    }

    FILE *simplex_file = fopen(args[1], "r");
    if(simplex_file == NULL)
    {
    	printf("Can't read from %s!", args[1]);
    	return 0;
    }
    FILE *output = fopen(args[2], "w");
    if(output == NULL)
	{
		printf("Can't write to %s!", args[2]);
		return 0;
	}
    if(argn > 3)
    	delim = args[3][0];

    int failed = parseSimplexFile(&st, simplex_file, delim);
    fclose(simplex_file);
    if(failed)
    {
    	printf("Failed to parse %s :(\n", args[1]);
    	return 0;
    }

    clock_t begin = clock();
    int result = 0;
    do
    	result = getNextSimplexTable(&st);
    while(result == 1);
    clock_t end = clock();
    if(result == 0)
    	printf("Success\n");
    if(result == -1)
    	printf("Fiasco\n");


    double time_spent = (double)(end - begin)*1000./CLOCKS_PER_SEC;

    printInitFunc(&st, output);
    printTables(&st, output);
    printResults(&st, output);
    printSensitivity(&st, output);

    fprintf(output, "Time escalated: %gms\n", time_spent);

    freeSimplexTable(&st);
    fclose(output);
    return 0;
}

