#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "control_chain.h"
#include "assignment.h"

#define SERIAL_PORT         "/dev/ttyACM0"
#define SERIAL_BAUDRATE     115200

int no_device = 1;
int dev_id;

void dev_desc(void *arg)
{
    cc_dev_descriptor_t *desc = arg;
    printf("device id = %i, name = %s plugged in\n", desc->id, desc->label->text);
    dev_id = desc->id;
    no_device = 0;
}

int main(void)
{
    cc_handle_t *handle = cc_init(SERIAL_PORT, SERIAL_BAUDRATE);
    if (!handle)
    {
        printf("can't initiate control chain using %s\n", SERIAL_PORT);
        exit(1);
    }

    cc_dev_descriptor_cb(handle, dev_desc);

    printf("waiting device descriptor\n");
    while (no_device) sleep(1);

    cc_assignment_t ass = {1, 0, 1.0, 0.0, 1.0, 0.0, 1};
    int ass_id = cc_assignment(handle, &ass);
    cc_unassignment(handle, ass_id);

    cc_finish(handle);

    return 0;
}
