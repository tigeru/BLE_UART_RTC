
#ifndef __D_FDS_H__
#define __D_FDS_H__

#include <stdint.h>
#include <stdbool.h>
#include "app_error.h"
#include "time.h"


#include "app_scheduler.h"
#include "nrf_calendar.h"
#include "fds.h"
#include "app_scheduler.h"


#include "app_timer.h"
#include "bsp.h"
#include "bsp_config.h"
#include "fds_example.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define D_FILE_ID 					0x0001
#define D_RECORD_KEY_OFFSET 0x1121
#define D_DATA_KEY_OFFSET 	0x11A1

/* Array to map FDS events to strings. */
static char const * fds_evt_str[] =
{
    "FDS_EVT_INIT",
    "FDS_EVT_WRITE",
    "FDS_EVT_UPDATE",
    "FDS_EVT_DEL_RECORD",
    "FDS_EVT_DEL_FILE",
    "FDS_EVT_GC",
};


// Add Duc's code
/* Keep track of the progress of a delete_all operation. */
static struct
{
    bool delete_next;   //!< Delete next record.
    bool pending;       //!< Waiting for an fds FDS_EVT_DEL_RECORD event, to delete the next record.
} m_delete_all;

/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;

/* Flag to check fds writing completed. */
static bool volatile md_fds_written = false;
static bool volatile md_fds_updated = false;

const char *fds_err_str(ret_code_t ret);
static void power_manage(void);
void wait_for_fds_ready(void);
void wait_for_writing(void);
void wait_for_updating(void);

ret_code_t d_fds_init(void);



#endif
