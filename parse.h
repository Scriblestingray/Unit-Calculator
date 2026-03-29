#pragma once

#include "structs.h"
#include "calc.h"

typedef struct {
    char *chars;
    int len;
} String;

Result ParseAndEvalExpression(char* expression, Systems systems);
void PrintResult(Result result);
void PrintResultNoLine(Result result);