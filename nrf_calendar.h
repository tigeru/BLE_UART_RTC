/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef __NRF_CALENDAR_H__
#define __NRF_CALENDAR_H__

#include <stdint.h>
#include <stdbool.h>
#include "app_error.h"
#include "time.h"

// Add Duc's code

#include "app_scheduler.h"

#include "app_timer.h"
#include "bsp.h"
#include "bsp_config.h"

#include "d_fds.h"
// End Duc's code

// Change the following defines to change the RTC timer used or the interrupt priority
// Add Duc's code: Change RTC0 > RTC2
#define CAL_RTC                 NRF_RTC2
#define CAL_RTC_IRQn            RTC2_IRQn
#define CAL_RTC_IRQHandler      RTC2_IRQHandler
#define CAL_RTC_IRQ_Priority    3
// End Duc's code
#define D_MAX_SLOT							48 
#define D_MODE_BLANK						0 
#define D_MODE_OFF              2
#define D_MODE_ON              	1

#define D_LESS		              0
#define D_SAME		              1
#define D_MORE		              2


// Add Duc's code
static bool volatile d_alarm_update = false;

// End Duc's code

// Add Duc's code

typedef struct
{
	time_t d_time;
	uint32_t d_mode;			// Mode to control at that time
} d_stamp_time_t;

static d_stamp_time_t d_stamp_data[D_MAX_SLOT];
// End Duc's code

// Initializes the calendar library. Run this before calling any other functions. 
void nrf_cal_init(void);

// Add Duc's code
void d_export_data(void);
ret_code_t d_app_rtc_init(void);
ret_code_t d_nrf_cal_set_alarm(uint8_t hour, uint8_t minute, uint8_t second, uint32_t d_mode );

// Write the alarm data to flash memory
ret_code_t d_write_data(d_stamp_time_t *d_data_pointer);
ret_code_t d_read_data(void);

void d_check_update(void);
	
// End Duc's code

// Enables a callback feature in the calendar library that can call a function automatically at the specified interval (seconds).
void nrf_cal_set_callback(void (*callback)(void), uint32_t interval);

// Sets the date and time stored in the calendar library. 
// When this function is called a second time calibration data will be automatically generated based on the error in time since the
// last call to the set time function. To ensure good calibration this function should not be called too often 
// (depending on the accuracy of the 32 kHz clock it should be sufficient to call it between once a week and once a month). 
void nrf_cal_set_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second);

// Returns the uncalibrated time as a tm struct. For more information about the tm struct and the time.h library in general please refer to:
// http://www.tutorialspoint.com/c_standard_library/time_h.htm
struct tm *nrf_cal_get_time(void);

// Returns the calibrated time as a tm struct. If no calibration data is available it will return the uncalibrated time.
struct tm *nrf_cal_get_time_calibrated(void);

// Returns a string for printing the date and time. Turn the calibration on/off by setting the calibrate parameter. 
char *nrf_cal_get_time_string(bool calibrated);

#endif
