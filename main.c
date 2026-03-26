#include "structs.h"
#include "temp_units.h"
#include "calc.h"
#include "parse.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    
    if (argc > 1)
    {
        // Evaluate each argument as an expression
        for (int i = 1; i < argc; i++) {
            Result result = ParseAndEvalExpression(argv[i]);
            printf("%s = \n", argv[i]);
            PrintResult(result);
            printf("\n");
        }
    }
    else
    {
        // REPL mode
        printf("Unit calculator 1.0\n");
        printf("To exit, type 'exit' and hit Enter.\n");
        int is_running = 1;
        while (is_running)
        {
            printf(">> ");
            char str[256] = {0};
            scanf("%255s", str);
            if (strncmp(str, "exit", 256) == 0)
            {
                is_running = 0;
            }
            else
            {
                Result result = ParseAndEvalExpression(str);
                PrintResult(result);
                printf("\n");
            }
        }
    }
    return 0;
}