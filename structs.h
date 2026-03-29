#pragma once

/*
Data structures for the unit systems
*/

typedef struct Unit Unit;
struct Unit
{
    char *abbreviation;
    int is_base_unit;
    Unit *base_unit;
    int multiplier;
    int divider; // NOTE: if 0, it is treated as if it was set to 1
    int offset;  // offset is in terms of the unit itself
    char *quantity;
    int visited; // Used for finding unit conversions, prevents infinite recursion
};

typedef struct UnitSystem UnitSystem;
struct UnitSystem
{
    char *name;
    Unit *units;
    int unit_count;
};

typedef struct Systems Systems;
struct Systems
{
    UnitSystem *systems;
    int count;
};