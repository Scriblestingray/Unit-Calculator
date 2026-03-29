#include "structs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Systems CreateSystems()
{
    Systems systems;
    systems.systems = malloc(10 * sizeof(UnitSystem));
    memset(systems.systems, 0, 10 * sizeof(UnitSystem));
    systems.count = 0;

    char buf[10] = "";
    char buf2[10];
    char buf3[10];
    int dec;
    int dec2;
    int dec3;
    int dec4;
    FILE *fileptr = fopen("Config.txt", "r");

    int i = 0;
    int j = 0;

    // get first line of the file
    //  fscanf(fileptr, "%s %d %s %d %d %d %s", buf, &dec, buf2, &dec2, &dec3, &dec4, buf3);

    // if its not END, create a system
    while (strcmp(buf, "END") != 0)
    {
        // title of system
        fscanf(fileptr, "%s", buf);
        char *name = malloc(10 * sizeof(char));
        strcpy(name, buf);
        systems.systems[j].name = name;

        if (strcmp(buf, "END") == 0)
        {
            break;
        }

        i = 0;

        systems.systems[j].units = malloc(20 * sizeof(Unit));
        memset(systems.systems[j].units, 0, 20 * sizeof(Unit));

        fscanf(fileptr, "%s %d %s %d %d %d %s", buf, &dec, buf2, &dec2, &dec3, &dec4, buf3);

        // if - not next line, add the next unit
        while (strcmp(buf, "-") != 0)
        {
            char *abbr = malloc(10 * sizeof(char));
            strcpy(abbr, buf);
            systems.systems[j].units[i].abbreviation = abbr;
            systems.systems[j].units[i].is_base_unit = dec;

            // if base unit is true, just make itself the base unit
            if (systems.systems[j].units[i].is_base_unit == 1)
            {
                systems.systems[j].units[i].base_unit = &systems.systems[j].units[i];
            }

            // if its not, search through previous units for its base unit
            else
            {
                int found = 0;
                for (int l = 0; l < systems.count + 1; l++)
                {
                    for (int k = 0; k < systems.systems[l].unit_count; k++)
                    {
                        if (strcmp(systems.systems[l].units[k].abbreviation, buf2) == 0)
                        {
                            systems.systems[j].units[i].base_unit = &systems.systems[l].units[k];
                            found = 1;
                            break;
                        }
                    }
                    if (found)
                    {
                        break;
                    }
                }
            }
            systems.systems[j].units[i].multiplier = dec2;
            systems.systems[j].units[i].divider = dec3;

            char *quantity = malloc(10 * sizeof(char));
            strcpy(quantity, buf3);
            systems.systems[j].units[i].quantity = quantity;

            i++;
            systems.systems[j].unit_count += 1;

            fscanf(fileptr, "%s %d %s %d %d %d %s", buf, &dec, buf2, &dec2, &dec3, &dec4, buf3);
        }
        j++;
        systems.count++;
    }
    fclose(fileptr);

    return systems;
}