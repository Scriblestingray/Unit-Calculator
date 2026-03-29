#include "structs.h"
#include "temp_units.h"
#include "calc.h"
#include "parse.h"
#include "Config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    Systems systems = CreateSystems();
    if (argc > 1)
    {
        // Evaluate each argument as an expression
        for (int i = 1; i < argc; i++) {
            Result result = ParseAndEvalExpression(argv[i], systems);
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
            int index = -1;
            do {
                index++;
                str[index] = getchar();
            } while (index < 255 && str[index] != '\n');

            if (strncmp(str, "exit\n", 256) == 0)
            {
                is_running = 0;
            }
            else
            {
                Result result = ParseAndEvalExpression(str, systems);
                PrintResult(result);
                printf("\n");
            }
        }
    }
    return 0;
}