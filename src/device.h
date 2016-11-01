#ifndef CC_DEVICE_H
#define CC_DEVICE_H


/*
****************************************************************************************************
*       INCLUDE FILES
****************************************************************************************************
*/

#include "utils.h"


/*
****************************************************************************************************
*       MACROS
****************************************************************************************************
*/


/*
****************************************************************************************************
*       CONFIGURATION
****************************************************************************************************
*/

#define CC_MAX_DEVICES  8


/*
****************************************************************************************************
*       DATA TYPES
****************************************************************************************************
*/

// enumeration to filter list of devices
// REGISTERED means that the device already received the device descriptor
// UNREGISTERED means that the device still waiting for the device descriptor
enum {CC_DEVICE_LIST_ALL, CC_DEVICE_LIST_REGISTERED, CC_DEVICE_LIST_UNREGISTERED};

typedef struct cc_actuator_t {
    int id;
} cc_actuator_t;

typedef struct cc_dev_descriptor_t {
    string_t *label;
    int actuators_count;
    cc_actuator_t **actuators;
} cc_dev_descriptor_t;

typedef struct device_t {
    int id;
    cc_dev_descriptor_t *descriptor;
} device_t;


/*
****************************************************************************************************
*       FUNCTION PROTOTYPES
****************************************************************************************************
*/

// create a device using device_id
void cc_device_create(int device_id);

// destroy the device
void cc_device_destroy(int device_id);

// set the device descriptor
void cc_device_descriptor(int device_id, cc_dev_descriptor_t *descriptor);

// return a NULL terminated devices list according the requested filter
device_t** cc_device_list(int filter);


/*
****************************************************************************************************
*       CONFIGURATION ERRORS
****************************************************************************************************
*/


#endif
