
#include "parse.h"
#include "temp_units.h"

#include <stdio.h>

typedef struct {
    char *expression;
    int sign;
    unsigned long long base;
    unsigned long long num;
    unsigned long long denom;
    DerivedUnit unit;
} ParseTestEntry;

static ParseTestEntry parse_entries[] = {
    // whole numbers
    { "345", 0, 345, 0, 1 },
    { "4068 + 4763", 0, 8831, 0, 1 },
    { "4763 + 4068", 0, 8831, 0, 1 },
    { "654 - 3234", 1, 2580, 0, 1 },
    { "3234 - 654", 0, 2580, 0, 1 },
    { "6912 * 731", 0, 5052672, 0, 1 },
    { "731 * 6912", 0, 5052672, 0, 1 },
    { "-3 * 39", 1, 117, 0, 1 },
    { "59 * -32", 1, 1888, 0, 1 },
    { "-395 * -31", 0, 12245, 0, 1 },
    { "3906 / 6", 0, 651, 0, 1 },
    { "6 / 3906", 0, 0, 1, 651 },
    // fractions
    { "0.002", 0, 0, 1, 500 },
    { "-0.002", 1, 0, 1, 500 },
    { "0.5 + 0.75", 0, 1, 1, 4 },
    { "0.5 - 0.75", 1, 0, 1, 4 },
    { "937 - 0.3", 0, 936, 7, 10 },
    { "-391 + 0.4", 1, 390, 3, 5 },
    { "30.3 - 60.1", 1, 29, 4, 5 },
    { "3952.5 * 394.5", 0, 1559261, 1, 4 },
    { "3952.5 * 394.1", 0, 1557680, 1, 4 },
    { "1 / 4", 0, 0, 1, 4 },
    { "-1 / 4", 1, 0, 1, 4 },
    { "-1 / .25", 1, 4, 0, 1 },
    { "-1 / -3.1", 0, 0, 10, 31 },
    { "8958937768937/2851718461558", 0, 3, 403782384263, 2851718461558 },
    // parentheses
    { "(4068) + 4763", 0, 8831, 0, 1 },
    { "-1 / (1/4)", 1, 4, 0, 1 },
    // chained expressions
    { "1 + 1 + 1", 0, 3, 0, 1 },
    { "2 * 2 / 2", 0, 2, 0, 1 },
    // unit + unitless
    { "1m + 12", 0, 13, 0, 1, (DerivedUnit) { { &metric_units[2] }, { 1 }, 1 } },
    { "5 - 4m", 0, 1, 0, 1, (DerivedUnit) { { &metric_units[2] }, { 1 }, 1 } },
    { "45 * 3.4m", 0, 153, 0, 1, (DerivedUnit) { { &metric_units[2] }, { 1 }, 1 } },
    { "40 / .1m", 0, 400, 0, 1, (DerivedUnit) { { &metric_units[2] }, { 1 }, 1 } },
    { 0 },
    // all forms of expressions with one number being a unit, both having same units
    // both having different units of the same quantity,
    // and for multiplication and division, different units
    // for different units of the same quantity, also test ones that are close and ones
    // that are far apart, and in different unit systems
};

int DerivedUnitEquals(DerivedUnit unit1, DerivedUnit unit2) {
    unit1 = BalanceUnit(unit1);
    unit2 = BalanceUnit(unit2);
    if (unit1.unit_count != unit2.unit_count)
        return 0;
    for (int i = 0; i < unit1.unit_count; i++) {
        printf("compare %lld, %lld, %d, %d\n", unit1.units[i], unit2.units[i], unit1.exponents[i], unit2.exponents[i]);
        if (unit1.units[i] != unit2.units[i] || unit1.exponents[i] != unit2.exponents[i])
            return 0;
    }
    return 1;
}

int NumEquals(Number num1, Number num2) {
    return num1.sign == num2.sign && num1.base == num2.base && 
           num1.numerator == num2.numerator && num1.denominator == num2.denominator &&
           DerivedUnitEquals(num1.derived_unit, num2.derived_unit);
}

int main() {
    ParseTestEntry *entry = &parse_entries[0];

    int tested = 0;
    int succeeded = 0;

    while (entry->expression != 0) {
        Result result = ParseAndEvalExpression(entry->expression);
        
        if (result.status != SUCCESS) {
            printf("X | %s = ", entry->expression);
            PrintResult(result);
        } else if (!NumEquals((Number) { entry->sign, entry->base, entry->num, entry->denom, entry->unit }, result.value)) {
            printf("X | %s = ", entry->expression);
            PrintResultNoLine(result);
            printf(" (expected ");
            PrintResultNoLine((Result) { SUCCESS, (Number) { entry->sign, entry->base, entry->num, entry->denom, entry->unit } });
            printf(")\n");
        } else {
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