#include "structs.h"
#include <stdio.h>
#include <string.h>

typedef struct Systems Systems;
struct Systems
{
    UnitSystem *systems;
    int count;
};

Systems CreateSystems() {
    Systems systems;
    systems.systems = malloc(10 * sizeof(UnitSystem));
    systems.count = 0;
    
    char buf[10];
    char buf2[10];
    char buf3[10];
    int dec;
    int dec2;
    int dec3;
    int dec4;
    FILE* fileptr = fopen("Config.txt", "r");

    int i = 0;
    int j = 0;
    fscanf(fileptr, "%s %d %s %d %d %d %s", buf, &dec, buf2, &dec2, &dec3, &dec4, buf3);
    while (strcmp(buf, "END") != 0) {
        //title of systen
        fscanf(fileptr, "%s", systems.systems[j].name);
        fscanf(fileptr, "%s %d %s %d %d %d %s", buf, &dec, buf2, &dec2, &dec3, &dec4, buf3);
        while (strcmp(buf, "-") != 0) {
            systems.systems[j].units[i].abbreviation = buf;
            systems.systems[j].units[i].is_base_unit = dec;
            if (systems.systems[j].units[i].is_base_unit == 1) {
                systems.systems[j].units[i].base_unit = &systems.systems[j].units[i];
            }
            else {
                for (int k = 0; k < sizeof(systems.systems[j].units); k++) {
                    if (strcmp(systems.systems[j].units[k].abbreviation, buf2) == 0) {
                        systems.systems[j].units[i].base_unit = &systems.systems[j].units[k];
                    }
                }
            }
            systems.systems[j].units[i].multiplier = dec2;
            systems.systems[j].units[i].divider = dec3;
            systems.systems[j].units[i].quantity = buf3;
            i++;
        }
        systems.systems[j].unit_count = i;
        j++;
    }
    fclose(fileptr);
    return systems;
}