#pragma once

#include "structs.h"

#define DERIVED_UNIT_LENGTH 10

typedef struct DerivedUnit DerivedUnit;
struct DerivedUnit {
    Unit *units[DERIVED_UNIT_LENGTH];
    int exponents[DERIVED_UNIT_LENGTH];
    int unit_count;
};

typedef struct Number Number;
struct Number {
    int sign; // 0 = positive, 1 = negative
    unsigned long long base;
    unsigned long long numerator;
    unsigned long long denominator;
    DerivedUnit derived_unit;
};

typedef enum Status Status;
enum Status {
    SUCCESS = 0,
    SYNTAX_ERROR,
    NUMBER_TOO_LARGE,
    NO_SUCH_UNIT,
    EXPECTED_RPAREN,
    UNEXPECTED_END,
    UNITS_DONT_MATCH,
};

typedef struct Result Result;
struct Result {
    Status status;
    Number value;
    int char_start;
    int char_end;
    char *message;
};

Number NumberInit();
int NumberIsZero(Number num);

unsigned long long GCD(unsigned long long denom1, unsigned long long denom2);
unsigned long long LCM(unsigned long long denom1, unsigned long long denom2);

Number Balance(Number num);
DerivedUnit BalanceUnit(DerivedUnit units);

Result AddUnitless(Number num1, Number num2);
Result MultiplyUnitless(Number num1, Number num2);

Result Add(Number num1, Number num2);
Result Subtract(Number num1, Number num2);
Result Multiply(Number num1, Number num2);
Result Divide(Number num1, Number num2);

Result AddResult(Result num1, Result num2);
Result SubtractResult(Result num1, Result num2);
Result MultiplyResult(Result num1, Result num2);
Result DivideResult(Result num1, Result num2);