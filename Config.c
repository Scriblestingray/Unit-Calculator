#include "structs.h"

struct UnitSystem CreateSystem() {
    struct UnitSystem sys;
    FILE* fileptr = fopen("Config.txt", "r");
    fscanf(fileptr, "%s");
    fscanf(fileptr, "%s", sys.name);
    while (fscanf(fileptr, "%s") != "-") {
        fscanf(fileptr, "%s %s %s %s %s %s", sys.name);
    }
    fclose(fileptr);
    return sys;
}
