#include "structs.h"

struct UnitSystem CreateSystem() {
    struct UnitSystem sys;
    FILE* fileptr = fopen("Config.txt", "r");
    fscanf(fileptr, "%s");
    fscanf(fileptr, "%s", sys.name);
    int i = 0;
    while (fscanf(fileptr, "%s") != "-") {
        fscanf(fileptr, "%s %d %s %d %d %s", sys.units[i].abbreviation, sys.units[i].is_base_unit, sys.units[i].base_unit, sys.units[i].multiplier, sys.units[i].offset, sys.units[i].quantity);
        i++;
    }
    fclose(fileptr);
    return sys;
}
