#include "calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Number NumberInit() {
    return (Number) {
        .sign = 0,
        .base = 0,
        .numerator = 0,
        .denominator = 1,
        .derived_unit = (DerivedUnit) { 0, 0, 0 }
    };
}

int NumberIsZero(Number num) {
    return num.base == 0 && num.numerator == 0;
}

unsigned long long GCD(unsigned long long denom1, unsigned long long denom2) {
    unsigned long long a = denom1;
    unsigned long long b = denom2;
    while (b != 0) {
        unsigned long long _a = a;
        a = b;
        b = _a % b;
    }
    return a;
}

unsigned long long LCM(unsigned long long denom1, unsigned long long denom2) {
    return (denom1 / GCD(denom1, denom2)) * denom2;
}

Number Balance(Number num) {
    unsigned long long gcd = GCD(num.numerator, num.denominator);
    num.numerator /= gcd;
    num.denominator /= gcd;
    unsigned long long extra = num.numerator / num.denominator;
    num.base += extra;
    num.numerator -= extra * num.denominator;
    return num;
}

#include "parse.h"

typedef enum Direction Direction;
enum Direction
{
    FORWARDS,
    BACKWARDS
};

typedef struct UnitChain UnitChain;
struct UnitChain
{
    Unit *unit;
    Direction direction;
    UnitChain *next;
};

int UnitEquals(Unit *unit1, Unit *unit2) {
    return strncmp(unit1->abbreviation, unit2->abbreviation, 20) == 0;
}

UnitChain *BuildChain(Unit *current_unit, Unit *last_unit, Systems systems) {
    
    // case 1: current_unit and last_unit are directly related to each other
    if (!last_unit->is_base_unit && UnitEquals(last_unit->base_unit, current_unit)) {
        UnitChain *chain = malloc(sizeof(UnitChain));
        *chain = (UnitChain) {
            .unit = last_unit,
            .direction = FORWARDS,
            .next = NULL
        };
        return chain;
    }
    if (!current_unit->is_base_unit && UnitEquals(current_unit->base_unit, last_unit)) {
        UnitChain *chain = malloc(sizeof(UnitChain));
        *chain = (UnitChain) {
            .unit = current_unit,
            .direction = BACKWARDS,
            .next = NULL
        };
        return chain;
    }

    // case 2:
    // they are NOT related to each other, propagate the chain one step forward
    // by recursively finding a chain using the current_unit's base unit.

    if (!current_unit->is_base_unit && !current_unit->base_unit->visited) {
        current_unit->base_unit->visited = 1;
        UnitChain *inner = BuildChain(current_unit->base_unit, last_unit, systems);
        current_unit->base_unit->visited = 0;
        if (inner != NULL) {
            UnitChain *chain = malloc(sizeof(UnitChain));
             *chain = (UnitChain) {
                .unit = current_unit,
                .direction = BACKWARDS,
                .next = inner
            };
            return chain;
        }
        free(inner);
    }

    // case 3:
    // propagating in that direction didn't work, or current unit has no
    // base unit.
    // try going the other direction: find a unit that uses current unit
    // as its base.

    for (int i = 0; i < systems.count; i++) {
        UnitSystem *system = &systems.systems[i];
        for (int j = 0; j < system->unit_count; j++) {
            if (UnitEquals(&system->units[j], last_unit))
                continue;
            if (strncmp(system->units[j].quantity, last_unit->quantity, 20) != 0)
                continue;
            if (system->units[j].is_base_unit)
                continue;
            if (!UnitEquals(system->units[j].base_unit, current_unit))
                continue;
            if (system->units[j].visited)
                continue;
            system->units[j].visited = 1;
            UnitChain *inner = BuildChain(&system->units[j], last_unit, systems);
            system->units[j].visited = 0;
            if (inner != NULL) {
                UnitChain *chain = malloc(sizeof(UnitChain));
                *chain = (UnitChain) {
                    .unit = &system->units[j],
                    .direction = FORWARDS,
                    .next = inner
                };
                return chain;
            }
            free(inner);
        }
    }

    return NULL;
}

