#include "structs.h"
#include <stdio.h>
#include <string.h>

struct UnitSystem CreateSystem() {
    struct UnitSystem sys;
    char buf[10];
    char buf2[10];
    char buf3[10];
    int dec;
    int dec2;
    int dec3;
    int dec4;
    FILE* fileptr = fopen("Config.txt", "r");

    //title of systen
    fscanf(fileptr, "%s", sys.name);


    int i = 0;
    fscanf(fileptr, "%s %d %s %d %d %d %s", buf, &dec, buf2, &dec2, &dec3, &dec4, buf3);
    while (strcmp(buf, "-") != 0) {
        sys.units[i].abbreviation = buf;
        sys.units[i].is_base_unit = dec;
        sys.units[i].base_unit = buf2; //fix this, not correct type, setting a struct to a string, not good
        sys.units[i].multiplier = dec2;
        sys.units[i].divider = dec3;
        sys.units[i].offset = dec4;
        sys.units[i].quantity = buf3; //also wrong type
        fscanf(fileptr, "%s %d %s %d %d %d %s", buf, &dec, buf2, &dec2, &dec3, &dec4, buf3);
        i++;
    }

    sys.unit_count = i;
    fclose(fileptr);
    return sys;
}
