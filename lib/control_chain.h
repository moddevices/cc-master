#ifndef CONTROL_CHAIN_H
#define CONTROL_CHAIN_H

/*
****************************************************************************************************
*       INCLUDE FILES
****************************************************************************************************
*/

#include "core.h"
#include "utils.h"
#include "msg.h"
#include "handshake.h"
#include "device.h"
#include "assignment.h"
#include "update.h"


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


/*
****************************************************************************************************
*       DATA TYPES
****************************************************************************************************
*/


/*
****************************************************************************************************
*       FUNCTION PROTOTYPES
****************************************************************************************************
*/

int cc_assignment(cc_handle_t *handle, cc_assignment_t *assignment);
void cc_unassignment(cc_handle_t *handle, cc_unassignment_t *unassignment);
void cc_data_update_cb(cc_handle_t *handle, void (*callback)(void *arg));
void cc_device_status_cb(cc_handle_t *handle, void (*callback)(void *arg));


/*
****************************************************************************************************
*       CONFIGURATION ERRORS
****************************************************************************************************
*/


#endif