// NOTE: This does not touch the derived_units inside num. This only adjusts the values.
Result ConvertUnit(Number num, Unit *old_unit, Unit *new_unit, Systems systems) {
    DerivedUnit old_derived_unit = num.derived_unit;

    UnitChain *chain = BuildChain(old_unit, new_unit, systems);
    UnitChain *prev_chain = NULL;
    for (; chain != 0; chain = chain->next) {
        if (prev_chain != 0)
            free(prev_chain);
        prev_chain = chain;
        if (chain->direction == BACKWARDS) {
            // num = num * multiplier / divider + offset
            Number mul = (Number) {
                .sign = 0,
                .base = 0,
                .numerator = chain->unit->multiplier == 0 ? 1 : chain->unit->multiplier,
                .denominator = chain->unit->divider == 0 ? 1 : chain->unit->divider,
            };
            Result inter = MultiplyUnitless(num, mul);
            if (inter.status != SUCCESS)
                return inter;
            Number add = (Number) {
                .sign = chain->unit->offset < 0,
                .base = abs(chain->unit->offset),
                .denominator = 1,
            };
            inter = AddUnitless(inter.value, add);
            if (inter.status != SUCCESS)
                return inter;
            num = inter.value;
        } else {
            // num = (num - offset) * divider / multiplier
            Number sub = (Number) {
                .sign = chain->unit->offset >= 0,
                .base = abs(chain->unit->offset),
                .denominator = 1,
            };
            Result inter = AddUnitless(num, sub);
            if (inter.status != SUCCESS)
                return inter;
            Number mul = (Number) {
                .sign = 0,
                .base = 0,
                .numerator = chain->unit->divider == 0 ? 1 : chain->unit->divider,
                .denominator = chain->unit->multiplier == 0 ? 1 : chain->unit->multiplier,
            };
            inter = MultiplyUnitless(inter.value, mul);
            if (inter.status != SUCCESS)
                return inter;
            num = inter.value;
        }
    }

    num.derived_unit = old_derived_unit;

    return (Result) {.status = SUCCESS, .value = num};
}

// Find which units match quantities and convert the number to more closely match the number.
// Non-matching quantities are ignored.
Result MatchUnits(Number num, DerivedUnit units, Systems systems) {
    for (int i = 0; i < num.derived_unit.unit_count; i++) {
        for (int j = 0; j < units.unit_count; j++) {
            if (strncmp(num.derived_unit.units[i]->quantity, units.units[j]->quantity, 20) == 0 &&
                num.derived_unit.exponents[i] == units.exponents[j] &&
                !UnitEquals(num.derived_unit.units[i], units.units[j])) {
                Result result;
                if (units.exponents[j] > 0)
                    result = ConvertUnit(num, num.derived_unit.units[i], units.units[j], systems);
                else
                    result = ConvertUnit(num, units.units[j], num.derived_unit.units[i], systems);
                if (result.status != SUCCESS)
                    return result;
                num = result.value;
                num.derived_unit.units[i] = units.units[j];
                break;
            }
        }
    }
    return (Result) { .status = SUCCESS, num };
}

typedef struct UnitExponentPair UnitExponentPair;
struct UnitExponentPair {
    Unit *unit;
    int exponent;
};

int CompareUnitOrder(const void *item1, const void *item2) {
    UnitExponentPair *pair1 = (UnitExponentPair*) item1;
    UnitExponentPair *pair2 = (UnitExponentPair*) item2;
    return pair1->unit < pair2->unit ? -1 : pair1->unit > pair2->unit ? 1 : 0;
}

