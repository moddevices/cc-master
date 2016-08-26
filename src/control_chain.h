#ifndef CONTROL_CHAIN_H
#define CONTROL_CHAIN_H

/*
************************************************************************************************************************
*       INCLUDE FILES
************************************************************************************************************************
*/

#include <stdint.h>
#include "utils.h"
#include "device.h"
#include "assignment.h"


/*
************************************************************************************************************************
*       MACROS
************************************************************************************************************************
*/

#define CC_MODE_TOGGLE  0x01
#define CC_MODE_TRIGGER 0x02


/*
************************************************************************************************************************
*       CONFIGURATION
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*       DATA TYPES
************************************************************************************************************************
*/

// fields names and sizes in bytes
// DEV_ADDRESS (1), COMMAND (1), DATA_SIZE (2), DATA (N), CHECKSUM (1)

enum cc_cmd_t {CC_CMD_CHAIN_SYNC, CC_CMD_HANDSHAKE, CC_CMD_DEV_DESCRIPTOR, CC_CMD_ASSIGNMENT, CC_CMD_DATA_UPDATE,
               CC_CMD_UNASSIGNMENT};

typedef struct cc_handle_t cc_handle_t;

typedef struct cc_data_t {
    uint8_t assignment_id;
    float value;
} cc_data_t;

typedef struct cc_data_update_t {
    uint8_t count;
    cc_data_t *updates_list;
} cc_data_update_t;


/*
************************************************************************************************************************
*       FUNCTION PROTOTYPES
************************************************************************************************************************
*/

cc_handle_t* cc_init(const char *port_name, int baudrate);
void cc_finish(cc_handle_t *handle);

int cc_assignment(cc_handle_t *handle, cc_assignment_t *assignment);
void cc_unassignment(cc_handle_t *handle, int assignment_id);

void cc_data_update_cb(cc_handle_t *handle, void (*callback)(void *arg));
void cc_dev_descriptor_cb(cc_handle_t *handle, void (*callback)(void *arg));


/*
************************************************************************************************************************
*   CONFIGURATION ERRORS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*   END HEADER
************************************************************************************************************************
*/

#endif
