#include "structs.h"
#include "temp_units.h"

#include <stdio.h>
#include <string.h>

int IsAlpha(char chr) {
    return (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z');
}

int IsDigit(char chr) {
    return (chr >= '0' && chr <= '9') || chr == '.';
}

int IsAlphanumeric(char chr) {
    return IsAlpha(chr) || IsDigit(chr);
}

Number NumberInit() {
    return (Number) {
        .sign = 0,
        .base = 0,
        .numerator = 0,
        .denominator = 1,
        .derived_unit = (DerivedUnit) { 0, 0, 0 }
    };
}

typedef struct {
    char *chars;
    int len;
} String;

void AdvanceIndex(String expression, int *index)
{
    do {
        *index++;
    }
    while (*index < expression.len && (expression.chars[*index] == ' ' ||
            expression.chars[*index] == '\t' || expression.chars[*index] == '\n'));
}

Result ParseValue(String expression, int *index)
{
    if (expression.chars[*index] == '(') {
        Result inner = ParseAddSub(expression, index);
        // expect right parenthesis
        return inner;
    }

    Number acc = NumberInit();
    Number current_number = NumberInit();

    while (IsAlphanumeric(expression.chars[*index])) {
        unsigned long long mul = 1;
        while (IsDigit(expression.chars[*index])) {
            // start parsing a sequence of numbers

            
        }
        while (IsAlpha(expression.chars[*index])) {
            // start parsing a sequence of letters
            // find the unit
            // add unit to current_number
        }
    }

    // if starts with a digit, parse number and add to previous number
    // if starts with a letter, parse unit and append to last parsed number

    // consists of 0-9,
    // optional decimal point and further 0-9 digits,
    // followed by a unit (with a whole num afterward to indicate exponent of the unit)

    // or it's just a unit without any number (which is equivalent to 1 of that unit, for inputting derived units)

    // a number can consist of one or more of the first entry, like 6ft 20in, as long as they share the same quantity
    // parse such entries until running into a non alphanumeric character
    // convert that into a Result and return it
}

typedef enum {
    POS, NEG, IDENTITY
} UnaryOp;

Result ParseUnary(String expression, int *index)
{
    UnaryOp op = IDENTITY;
    if (expression.chars[*index] == '+') {
        op = POS;
        AdvanceIndex(expression, index);
    } else if (expression.chars[*index] == '-') {
        op = NEG;
        AdvanceIndex(expression, index);
    }
    Result value = ParseValue(expression, index);
    // compute pos or neg if needed, else pass value as-is.
}

// Possibly stick exponent operator in-between here?

Result ParseMulDiv(String expression, int *index)
{
    // op1 = ParseUnary()
    // if next char is '*', mul, if '/' it's div, else syntax error
    // op2 = ParseUnary()
    // compute op1 * op2 or op1 / op 2 depending on what was parsed
}

Result ParseAddSub(String expression, int *index)
{
    // op1 = ParseMulDiv()
    // if next char is '+', add, if '-' it's sub, else syntax error
    // op2 = ParseMulDiv()
    // compute op1 + op2 or op1 - op 2 depending on what was parsed
}

Result ParseAndEvalExpression(char* expression)
{
    String str = (String) {.chars = expression, .len = strnlen(expression, 256)};
    return ParseAddSub(str, 0);
}

void PrintResult(Result result)
{
    if (result.status == SUCCESS) {
        // print number
        // try to write it as a decimal if the denominator is a multiple of 1/10
        // in other words, don't convert it to float and then write it
        // do it long-division style
    } else {
        // print error. also print location in expression if char_start >= 0
    }
}

int main(int argc, char **argv)
{
    // TODO process all unit systems so each unit maps to its quantity's base unit
    if (argc > 0)
    {
        // Evaluate each argument as an expression
        for (int i = 0; i < argc; i++) {
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