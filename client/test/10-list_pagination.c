#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "control_chain.h"
#include "assignment.h"
#include "utils.h"
#include <string.h>

//Duo
#define SERIAL_PORT            "/dev/ttyS1"
//DuoX
//#define SERIAL_PORT         "/dev/ttymxc0"
#define SERIAL_BAUDRATE     115200

int no_device = 1;
int dev_id = -1;

void dev_desc(void *arg)
{
    cc_device_t *device = arg;
    dev_id = device->id;
    printf("device %s connected\n", device->label->text);
    no_device = 0;
}

void data_update(void *arg)
{
    cc_update_list_t *updates = arg;
    printf("*** received %i updates\n", updates->count);

    for (int i = 0; i < updates->count; ++i)
    {
        cc_update_data_t *data = &updates->list[i];
        printf("id = %i, value = %f\n", data->assignment_id, data->value);
    }
}

// String concatenation
 char* ConcatenateStrings(const char* s1, const char* s2)
{
    const size_t s1Length = strlen(s1);
    const size_t s2Length = strlen(s2);
    // +1 for the null-terminator
    char* result = (char*)malloc(s1Length + s2Length + 1);
    memcpy(result, s1, s1Length);
    // +1 to copy the null-terminator
    memcpy(result + s1Length, s2, s2Length + 1);
    return result;
}

int main(void)
{
    cc_handle_t *handle = cc_init(SERIAL_PORT, SERIAL_BAUDRATE);
    if (!handle)
    {
        printf("can't initiate control chain using %s\n", SERIAL_PORT);
        exit(1);
    }

    cc_device_status_cb(handle, dev_desc);
    cc_data_update_cb(handle, data_update);

    printf("waiting device descriptor\n");
    while (no_device)
    {
    	sleep(1);
    }  

    // List size & limit
    const int list_count = 100;
    const int list_limit = 1000;
    
    cc_item_t items [list_count];


 for (int i = 0; i < list_count && i < list_limit ; i++)
    {
        // Has to be big enough so it can't overflow, in this case 1000 so 5 bytes is enough. (we need one for the null-terminator)
        char number_str[5];
        sprintf(number_str, "%d", (i + 1));

        items[i].label = ConcatenateStrings("option ", (const char*)number_str);
        printf("label_name: %s\n", items[i].label);
        // Save the item value
        items[i].value = (float)(i + 1);
        printf("label_value: %f\n", items[i].value);
    }

    cc_item_t **list_items = malloc(sizeof(cc_item_t *) * list_count);

    for (int i = 0; i < list_count; ++i)
    {
        list_items[i] = &items[i];
    }

 	int assign_id = 0;

    // assignment id, device_id, actuator_id, label, value, min, max, def, mode, steps, unit,
    // list_count, list_items
    cc_assignment_t ass_1 = {assign_id, dev_id, 0, "List", 1.0, 1.0, 8.0, 10.0, 260, 25, "-",list_count, list_items};

    printf("assigning %i\n", assign_id);

    int id_1 = cc_assignment(handle, &ass_1);

    if (id_1 < 0)
    {
        printf("error in assignment %i\n", id_1);
    }

    // assignment of Static LED list
    assign_id = 1;

    // assignment id, device_id, actuator_id, label, value, min, max, def, mode, steps, unit,
    // list_count, list_items
    
    cc_assignment_t ass_2 = {assign_id, dev_id, 1, "List", 1.0, 1.0, 8.0, 10.0, 4, 8, "-",list_count, list_items};

    printf("assigning %i\n", assign_id);

    int id_2 = cc_assignment(handle, &ass_2);

    if (id_2 < 0)
    {
        printf("error in assignment %i\n", id_2);
    }

    // Free up memory
    for (int i = 0; i < list_count; i++)
    {
        free((char*)items[i].label);
    }

    //give some time to test the actuatots
    sleep(60);

    //unassign
    printf("removing assignment %i\n", id_1);
    cc_assignment_key_t key_1 = {id_1, dev_id};
    cc_unassignment(handle, &key_1);
    sleep(1);

    printf("removing assignment %i\n", id_2);
    cc_assignment_key_t key_2 = {id_2, dev_id};
    cc_unassignment(handle, &key_2);
    sleep(1);

    // assignment of LED cycling 
    cc_finish(handle);

    return 0;
}

