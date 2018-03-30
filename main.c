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

    FILE *simplex_file = fopen(args[1], "r");
    FILE *output = stdout;
    if(simplex_file == NULL)
    {
    	printf("Can't read from %s!", args[1]);
    	return 0;
    }
    if(argn > 2)
    {
    	if(strlen(args[2])<=2)
    		delim = args[2][0];
    	else
    	{
			output = fopen(args[2], "w");
			if(output == NULL)
			{
				printf("Can't write to %s!", args[2]);
				return 0;
			}
    	}
    }

    if(argn > 3)
	{
    	output = fopen(args[3], "w");
		if(output == NULL)
		{
			printf("Can't write to %s!", args[2]);
			return 0;
		}
	}

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
    struct SimplexTable *next_branch = copyTable(&st);
    int iterr=0;
    while(checkBranch(next_branch, st.init_cols) != UINT32_MAX)
    {
    	next_branch = branch(next_branch, checkBranch(next_branch, st.init_cols), st.init_cols);
    	if(!next_branch)
    	{
    		printf("U r in a deep  shit boi!!");
    		break;
    	}
//    	printf("Branch %d,\n", iterr);
//    	printTables(next_branch, output);
    	iterr++;
    }
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
    printResults(next_branch, output);

    fprintf(output, "Time escalated: %gms\n", time_spent);

    freeSimplexTable(&st);
    freeSimplexTable(next_branch);
    fclose(output);
    return 0;
}