DerivedUnit BalanceUnit(DerivedUnit units) {
    // copy to a data structure more suitable for sorting,
    // while removing empty units
    UnitExponentPair pairs[DERIVED_UNIT_LENGTH] = { 0 };
    int j = 0;
    for (int i = 0; i < units.unit_count; i++) {
        pairs[j].unit = units.units[i];
        pairs[j].exponent = units.exponents[i];
        if (units.exponents[i] != 0)
            j++;
    }
    units.unit_count = j;

    // sort the units based on pointer address
    // (the sorting order will change on each run, but the point is that
    // units will sort the same way throughout the program's run)
    qsort(pairs, j, sizeof(UnitExponentPair), CompareUnitOrder);

    // insert them back into the DerivedUnit object and return it
    for (int i = 0; i < units.unit_count; i++) {
        units.units[i] = pairs[i].unit;
        units.exponents[i] = pairs[i].exponent;
    }

    return units;
}

// Similar to MatchUnits, but enforces that the converted number must have the exact units.
Result MatchQuantities(Number num, DerivedUnit units, Systems systems) {
    Result matched = MatchUnits(num, units, systems);
    if (matched.status != SUCCESS)
        return matched;

    units = BalanceUnit(units);
    matched.value.derived_unit = BalanceUnit(matched.value.derived_unit);
    
    num = matched.value;

    if (units.unit_count != 0 && num.derived_unit.unit_count != 0) {
        if (units.unit_count != num.derived_unit.unit_count) {
            return (Result) { .status = UNITS_DONT_MATCH };
        }
        for (int i = 0; i < units.unit_count; i++) {
            if (!UnitEquals(units.units[i], num.derived_unit.units[i]) || units.exponents[i] != num.derived_unit.exponents[i])
                return (Result) { .status = UNITS_DONT_MATCH };
        }
    }
    return matched;
}

Result AddUnitless(Number num1, Number num2) {
    int signs_unequal = num1.sign != num2.sign;

    // combine the two bases
    int base_flip_sign = num1.base < num2.base;
    unsigned long long base = 
        signs_unequal ? 
            (base_flip_sign ? num2.base - num1.base : num1.base - num2.base) 
        : num1.base + num2.base;

    // give the two fractions a common denominator
    unsigned long long denominator = LCM(num1.denominator, num2.denominator);
    unsigned long long num1_num = num1.numerator * (denominator/num1.denominator);
    unsigned long long num2_num = num2.numerator * (denominator/num2.denominator);
    
    // combine the two fractions
    int frac_flip_sign = num1_num < num2_num;
    unsigned long long numerator = signs_unequal ? 
        (frac_flip_sign ? num2_num - num1_num : num1_num - num2_num) :
        num1_num + num2_num;


    int sign = signs_unequal && base_flip_sign ? !num1.sign : num1.sign;

    // if we're doing subtraction
    if (signs_unequal && (base_flip_sign || frac_flip_sign)) { 
        if (base != 0) {
            base -= 1;
            numerator = denominator - numerator;
        } else {
            sign = !num1.sign;
        }
    }

    Number num = (Number) {
        .sign = sign,
        .base = base,
        .numerator = numerator,
        .denominator = denominator
    };
    num = Balance(num);

    return (Result) {.status = SUCCESS, .value = num};
}

Result Add(Number num1, Number num2, Systems systems) {
    Result num2_result = MatchQuantities(num2, num1.derived_unit, systems);
    if (num2_result.status != SUCCESS)
        return num2_result;
    num2 = num2_result.value;

    Result result = AddUnitless(num1, num2);
    if (result.status != SUCCESS)
        return result;

    result.value.derived_unit = num1.derived_unit.unit_count == 0 ? num2.derived_unit : num1.derived_unit;
    return result;
}

Result Subtract(Number num1, Number num2, Systems systems) {
    Number num2_flipped = num2;
    num2_flipped.sign = !num2_flipped.sign;
    return Add(num1, num2_flipped, systems);
}

