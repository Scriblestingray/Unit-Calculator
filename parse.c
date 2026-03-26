#include "structs.h"
#include "temp_units.h"
#include "calc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parse.h"

// NOTE: "unit_systems" and "unit_system_count" are global variables.
// Along with "resolved_units"

int IsAlpha(char chr) {
    return (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z');
}

int IsDigit(char chr) {
    return (chr >= '0' && chr <= '9') || chr == '.';
}

int IsAlphanumeric(char chr) {
    return IsAlpha(chr) || IsDigit(chr);
}

char Lowercase(char chr) {
    return chr >= 'A' && chr <= 'Z' ? chr + 32 : chr;
}

void AdvanceIndex(String expression, int *index)
{
    do {
        (*index)++;
    }
    while (*index < expression.len && (expression.chars[*index] == ' ' ||
            expression.chars[*index] == '\t' || expression.chars[*index] == '\n'));
}

#define MAX_UNIT_LEN 10

Unit* SearchUnit(char *chars, int len) {
    for (int i = 0; i < unit_system_count; i++) {
        UnitSystem *system = &unit_systems[i];
        for (int j = 0; j < system->unit_count; j++) {
            if (strncmp(chars, system->units[j].abbreviation, len) == 0)
                return &system->units[j];
        }
    }
    return NULL;
}

// ParseValue(...) references it, in a recursive fashion
Result ParseAddSub(String expression, int *index);

Result ParseValue(String expression, int *index)
{
    if (*index >= expression.len)
        return (Result) {.status = UNEXPECTED_END};

    if (expression.chars[*index] == '(') {
        AdvanceIndex(expression, index);
        Result inner = ParseAddSub(expression, index);
        if (*index >= expression.len || expression.chars[*index] != ')') {
            return (Result) { .status = EXPECTED_RPAREN };
        }
        AdvanceIndex(expression, index);
        return inner;
    }

    Number acc = NumberInit();

    while (*index < expression.len && IsAlphanumeric(expression.chars[*index])) {
        unsigned long long mul = 1;
        int is_fraction = 0;
        Number current_number = NumberInit();
        char unit_chars[MAX_UNIT_LEN] = { 0 };
        int unit_len = 0;
        int digits_parsed = 0;
        while (*index < expression.len && IsDigit(expression.chars[*index])) {
            digits_parsed = 1;
            if (expression.chars[*index] == '.') {
                is_fraction = 1;
                mul = 1;
            } else if (is_fraction) {
                // ideally check overflow
                current_number.numerator *= 10;
                current_number.denominator *= 10;
                current_number.numerator += expression.chars[*index] - '0';
            } else {
                // ideally check overflow
                current_number.base *= 10;
                current_number.base += expression.chars[*index] - '0';
            }
            AdvanceIndex(expression, index);
        }
        while (unit_len < MAX_UNIT_LEN && IsAlpha(expression.chars[*index])) {
            unit_chars[unit_len] = Lowercase(expression.chars[*index]);
            unit_len++;
            AdvanceIndex(expression, index);
        }
        if (unit_len > 0) {
            Unit* unit = SearchUnit(unit_chars, unit_len);
            if (unit == NULL) {
                return (Result) { .status = NO_SUCH_UNIT };
            }
            if (NumberIsZero(current_number) && !digits_parsed) {
                current_number.base = 1;
            }
            current_number.derived_unit.units[0] = unit;
            current_number.derived_unit.exponents[0] = 1;
            current_number.derived_unit.unit_count = 1;
            unit_len = 0;
        }
        Result acc_potential = Add(acc, current_number);
        if (acc_potential.status != SUCCESS) {
            return acc_potential;
        }
        acc = acc_potential.value;
        digits_parsed = 0;
    }

    acc = Balance(acc);

    return (Result) { .status = SUCCESS, .value = acc };
}

typedef enum {
    POS, NEG, IDENTITY
} UnaryOp;

Result ParseUnary(String expression, int *index)
{
    UnaryOp op = IDENTITY;
    if (*index < expression.len &&expression.chars[*index] == '+') {
        op = POS;
        AdvanceIndex(expression, index);
    } else if (*index < expression.len && expression.chars[*index] == '-') {
        op = NEG;
        AdvanceIndex(expression, index);
    }
    Result value = ParseValue(expression, index);
    if (op == NEG) {
        value.value.sign = !value.value.sign;
    } else if (op == POS) {
        value.value.sign = 0;
    }
    return value;
}

// Possibly stick exponent operator in-between here?

Result ParseMulDiv(String expression, int *index)
{
    Result op1 = ParseUnary(expression, index);
    while (1) {
        if (*index < expression.len && expression.chars[*index] == '*') {
            AdvanceIndex(expression, index);
            Result op2 = ParseUnary(expression, index);
            op1 = MultiplyResult(op1, op2);
            if (op1.status != SUCCESS)
                return op1;
        } else if (*index < expression.len && expression.chars[*index] == '/') {
            AdvanceIndex(expression, index);
            Result op2 = ParseUnary(expression, index);
            op1 = DivideResult(op1, op2);
            if (op1.status != SUCCESS)
                return op1;
        } else {
            break;
        }
    }
    return op1;
}

Result ParseAddSub(String expression, int *index)
{
    Result op1 = ParseMulDiv(expression, index);
    while (1) {
        if (*index < expression.len && expression.chars[*index] == '+') {
            AdvanceIndex(expression, index);
            Result op2 = ParseMulDiv(expression, index);
            op1 = AddResult(op1, op2);
            if (op1.status != SUCCESS)
                return op1;
        } else if (*index < expression.len && expression.chars[*index] == '-') {
            AdvanceIndex(expression, index);
            Result op2 = ParseMulDiv(expression, index);
            op1 = SubtractResult(op1, op2);
            if (op1.status != SUCCESS)
                return op1;
        } else {
            break;
        }
    }
    return op1;
}

Result ParseAndEvalExpression(char* expression)
{
    String str = (String) {.chars = expression, .len = strnlen(expression, 256)};
    int index = 0;
    return ParseAddSub(str, &index);
}


void PrintUnits(DerivedUnit derived_unit) {
    for (int i = 0; i < derived_unit.unit_count; i++) {
        if (derived_unit.exponents[i] < 0) {
            continue;
        } else if (derived_unit.exponents[i] > 0 && i > 0) {
            printf("*");
        }
        if (derived_unit.exponents[i] != 0) {
            printf("%s", derived_unit.units[i]->abbreviation);
        }
        if (abs(derived_unit.exponents[i]) != 1) {
            printf("%d", abs(derived_unit.exponents[i]));
        }
    }

    for (int i = 0; i < derived_unit.unit_count; i++) {
        if (derived_unit.exponents[i] < 0) {
            printf("/");
            printf("%s", derived_unit.units[i]->abbreviation);
            if (abs(derived_unit.exponents[i]) != 1) {
                printf("%d", abs(derived_unit.exponents[i]));
            }
        }
    }
}

void PrintResultNoLine(Result result) {
    if (result.status == SUCCESS) {
        result.value.derived_unit = BalanceUnit(result.value.derived_unit);

        if (result.value.sign)
            printf("-");

        // print the whole number part
        // experiment with putting underscores every 3 digits
        printf("%llu", result.value.base);

        unsigned long long num = result.value.numerator;
        unsigned long long denom = result.value.denominator;
        
        if (num > 0) {
            printf(".");
            for (int i = 0; i < 25 && num > 0; i++) {
                num *= 10;
                int digit = (int)(num / denom);
                printf("%d", digit);
                num -= ((unsigned long long) digit) * denom;
            }
        }

        // write all the multiplied units first, then the divided units

        PrintUnits(result.value.derived_unit);

        num = result.value.numerator;
        if (num > 0) {
            printf("    { ");
            if (result.value.sign)
                printf("-");
            if (result.value.base > 0)
                printf("%llu ", result.value.base);
            printf("%llu/%llu ", num, denom);
            if (result.value.derived_unit.unit_count > 0) {
                PrintUnits(result.value.derived_unit);
                printf(" ");
            }
            printf("}");
        }

    } else {
        printf("ERROR: ");
        switch (result.status) {
            case SUCCESS: { printf("UNREACHABLE."); break; }
            case SYNTAX_ERROR: { printf("Invalid syntax."); break; }
            case NUMBER_TOO_LARGE: { printf("Resulting numbers are too large."); break; }
            case NO_SUCH_UNIT: { printf("No such unit."); break; }
            case EXPECTED_RPAREN: { printf("Missing parenthesis."); break; }
            case UNEXPECTED_END: { printf("Unexpected end of expression."); break; }
            case UNITS_DONT_MATCH: { printf("Units do not match."); break; }
        }
        // print error. also print location in expression if char_start >= 0
    }
}

void PrintResult(Result result)
{
    PrintResultNoLine(result);
    printf("\n");
}