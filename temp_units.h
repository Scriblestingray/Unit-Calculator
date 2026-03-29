#pragma once

#include "structs.h"

// hardcoding some units for now, in order to test the system.

static Unit time_units[] = {
    (Unit){
        .abbreviation = "s",
        .is_base_unit = 1,
        .quantity = "time",
    },
    (Unit){
        .abbreviation = "min",
        .is_base_unit = 0,
        .quantity = "time",
        .base_unit = &time_units[0],
        .multiplier = 60,
        .offset = 0,
    },
    (Unit){
        .abbreviation = "hr",
        .is_base_unit = 0,
        .quantity = "time",
        .base_unit = &time_units[1],
        .multiplier = 60,
        .offset = 0,
    },
    (Unit){
        .abbreviation = "dy",
        .is_base_unit = 0,
        .quantity = "time",
        .base_unit = &time_units[2],
        .multiplier = 24,
        .offset = 0,
    }};

static Unit imperial_units[] = {
    (Unit){
        .abbreviation = "in",
        .is_base_unit = 1,
        .quantity = "distance",
    },
    (Unit){
        .abbreviation = "ft",
        .is_base_unit = 0,
        .quantity = "distance",
        .base_unit = &imperial_units[0],
        .multiplier = 12,
        .offset = 0,
    },
    (Unit){
        .abbreviation = "mi",
        .is_base_unit = 0,
        .quantity = "distance",
        .base_unit = &imperial_units[1],
        .multiplier = 5280,
        .offset = 0,
    },
    (Unit){
        .abbreviation = "F",
        .is_base_unit = 1,
        .quantity = "temperature",
    },
};

static Unit metric_units[] = {
    (Unit){
        .abbreviation = "mm",
        .is_base_unit = 0,
        .quantity = "distance",
        .base_unit = &imperial_units[0],
        .multiplier = 10,
        .divider = 254,
        .offset = 0,
    },
    (Unit){
        .abbreviation = "cm",
        .is_base_unit = 0,
        .quantity = "distance",
        .base_unit = &metric_units[0],
        .multiplier = 10,
        .offset = 0,
    },
    (Unit){
        .abbreviation = "m",
        .is_base_unit = 0,
        .quantity = "distance",
        .base_unit = &metric_units[1],
        .multiplier = 100,
        .offset = 0,
    },
    (Unit){
        .abbreviation = "km",
        .is_base_unit = 0,
        .quantity = "distance",
        .base_unit = &metric_units[2],
        .multiplier = 1000,
        .offset = 0,
    },
    (Unit){
        .abbreviation = "˚F",
        .is_base_unit = 0,
        .quantity = "temperature",
        .base_unit = &imperial_units[3],
        .multiplier = 9,
        .divider = 5,
        .offset = 32,
    },
};

static UnitSystem unit_systems[] = {
    (UnitSystem){
        .name = "time",
        .units = time_units,
        .unit_count = sizeof(time_units) / sizeof(Unit)},
    (UnitSystem){
        .name = "imperial",
        .units = imperial_units,
        .unit_count = sizeof(imperial_units) / sizeof(Unit)},
    (UnitSystem){
        .name = "metric",
        .units = metric_units,
        .unit_count = sizeof(metric_units) / sizeof(Unit)}};

static int unit_system_count = 3;