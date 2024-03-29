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
#include <jansson.h>
#include "device.h"


/*
****************************************************************************************************
*       INTERNAL MACROS
****************************************************************************************************
*/

// device status
enum {DEV_WAITING_HANDSHAKE, DEV_WAITING_DESCRIPTOR, DEV_WAITING_ASSIGNMENT};


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

static cc_device_t g_devices[CC_MAX_DEVICES];


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

cc_device_t* cc_device_create(cc_handshake_dev_t *handshake)
{
    for (int i = 0; i < CC_MAX_DEVICES; i++)
    {
        if (g_devices[i].id == 0)
        {
            // delete possible old data
            memset(&g_devices[i], 0, sizeof(cc_device_t));

            // device id cannot be zero
            g_devices[i].id = i + 1;

            // store handshake info
            g_devices[i].protocol.major = handshake->protocol.major;
            g_devices[i].protocol.minor = handshake->protocol.minor;
            g_devices[i].firmware.major = handshake->firmware.major;
            g_devices[i].firmware.minor = handshake->firmware.minor;
            g_devices[i].firmware.micro = handshake->firmware.micro;

            // only for version before v0.4
            if (handshake->uri)
                g_devices[i].uri = handshake->uri;

            return &g_devices[i];
        }
    }

    return NULL;
}

void cc_device_destroy(int device_id)
{
    cc_device_t *device = cc_device_get(device_id);

    if (!device)
        return;

    // destroy URI and label
    string_destroy(device->uri);
    string_destroy(device->label);
    device->uri = device->label = NULL;

    // destroy actuators
    if (device->actuators)
    {
        for (int i = 0; i < device->actuators_count * device->amount_of_pages; i++)
        {
            if (device->actuators[i])
            {
                string_destroy(device->actuators[i]->name);
                free(device->actuators[i]);
            }
        }
        free(device->actuators);
        device->actuators = NULL;
    }

    // destroy actuator groups
    if (device->actuatorgroups)
    {
        for (int i = 0; i < device->actuatorgroups_count * device->amount_of_pages; i++)
        {
            if (device->actuatorgroups[i])
            {
                string_destroy(device->actuatorgroups[i]->name);
                free(device->actuatorgroups[i]);
            }
        }
        free(device->actuatorgroups);
        device->actuatorgroups = NULL;
    }

    // destroy assigments list
    if (device->assignments)
    {
        for (int i = 0; i < CC_MAX_ASSIGNMENTS; i++)
        {
            if (device->assignments[i])
                cc_assignment_free(device->assignments[i]);
        }
        free(device->assignments);
        device->assignments = NULL;
    }

    // reset status and id
    device->status = CC_DEVICE_DISCONNECTED;
    device->id = 0;
}

