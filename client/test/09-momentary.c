#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "control_chain.h"
#include "assignment.h"

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
    while (no_device) sleep(1);


    uint8_t list_bitmask = 0;
    //assigning 4 different tap types


    //tap 1, bpm
    // assignment id, device_id, actuator_id, label, value, min, max, def, mode, steps, unit, list_count, list bitmask, list_items
    int assign_id = 1;
    cc_assignment_t ass_1 = {assign_id, dev_id, 0, "Momentary", 0, 0, 1, 10.0, 128, 0, "-",0,list_bitmask, NULL};

    printf("assigning %i\n", assign_id);

    int id_1 = cc_assignment(handle, &ass_1);

    if (id_1 < 0)
    {
        printf("error in assignment %i\n", id_1);
    }

    //give some time to test the actuatots
    sleep(60);

    //unassign
    printf("removing assignment %i\n", id_1);
    cc_assignment_key_t key_1 = {id_1, dev_id};
    cc_unassignment(handle, &key_1);
    sleep(1);


    cc_finish(handle);

    return 0;
}

