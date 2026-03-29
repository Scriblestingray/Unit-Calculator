
#include "parse.h"
#include "temp_units.h"
#include "Config.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

typedef struct
{
    char *expression;
    int sign;
    unsigned long long base;
    unsigned long long num;
    unsigned long long denom;
    DerivedUnit unit;
} ParseTestEntry;

static ParseTestEntry parse_entries[] = {
    // whole numbers
    {"345", 0, 345, 0, 1},
    {"4068 + 4763", 0, 8831, 0, 1},
    {"4763 + 4068", 0, 8831, 0, 1},
    {"654 - 3234", 1, 2580, 0, 1},
    {"3234 - 654", 0, 2580, 0, 1},
    {"6912 * 731", 0, 5052672, 0, 1},
    {"731 * 6912", 0, 5052672, 0, 1},
    {"-3 * 39", 1, 117, 0, 1},
    {"59 * -32", 1, 1888, 0, 1},
    {"-395 * -31", 0, 12245, 0, 1},
    {"3906 / 6", 0, 651, 0, 1},
    {"6 / 3906", 0, 0, 1, 651},
    // fractions
    {"0.002", 0, 0, 1, 500},
    {"-0.002", 1, 0, 1, 500},
    {"0.5 + 0.75", 0, 1, 1, 4},
    {"0.5 - 0.75", 1, 0, 1, 4},
    {"937 - 0.3", 0, 936, 7, 10},
    {"-391 + 0.4", 1, 390, 3, 5},
    {"30.3 - 60.1", 1, 29, 4, 5},
    {"3952.5 * 394.5", 0, 1559261, 1, 4},
    {"3952.5 * 394.1", 0, 1557680, 1, 4},
    {"1 / 4", 0, 0, 1, 4},
    {"-1 / 4", 1, 0, 1, 4},
    {"-1 / .25", 1, 4, 0, 1},
    {"-1 / -3.1", 0, 0, 10, 31},
    {"8958937768937/2851718461558", 0, 3, 403782384263, 2851718461558},
    // parentheses
    {"(4068) + 4763", 0, 8831, 0, 1},
    {"-1 / (1/4)", 1, 4, 0, 1},
    // chained expressions
    {"1 + 1 + 1", 0, 3, 0, 1},
    {"2 * 2 / 2", 0, 2, 0, 1},
    // unit + unitless
    {"1m + 12", 0, 13, 0, 1, (DerivedUnit){{&metric_units[2]}, {1}, 1}},
    {"5 - 4m", 0, 1, 0, 1, (DerivedUnit){{&metric_units[2]}, {1}, 1}},
    {"45 * 3.4m", 0, 153, 0, 1, (DerivedUnit){{&metric_units[2]}, {1}, 1}},
    {"40 / .1m", 0, 400, 0, 1, (DerivedUnit){{&metric_units[2]}, {-1}, 1}},
    // same unit
    {"1m + 12m", 0, 13, 0, 1, (DerivedUnit){{&metric_units[2]}, {1}, 1}},
    {"5m - 4m", 0, 1, 0, 1, (DerivedUnit){{&metric_units[2]}, {1}, 1}},
    {"45m * 3.4m", 0, 153, 0, 1, (DerivedUnit){{&metric_units[2]}, {2}, 1}},
    {"40m / .1m", 0, 400, 0, 1},
    // different units
    {"1m + 100cm", 0, 2, 0, 1, (DerivedUnit){{&metric_units[2]}, {1}, 1}},
    {"2km - 100000cm", 0, 1, 0, 1, (DerivedUnit){{&metric_units[3]}, {1}, 1}},
    {"1km * 1000m", 0, 1, 0, 1, (DerivedUnit){{&metric_units[3]}, {2}, 1}},
    {"100cm + 1m", 0, 200, 0, 1, (DerivedUnit){{&metric_units[1]}, {1}, 1}},
    {"3600s + 1hr", 0, 7200, 0, 1, (DerivedUnit){{&time_units[0]}, {1}, 1}},
    {"1cm + 1in", 0, 3, 27, 50, (DerivedUnit){{&metric_units[1]}, {1}, 1}},
    {"1cm + 1ft", 0, 31, 12, 25, (DerivedUnit){{&metric_units[1]}, {1}, 1}},
    {"1ft + 1cm", 0, 1, 25, 762, (DerivedUnit){{&imperial_units[1]}, {1}, 1}},
    {"1ft * 1cm", 0, 0, 25, 762, (DerivedUnit){{&imperial_units[1]}, {2}, 1}},
    {"1ft/s * 1cm/s", 0, 0, 25, 762, (DerivedUnit){{&imperial_units[1], &time_units[0]}, {2, -2}, 2}},
    {"0ft/hr + 1ft/s", 0, 3600, 0, 1, (DerivedUnit){{&imperial_units[1], &time_units[2]}, {1, -1}, 2}},
    {0},
};

int DerivedUnitEquals(DerivedUnit unit1, DerivedUnit unit2)
{
    unit1 = BalanceUnit(unit1);
    unit2 = BalanceUnit(unit2);
    if (unit1.unit_count != unit2.unit_count)
        return 0;
    for (int i = 0; i < unit1.unit_count; i++)
    {
        if (strncmp(unit1.units[i]->abbreviation, unit2.units[i]->abbreviation, 20) != 0 ||
            unit1.exponents[i] != unit2.exponents[i])
            return 0;
    }
    return 1;
}

int NumEquals(Number num1, Number num2)
{
    return num1.sign == num2.sign && num1.base == num2.base &&
           num1.numerator == num2.numerator && num1.denominator == num2.denominator &&
           DerivedUnitEquals(num1.derived_unit, num2.derived_unit);
}

int main()
{
    setlocale(LC_ALL, "");
    ParseTestEntry *entry = &parse_entries[0];
    Systems systems = CreateSystems();

    int tested = 0;
    int succeeded = 0;

    while (entry->expression != 0)
    {
        Result result = ParseAndEvalExpression(entry->expression, systems);

        if (result.status != SUCCESS)
        {
            printf("X | %s = ", entry->expression);
            PrintResult(result);
        }
        else if (!NumEquals((Number){entry->sign, entry->base, entry->num, entry->denom, entry->unit}, result.value))
        {
            printf("X | %s = ", entry->expression);
            PrintResultNoLine(result);
            printf(" (expected ");
            PrintResultNoLine((Result){SUCCESS, (Number){entry->sign, entry->base, entry->num, entry->denom, entry->unit}});
            printf(")\n");
        }
        else
        {
            printf("_ | %s = ", entry->expression);
            PrintResult(result);
            succeeded += 1;
        }

        tested += 1;
        entry++;
    }

    printf("---------\n");
    printf("%d TESTS | %d SUCCEEDED | %d FAILED\n", tested, succeeded, tested - succeeded);
}