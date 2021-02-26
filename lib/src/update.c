/*
 * This file is part of the control chain project
 *
 * Copyright (C) 2016 Ricardo Crudo <ricardo.crudo@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
****************************************************************************************************
*       INCLUDE FILES
****************************************************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include "update.h"
#include "assignment.h"
#include "device.h"
#include "utils.h"
#include <stdio.h>


/*
****************************************************************************************************
*       INTERNAL MACROS
****************************************************************************************************
*/
#define DEBUG_MSG(...)      do { if (g_debug) fprintf(stderr, "[cc-lib] " __VA_ARGS__); } while (0)
int g_debug;
/*
****************************************************************************************************
*       INTERNAL CONSTANTS
****************************************************************************************************
*/


/*
****************************************************************************************************
*       INTERNAL DATA TYPES
****************************************************************************************************
*/


/*
****************************************************************************************************
*       INTERNAL GLOBAL VARIABLES
****************************************************************************************************
*/


/*
****************************************************************************************************
*       INTERNAL FUNCTIONS
****************************************************************************************************
*/


/*
****************************************************************************************************
*       GLOBAL FUNCTIONS
****************************************************************************************************
*/

cc_update_list_t *cc_update_parse(int device_id, uint8_t *raw_data, int check_assignments)
{
    const int update_data_size = (sizeof(float) + 1);
    int count = raw_data[0];

    // create and initialize update list
    cc_update_list_t *updates = malloc(sizeof(cc_update_list_t));
    updates->count = 0;
    updates->device_id = device_id;
    updates->list = malloc(sizeof(cc_update_data_t) * count);
    updates->raw_data = malloc(update_data_size * count + 1);

    // parse data to struct
    int j = 1, k = 1;
    for (int i = 0; i < count; i++)
    {
        cc_assignment_key_t assignment;
        assignment.device_id = device_id;

        //if actuator is grouped, and second actuator, change assgnment ID to the first one
        cc_device_t *device = cc_device_get(device_id);
        DEBUG_MSG(" Device id retrieved by update parse: %i\n", device->id);

        int actuator_id = device->assignments[raw_data[j]]->actuator_id;
        DEBUG_MSG(" Current actuator ID is: %i\n", actuator_id);
        if (device->actuators[actuator_id]->grouped)
        {
            DEBUG_MSG(" Actuator is grouped indeed \n");
            for (int i = 0; i < device->actuatorgroups_count; i++)
            {
                if (device->actuatorgroups[i]->actuators_in_actuatorgroup[1] == actuator_id)
                {
                    DEBUG_MSG(" Found actuator in group %i\n", i);
                    //retrieve other assignment ID
                    for (int q = 0; device->assignments[q]; q++)
                    {
                        if (device->assignments[q]->actuator_id == device->actuatorgroups[i]->actuators_in_actuatorgroup[0])
                        {
                            DEBUG_MSG(" Found group assignment with ID %i\n", q);
                            raw_data[j] = q;
                            continue;
                        }
                    }
                }
                else if (device->actuatorgroups[i]->actuators_in_actuatorgroup[0] == actuator_id)
                    continue;
            }
        }

        assignment.id = raw_data[j];

        if (check_assignments == 0 || cc_assignment_check(&assignment))
        {
            cc_update_data_t *data = &updates->list[i];

            // update id
            data->assignment_id = assignment.id;

            // update value
            float *value = (float *) &raw_data[j + 1];
            data->value = *value;

            // copy raw data
            memcpy(&updates->raw_data[k], &raw_data[j], update_data_size);
            k += update_data_size;

            updates->count++;
        }

        // increment by update data size
        j += update_data_size;
    }

    // update count and size of raw_data
    updates->raw_data[0] = updates->count;
    updates->raw_size = k;

    return updates;
}

void cc_update_free(cc_update_list_t *updates)
{
    free(updates->list);
    free(updates->raw_data);
    free(updates);
}