// apply the exponents from num2 to num1, as if it were multiplied
DerivedUnit ApplyExponents(DerivedUnit d1, DerivedUnit d2) {
    for (int i = 0; i < d2.unit_count; i++) {
        int found = 0;
        for (int j = 0; j < d1.unit_count; j++) {
            if (UnitEquals(d1.units[j], d2.units[i])) {
                d1.exponents[j] += d2.exponents[i];
                found = 1;
                break;
            }
        }
        if (!found) {
            d1.units[d1.unit_count] = d2.units[i];
            d1.exponents[d1.unit_count] = d2.exponents[i];
            d1.unit_count++;
        }
    }
    return d1;
}

Result MultiplyUnitless(Number num1, Number num2) {
    // (b1 + n1/d1) * (b2 + n2/d2)
    // ((b1*b2) + (b1*n2)/d2) + (b2*n1)/d1 + (n1*n2)/(d1*d2)
    Number p1 = (Number) {
        .base = num1.base * num2.base,
        .numerator = num1.base * num2.numerator,
        .denominator = num2.denominator
    };
    Number p2 = (Number) {
        .base = 0,
        .numerator = num1.numerator * num2.base,
        .denominator = num1.denominator
    };
    unsigned long long gcd1 = GCD(num2.numerator, num1.denominator);
    unsigned long long gcd2 = GCD(num1.numerator, num2.denominator);
    Number p3 = (Number) {
        .base = 0,
        .numerator = (num2.numerator/gcd1) * (num1.numerator/gcd2),
        .denominator = (num1.denominator/gcd1) * (num2.denominator/gcd2)
    };
    p1 = Balance(p1);
    p2 = Balance(p2);
    p3 = Balance(p3);
    Result inter1 = AddUnitless(p1, p2);
    if (inter1.status != SUCCESS) {
        return inter1;
    }
    Result result = AddUnitless(inter1.value, p3);
    if (result.status != SUCCESS) {
        return result;
    }
    result.value.sign = num1.sign ^ num2.sign;
    return result;
}

Result Multiply(Number num1, Number num2, Systems systems) {
    Result num2_result = MatchUnits(num2, num1.derived_unit, systems);
    if (num2_result.status != SUCCESS)
        return num2_result;
    num2 = num2_result.value;

    Result result = MultiplyUnitless(num1, num2);
    if (result.status != SUCCESS) {
        return result;
    }
    result.value.derived_unit = ApplyExponents(num1.derived_unit, num2.derived_unit);
    return result;
}

Result Divide(Number num1, Number num2, Systems systems) {
    // 1 / (b2 + n2/d2)
    // 1 / (b2*d2/d2 + n2/d2)
    // 1 / ((b2*d2+n2)/d2)
    // d2 / (b2*d2+n2)

    Number reciprocal = (Number) {
        .sign = num2.sign,
        .numerator = num2.denominator,
        .denominator = num2.base * num2.denominator + num2.numerator,
        .derived_unit = num2.derived_unit
    };
    reciprocal = Balance(reciprocal);
    for (int i = 0; i < reciprocal.derived_unit.unit_count; i++) {
        reciprocal.derived_unit.exponents[i] = -reciprocal.derived_unit.exponents[i];
    }
    return Multiply(num1, reciprocal, systems);
}


Result AddResult(Result num1, Result num2, Systems systems) {
    if (num1.status != SUCCESS)
        return num1;
    if (num2.status != SUCCESS)
        return num2;
    return Add(num1.value, num2.value, systems);
}

Result SubtractResult(Result num1, Result num2, Systems systems) {
    if (num1.status != SUCCESS)
        return num1;
    if (num2.status != SUCCESS)
        return num2;
    return Subtract(num1.value, num2.value, systems);
}

Result MultiplyResult(Result num1, Result num2, Systems systems) {
    if (num1.status != SUCCESS)
        return num1;
    if (num2.status != SUCCESS)
        return num2;
    return Multiply(num1.value, num2.value, systems);
}

Result DivideResult(Result num1, Result num2, Systems systems) {
    if (num1.status != SUCCESS)
        return num1;
    if (num2.status != SUCCESS)
        return num2;
    return Divide(num1.value, num2.value, systems);
}