char* cc_device_descriptor(int device_id)
{
    cc_device_t* device = cc_device_get(device_id);

    if (!device)
        return NULL;

    json_t *root = json_object();

    // label
    json_t *label = json_stringn(device->label->text, device->label->size);
    json_object_set_new(root, "label", label);

    // uri
    json_t *uri = json_stringn(device->uri->text, device->uri->size);
    json_object_set_new(root, "uri", uri);

    // firmware version
    char buffer[16];
    snprintf(buffer, sizeof(buffer)-1, "%i.%i.%i",
        device->firmware.major, device->firmware.minor, device->firmware.micro);
    buffer[sizeof(buffer)-1] = 0;
    json_t *version = json_stringn(buffer, strlen(buffer));
    json_object_set_new(root, "version", version);

    // protocol version
    snprintf(buffer, sizeof(buffer)-1, "%i.%i",
        device->protocol.major, device->protocol.minor);
    buffer[sizeof(buffer)-1] = 0;
    json_t *protocol = json_stringn(buffer, strlen(buffer));
    json_object_set_new(root, "protocol", protocol);

    // chain id
    if (device->chain_id != 0)
    {
        json_t *chain_id = json_integer(device->chain_id);
        json_object_set_new(root, "chain_id", chain_id);
    }

    // actuators
    json_t *json_actuators = json_array();
    json_object_set_new(root, "actuators", json_actuators);

    // populate actuators list
    for (int i = 0; i < device->actuators_count; i++)
    {
        cc_actuator_t *actuator = device->actuators[i];
        json_t *json_actuator = json_object();

        // actuator id
        json_t *id = json_integer(actuator->id);
        json_object_set_new(json_actuator, "id", id);

        // actuator name
        json_t *name = json_stringn(actuator->name->text, actuator->name->size);
        json_object_set_new(json_actuator, "name", name);

        // actuator supported modes
        json_t *supported_modes = json_integer(actuator->supported_modes);
        json_object_set_new(json_actuator, "supported_modes", supported_modes);

        // actuator maximum assignments
        json_t *max_assignments = json_integer(actuator->max_assignments);
        json_object_set_new(json_actuator, "max_assignments", max_assignments);

        // add to list
        json_array_append_new(json_actuators, json_actuator);
    }

    // actuator groups
    json_t *json_actuatorgroups = json_array();
    json_object_set_new(root, "actuatorgroups", json_actuatorgroups);

    // populate actuator groups list
    for (int i = 0; i < device->actuatorgroups_count; i++)
    {
        cc_actuatorgroup_t *actuatorgroup = device->actuatorgroups[i];
        json_t *json_actuatorgroup = json_object();

        // actuator group id
        json_t *id = json_integer(actuatorgroup->id);
        json_object_set_new(json_actuatorgroup, "id", id);

        // actuator group name
        json_t *name = json_stringn(actuatorgroup->name->text, actuatorgroup->name->size);
        json_object_set_new(json_actuatorgroup, "name", name);

        // actuator group actuators #1
        json_t *actuator1 = json_integer(actuatorgroup->actuators_in_actuatorgroup[0]);
        json_object_set_new(json_actuatorgroup, "actuator1", actuator1);

        // actuator group actuators #2
        json_t *actuator2 = json_integer(actuatorgroup->actuators_in_actuatorgroup[1]);
        json_object_set_new(json_actuatorgroup, "actuator2", actuator2);

        // add to list
        json_array_append_new(json_actuatorgroups, json_actuatorgroup);
    }

    // check if we have pages and init those too
    if (device->amount_of_pages > 1)
    {
        int page_actuator_id = device->actuators_count;
        int page_actuatorgroup_id = device->actuatorgroups_count;

        for (int j = 1; j < device->amount_of_pages; j++)
        {
            for (int i = 0; i < device->actuators_count; i++)
            {
                cc_actuator_t *actuator = device->actuators[page_actuator_id++];
                json_t *json_actuator = json_object();

                // actuator id
                json_t *id = json_integer(actuator->id);
                json_object_set_new(json_actuator, "id", id);

                // actuator name
                json_t *name = json_stringn(actuator->name->text, actuator->name->size);
                json_object_set_new(json_actuator, "name", name);

                // actuator supported modes
                json_t *supported_modes = json_integer(actuator->supported_modes);
                json_object_set_new(json_actuator, "supported_modes", supported_modes);

                // actuator maximum assignments
                json_t *max_assignments = json_integer(actuator->max_assignments);
                json_object_set_new(json_actuator, "max_assignments", max_assignments);

                // add to list
                json_array_append_new(json_actuators, json_actuator);
            }

            // populate actuator groups list
            for (int i = 0; i < device->actuatorgroups_count; i++)
            {
                cc_actuatorgroup_t *actuatorgroup = device->actuatorgroups[page_actuatorgroup_id++];
                json_t *json_actuatorgroup = json_object();

                // actuator group id
                json_t *id = json_integer(actuatorgroup->id);
                json_object_set_new(json_actuatorgroup, "id", id);

                // actuator group name
                json_t *name = json_stringn(actuatorgroup->name->text, actuatorgroup->name->size);
                json_object_set_new(json_actuatorgroup, "name", name);

                // actuator group actuators #1
                json_t *actuator1 = json_integer(actuatorgroup->actuators_in_actuatorgroup[0]);
                json_object_set_new(json_actuatorgroup, "actuator1", actuator1);

                // actuator group actuators #2
                json_t *actuator2 = json_integer(actuatorgroup->actuators_in_actuatorgroup[1]);
                json_object_set_new(json_actuatorgroup, "actuator2", actuator2);

                // add to list
                json_array_append_new(json_actuatorgroups, json_actuatorgroup);
            }
        }
    }

    char *str = json_dumps(root, 0);

    // free json object
    json_decref(root);

    return str;
}

int* cc_device_list(int filter)
{
    int count = 0;
    int *devices_list = malloc((CC_MAX_DEVICES + 1) * sizeof(int));

    for (int i = 0; i < CC_MAX_DEVICES; i++)
    {
        if (!g_devices[i].id)
            continue;

        if (filter == CC_DEVICE_LIST_ALL ||
           (filter == CC_DEVICE_LIST_REGISTERED && g_devices[i].label) ||
           (filter == CC_DEVICE_LIST_UNREGISTERED && !g_devices[i].label))
        {
            devices_list[count++] = g_devices[i].id;
        }
    }

    devices_list[count] = 0;
    return devices_list;
}

int cc_device_count(const char *uri)
{
    int count = 0;

    for (int i = 0; i < CC_MAX_DEVICES; i++)
    {
        if (!g_devices[i].id || g_devices[i].status == CC_DEVICE_DISCONNECTED)
            continue;

        if (g_devices[i].uri && strcmp(uri, g_devices[i].uri->text) == 0)
            count++;
    }

    return count;
}

cc_device_t* cc_device_get(int device_id)
{
    if (device_id)
    {
        for (int i = 0; i < CC_MAX_DEVICES; i++)
        {
            if (g_devices[i].id == device_id)
                return &g_devices[i];
        }
    }

    return NULL;
}
