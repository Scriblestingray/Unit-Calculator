#pragma once

/*
Data structures for the unit systems
*/

typedef struct Quantity Quantity;
struct Quantity
{
    char *name;
};

typedef struct Unit Unit;
struct Unit
{
    char *abbreviation;
    int is_base_unit;
    Unit *base_unit;
    int multiplier;
    int divider; // NOTE: if 0, it is treated as if it was set to 1
    int offset; // offset is in terms of the unit itself
    Quantity *quantity;
};

typedef struct UnitSystem UnitSystem;
struct UnitSystem
{
    char *name;
    Unit *units;
    int unit_count;
};

#define DERIVED_UNIT_LENGTH 10

typedef struct DerivedUnit DerivedUnit;
struct DerivedUnit
{
    Unit *units[DERIVED_UNIT_LENGTH];
    int exponents[DERIVED_UNIT_LENGTH];
    int unit_count;
};

typedef struct Number Number;
struct Number
{
    int sign; // 0 = positive, 1 = negative
    unsigned long long base;
    unsigned long long numerator;
    unsigned long long denominator;
    DerivedUnit derived_unit;
};

typedef enum Status Status;
enum Status
{
    SUCCESS = 0,
    SYNTAX_ERROR = 1,
    NUMBER_TOO_LARGE = 2,
};

typedef struct Result Result;
struct Result
{
    Status status;
    Number value;
    int char_start;
    int char_end;
    char *message;
};