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
 
#include "nrf_calendar.h"
#include "nrf.h"

// Add Duc's code
APP_TIMER_DEF(d_rtc_tmr);

// End Duc's code

static struct tm time_struct, m_tm_return_time;
static time_t m_time, m_last_calibrate_time = 0;
static float m_calibrate_factor = 0.0f;
static uint32_t m_rtc_increment = 60;
static void (*cal_event_callback)(void) = 0;

// Add Duc's code

time_t d_convert_today(time_t* value_tobe_converted ,time_t* current_time)
{
	struct tm d_tmp_time_struct_1, d_tmp_time_struct_2;
	d_tmp_time_struct_1 = *localtime(value_tobe_converted);
	d_tmp_time_struct_2 = *localtime(current_time);
	
	d_tmp_time_struct_1.tm_year = d_tmp_time_struct_2.tm_year;
	d_tmp_time_struct_1.tm_mon = d_tmp_time_struct_2.tm_mon;
	d_tmp_time_struct_1.tm_mday = d_tmp_time_struct_2.tm_mday;
	
	return mktime(&d_tmp_time_struct_1);
}

// End Duc's code

// Add Duc's code
void d_export_data(void)
{
			struct tm d_tm_return_time;
			for(uint32_t i=0; i<= D_MAX_SLOT;i++)
			{

					if(d_stamp_data[i].d_mode == D_MODE_BLANK ) {NRF_LOG_INFO("CM is 0"); break;}
					//if(d_stamp_data[i].d_mode == D_MODE_BLANK ) break;
				
					d_tm_return_time = *localtime(&(d_stamp_data[i].d_time));
				//NRF_LOG_INFO("DT %d is: %02d:%02d M:%d",i, d_tm_return_time.tm_hour, d_tm_return_time.tm_min, d_stamp_data[i].d_mode);
				//printf("DT %d is: %02d:%02d M:%d",i, d_tm_return_time.tm_hour, d_tm_return_time.tm_min, d_stamp_data[i].d_mode);
			}
}
/* Function to compare 2 time value on HH:MM:SS only
		Return: D_LESS (0) if value_a (hour)< value_b  (hour)
						D_SAME (1) if value_a (hour)= value_b  (hour)
						D_MORE (2) if value_a (hour)> value_b  (hour)
*/		
uint32_t d_hour_compare(time_t* value_a ,time_t* value_b)
{
	struct tm d_tmp_time_struct_a, d_tmp_time_struct_b;
	d_tmp_time_struct_a = *localtime(value_a);
	d_tmp_time_struct_b = *localtime(value_b);
	
	if (d_tmp_time_struct_a.tm_hour < d_tmp_time_struct_b.tm_hour)
		return D_LESS;
	else if(d_tmp_time_struct_a.tm_hour > d_tmp_time_struct_b.tm_hour)
		return D_MORE;
	else if(d_tmp_time_struct_a.tm_min < d_tmp_time_struct_b.tm_min)
		return D_LESS;
	else if(d_tmp_time_struct_a.tm_min > d_tmp_time_struct_b.tm_min)
		return D_MORE;
	else if(d_tmp_time_struct_a.tm_sec < d_tmp_time_struct_b.tm_sec)
		return D_LESS;
	else if(d_tmp_time_struct_a.tm_sec > d_tmp_time_struct_b.tm_sec)
		return D_MORE;
	else return D_SAME;
}
// End Duc's code

// Add Duc's code
ret_code_t d_write_data(d_stamp_time_t *d_data_pointer)
{
	static d_stamp_time_t d_fds_time_data[D_MAX_SLOT] = {0};
	
	memcpy(d_fds_time_data, d_data_pointer, sizeof(d_fds_time_data));
	
	fds_record_t        record;
	fds_record_desc_t   record_desc = {0};
	fds_find_token_t   	d_ftok = {0};
	// Set up record.
	record.file_id           = D_FILE_ID;
	//record.key               = D_DATA_KEY_OFFSET + index;
	record.key               = D_DATA_KEY_OFFSET;
	
	//record.data.p_data       = d_data_pointer + index;
	record.data.p_data       = &d_fds_time_data;
	
	//record.data.length_words = (sizeof(d_stamp_time_t) + 3) / sizeof(uint32_t);   /* one word is four bytes. */
	record.data.length_words = (sizeof(d_fds_time_data) + 3) / sizeof(uint32_t);
	
	//uint32_t *d_p_uint32;
	//d_p_uint32 = (uint32_t *)record.data.p_data;
	//NRF_LOG_INFO("1st word of record data in hex: 0x%08X",*d_p_uint32);
	//NRF_LOG_INFO("2nd word of record data in hex: 0x%08X",*(d_p_uint32 + 1) );
	//NRF_LOG_INFO("Add of record data: 0x%08X",(uint32_t )record.data.p_data);
	
	//NRF_LOG_INFO("Start find function");
	//ret_code_t ret = fds_record_find(D_FILE_ID, D_DATA_KEY_OFFSET + index, &record_desc, &d_ftok);
	
		ret_code_t	d_ret = fds_gc();
		if( d_ret !=NRF_SUCCESS )
		{
			NRF_LOG_INFO("There are some error during garbage collection.");
		}
		
	ret_code_t ret = fds_record_find(D_FILE_ID, D_DATA_KEY_OFFSET, &record_desc, &d_ftok);
	if (ret == NRF_SUCCESS)
	{						
			ret = fds_record_update(&record_desc, &record);
			if (ret == FDS_ERR_NO_SPACE_IN_FLASH)
				NRF_LOG_INFO("N Sp");
				
			NRF_LOG_INFO("Logger configuration file updated with result:%d", ret);
			//printf("URET:%d\r\n", ret);
		
			//wait_for_updating();	
	}
	else if (ret == FDS_ERR_NOT_FOUND)
	{
			ret = fds_record_write(&record_desc, &record);
			NRF_LOG_INFO("Logger configuration file written with result:%d", ret);
		
			// Wait until completed writing
			//wait_for_writing();
	}
	else
	{	
			NRF_LOG_INFO("There are some error.");
			ret = NRF_ERROR_INTERNAL;
			NRF_LOG_INFO("Error:%d", ret);
	}
	
	//NRF_LOG_INFO("Writing completed.");
	return ret;
}

ret_code_t d_read_data(void)
{
	ret_code_t d_ret;
	//d_stamp_time_t d_stamp_data[D_MAX_SLOT] = {0};
	
	fds_record_desc_t   record_desc;
	fds_find_token_t  tok  = {0};
	fds_flash_record_t  flash_record;
	
	/* It is required to zero the token before first use. */
	memset(&tok, 0x00, sizeof(fds_find_token_t));
	
	uint32_t d_find_count = 0;
	while (fds_record_find(D_FILE_ID, D_DATA_KEY_OFFSET, &record_desc, &tok) == NRF_SUCCESS)
	{
			d_find_count++;
			if (d_find_count > 1)
			{
				NRF_LOG_INFO("There are more than 1 record found with D_DATA_KEY_OFFSET ");
				d_ret = NRF_ERROR_INTERNAL;
				NRF_LOG_INFO("Error:%d", d_ret);
				break;
			}
			
			d_ret = fds_record_open(&record_desc, &flash_record) ;
			if (d_ret != NRF_SUCCESS)
			{
				NRF_LOG_INFO("There are some error in record open.");
				d_ret = NRF_ERROR_INTERNAL;
				NRF_LOG_INFO("Error:%d", d_ret);
				break;
			}
			
			/* Access the record through the flash_record structure. */
				memcpy(d_stamp_data, flash_record.p_data, sizeof(d_stamp_data) );

			/* 					Testing read-record data
						//				uint32_t *d_p_uint32;
						//				d_p_uint32 = (uint32_t *)flash_record.p_data;
						//				memcpy(&d_tmp_uint32, flash_record.p_data, sizeof(d_tmp_uint32) );
						//			
						//				NRF_LOG_INFO("1st word of frecord data in hex: 0x%08X",*d_p_uint32);
						//				NRF_LOG_INFO("2nd word of frecord data in hex: 0x%08X",*(d_p_uint32 + 1) );
						//				NRF_LOG_INFO("Location of record data: 0x%08X",(uint32_t)flash_record.p_data);
						//				NRF_LOG_INFO("Data of frecord data: 0x%08X",d_tmp_uint32);
			*/
			
			
			/* Close the record when done. */
			if (fds_record_close(&record_desc) != NRF_SUCCESS)
			{
					/* Handle error. */
				NRF_LOG_INFO("There are some error in record close.");
				d_ret = NRF_ERROR_INTERNAL;
				NRF_LOG_INFO("Error:%d", d_ret);
				break;
			}
	}
			/*					Testing read-record data
			//	NRF_LOG_INFO("Data was read in hex: 0x%4X",d_tmp_uint32);
			//	NRF_LOG_INFO("D Time was read data in hex: 0x%08X",d_stamp_data.d_time);
			//	NRF_LOG_INFO("D Mode was read data in hex: 0x%08X",d_stamp_data.d_mode);
			*/
	
	// Print all the records
	struct tm d_tm_return_time;
	for(uint32_t i=0; i<= D_MAX_SLOT;i++)
	{

			if(d_stamp_data[i].d_mode == D_MODE_BLANK ) break;
		
			d_tm_return_time = *localtime(&(d_stamp_data[i].d_time));
		NRF_LOG_INFO("DT %d is: %02d:%02d:%02d M:%d",i, d_tm_return_time.tm_hour, d_tm_return_time.tm_min, d_tm_return_time.tm_sec,d_stamp_data[i].d_mode);
	}
	return d_ret;
}

	/**@brief RTC 1 second handler function to be called by the scheduler.
 */
void d_rtc_scheduler_event_handler(void *p_event_data, uint16_t event_size)
{
	time_t check_time, current_time = 0, d_last_check_time = 0;

	uint32_t d_last_check_mode = D_MODE_BLANK;
	current_time = *((time_t*)p_event_data);
	
	// Looking for the time stamp and execue the MODE
	
	//printf(" Looking for time stamp.\n\r");

	for(uint32_t i=0;i <= D_MAX_SLOT;i++)
	{
		//printf("Cking item %d \n\r",i);
		
		check_time = d_convert_today(&d_stamp_data[i].d_time,&current_time);
		//printf (" Ck tm: %d Crt tm: %d \n",check_time, current_time);
		
		//printf("Time after convert: %d \n\r", check_time);
		
		if( current_time > check_time)	// && ((d_stamp_data[i].d_mode == D_MODE_ON)||(d_stamp_data[i].d_mode == D_MODE_OFF)) )
		{
			// If it's blank data
			if ( check_time < d_last_check_time )
			{
				//printf ("Blk dta \n\r");
				
				d_last_check_mode = d_stamp_data[i-1].d_mode;		
				switch( d_last_check_mode)
				{
				case D_MODE_ON:
					//printf(" Run ON\n");
					bsp_board_led_on(BSP_LED_INDICATE_USER_LED2);
					break;
				case D_MODE_OFF:
					//printf(" Run OFF\n");
					bsp_board_led_off(BSP_LED_INDICATE_USER_LED2);
					break;
				default:
					break;
				}
					
				break;
				
			}
			
			// Not yet reach the setting time
			
			// Remember the last checking mode
			if (d_stamp_data[i].d_mode != D_MODE_BLANK)
			{
				d_last_check_mode = d_stamp_data[i].d_mode;
				d_last_check_time = check_time;
			}
			// Checking next setting stamp
			if( i== D_MAX_SLOT) //printf("Finish checking\r\n");
			continue;
		}
		else 
		{
			
			//printf(" Check alarm\n\r");
			// Running with last checking mode
			switch( d_last_check_mode)
			{
				case D_MODE_ON:
					//printf(" Run ON\n");
					bsp_board_led_on(BSP_LED_INDICATE_USER_LED2);
					break;
				case D_MODE_OFF:
					//printf(" Run OFF\n");
					bsp_board_led_off(BSP_LED_INDICATE_USER_LED2);
					break;
				default:
					break;
			}	
			//printf(" Running at Time slot: %d at mode: %d . \n\r",i ,d_last_check_mode);
			//printf("Cpt at item %d\r\n",i);
			break;
		}
	}
	
//	if (check_time > current_time)
//		printf("You're in the past of 15:00:00.\n\r");
//	else
//		printf("You're in the present or future.\n\r");
//    // In this case, p_event_data is a pointer to a nrf_drv_gpiote_pin_t that represents
//    // the pin number of the button pressed. The size is constant, so it is ignored.
//    printf(" Running RTC handler in thread/main mode \n\r");
}


/**@brief Handle events from d_rtc timer.
 *
 * @param[in]   p_context   parameter registered in timer start function.
 */
static void d_rtc_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    //bsp_board_led_invert(BSP_LED_INDICATE_USER_LED4);
	
		// Increase current time to one second
		m_time ++;
	
		// Put task to the scheduler
		app_sched_event_put(&m_time, sizeof(m_time),d_rtc_scheduler_event_handler);
}
// End Duc's code
 
void nrf_cal_init(void)
{
    // Select the 32 kHz crystal and start the 32 kHz clock
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos;
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while(NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
    
    // Configure the RTC for 1 minute wakeup (default)
    CAL_RTC->PRESCALER = 0xFFF;
    CAL_RTC->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
    CAL_RTC->INTENSET = RTC_INTENSET_COMPARE0_Msk;
    CAL_RTC->CC[0] = m_rtc_increment * 8;
    CAL_RTC->TASKS_START = 1;
    NVIC_SetPriority(CAL_RTC_IRQn, CAL_RTC_IRQ_Priority);
	
		// Add Duc's mod
		// Temporary disable this IRQ
    //NVIC_EnableIRQ(CAL_RTC_IRQn);
		// End Duc's mod
}

// Add Duc's code
ret_code_t d_app_rtc_init(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	
//		// Reset all alarm data before recording
//		for(uint32_t i=0;i <= D_MAX_SLOT;i++)
//		d_stamp_data[i].d_mode = D_MODE_BLANK;
	
	err_code = app_timer_create(&d_rtc_tmr,
                                        APP_TIMER_MODE_REPEATED,
                                        d_rtc_timer_handler);
	if (err_code == NRF_SUCCESS)
  {
		// Start for the tick with ~1 second
		err_code = app_timer_start(d_rtc_tmr,16384,NULL);
  }
	
	return err_code;
}

// End Duc's code

void nrf_cal_set_callback(void (*callback)(void), uint32_t interval)
{
    // Set the calendar callback, and set the callback interval in seconds
    cal_event_callback = callback;
    m_rtc_increment = interval;
    m_time += CAL_RTC->COUNTER / 8;
    CAL_RTC->TASKS_CLEAR = 1;
    CAL_RTC->CC[0] = interval * 8;  
}
 
// Add Duc's code
ret_code_t d_nrf_cal_set_alarm(uint8_t hour, uint8_t minute, uint8_t second, uint32_t d_mode )
{
	ret_code_t err_code = NRF_SUCCESS;
	struct tm d_tmp_alarm_struct;
	time_t temp_alarm;
	
	// Temporary set the year day month to 0 from Unix Epoch
	d_tmp_alarm_struct.tm_year = 1970 - 1900;
  d_tmp_alarm_struct.tm_mon = 1-1;
  d_tmp_alarm_struct.tm_mday = 1;
	
	d_tmp_alarm_struct.tm_hour = hour;
	d_tmp_alarm_struct.tm_min = minute;
	d_tmp_alarm_struct.tm_sec = second; 
	
	temp_alarm = mktime(&d_tmp_alarm_struct);
	
	
		// Print all the records
	struct tm d_tm_return_time;
	for(uint32_t i=0; i<= D_MAX_SLOT;i++)
	{

			if(d_stamp_data[i].d_mode == D_MODE_BLANK ) {NRF_LOG_INFO("CM is 0"); break;}
		
			d_tm_return_time = *localtime(&(d_stamp_data[i].d_time));
		NRF_LOG_INFO("CDT %d is: %02d:%02d:%02d M:%d",i, d_tm_return_time.tm_hour, d_tm_return_time.tm_min, d_tm_return_time.tm_sec,d_stamp_data[i].d_mode);
	}
	
	
	//NRF_LOG_INFO("Start saving the data.");
	//printf("Start saving the data.\r\n");

	for(uint32_t i=0;i <= D_MAX_SLOT;i++)
	{	
		if( (d_stamp_data[i].d_mode != D_MODE_ON) && d_stamp_data[i].d_mode != D_MODE_OFF ) 
		{
			d_stamp_data[i].d_mode = d_mode;
			d_stamp_data[i].d_time = temp_alarm;
			
			// Enable Flag: Data was updated			
			d_alarm_update = true;
			NRF_LOG_INFO("Last Item: %d",i);
			//NRF_LOG_INFO("Data was recorded in item: %d",i);
			break;		
		}
		else if ( d_hour_compare(&temp_alarm,&d_stamp_data[i].d_time) == D_SAME)
		{
			d_stamp_data[i].d_mode = d_mode;
			
			// Enable Flag: Data was updated			
			d_alarm_update = true;
			NRF_LOG_INFO("Last Item: %d",i);
			
			//NRF_LOG_INFO("Data was updated in item: %d with new mode.",i);	
			break;
		}
		else if ( d_hour_compare(&temp_alarm,&d_stamp_data[i].d_time) == D_LESS)
		{
			
			//NRF_LOG_INFO("Value is less than current item.\n\r");
			
			// Re-arrange the table
			d_stamp_time_t d_tmp_data_1, d_tmp_data_2;
			
			d_tmp_data_1 = d_stamp_data[i];
			d_stamp_data[i].d_mode = d_mode;
			d_stamp_data[i].d_time = temp_alarm;
			
			for(uint32_t y = i; y < D_MAX_SLOT;y++)
			{
				d_tmp_data_2 = d_stamp_data[y+1];
				
				d_stamp_data[y+1] = d_tmp_data_1;
				if ((d_tmp_data_2.d_mode != D_MODE_ON) &&(d_tmp_data_2.d_mode != D_MODE_OFF)) break;
				
				d_tmp_data_1 = d_tmp_data_2;
				
				if ( y == D_MAX_SLOT - 1)
					NRF_LOG_INFO("Last alarm was lost to give memory for new alarm.");
				
			}
			
			// Enable Flag: Data was updated			
			d_alarm_update = true;
			NRF_LOG_INFO("Last Item: %d",i);
			
			// After re-arrange, need to close this setting
			break;
			
		}
		else 
		{
						//NRF_LOG_INFO("Setting data is higher than current item, go to nxt.");
						continue;
		}
			
	}
	
		if(d_alarm_update == true)
		{
			//NRF_LOG_INFO("Update data.");
			// Write data
			APP_ERROR_CHECK(d_write_data(d_stamp_data));
						// Enable Flag: Data was updated			
			d_alarm_update = false;
		}
	
	
				// Enable Flag: Data was updated			
			//d_alarm_update = true;
	
			//NRF_LOG_INFO("d_alarm_update is set %d:",d_alarm_update); 
	
	// Record data into flash memory using FDS
	//err_code = d_write_data(d_stamp_data);
	
	return err_code;
}

void d_check_update(void)
{
	
		if(d_alarm_update == true)
		{
			//NRF_LOG_INFO("Update data.");
			// Write data
			APP_ERROR_CHECK(d_write_data(d_stamp_data));
			// Enable Flag: Data was updated			
			d_alarm_update = false;
		}
		
}
// End Duc's code
void nrf_cal_set_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second)
{
    static time_t uncal_difftime, difftime, newtime;
    time_struct.tm_year = year - 1900;
    time_struct.tm_mon = month;
    time_struct.tm_mday = day;
    time_struct.tm_hour = hour;
    time_struct.tm_min = minute;
    time_struct.tm_sec = second;   
    newtime = mktime(&time_struct);
    CAL_RTC->TASKS_CLEAR = 1;  
    
    // Calculate the calibration offset 
    if(m_last_calibrate_time != 0)
    {
        difftime = newtime - m_last_calibrate_time;
        uncal_difftime = m_time - m_last_calibrate_time;
        m_calibrate_factor = (float)difftime / (float)uncal_difftime;
    }
    
    // Assign the new time to the local time variables
    m_time = m_last_calibrate_time = newtime;
}    

struct tm *nrf_cal_get_time(void)
{
    time_t return_time;
	
		// Add Duc's mod
		//return_time = m_time + CAL_RTC->COUNTER / 8;
		return_time = m_time;
		// End Duc's mod
    
    m_tm_return_time = *localtime(&return_time);
    return &m_tm_return_time;
}

struct tm *nrf_cal_get_time_calibrated(void)
{
    time_t uncalibrated_time, calibrated_time;
    if(m_calibrate_factor != 0.0f)
    {
        uncalibrated_time = m_time + CAL_RTC->COUNTER / 8;
        calibrated_time = m_last_calibrate_time + (time_t)((float)(uncalibrated_time - m_last_calibrate_time) * m_calibrate_factor + 0.5f);
        m_tm_return_time = *localtime(&calibrated_time);
        return &m_tm_return_time;
    }
    else return nrf_cal_get_time();
}

char *nrf_cal_get_time_string(bool calibrated)
{
    static char cal_string[80];
    strftime(cal_string, 80, "%x - %H:%M:%S", (calibrated ? nrf_cal_get_time_calibrated() : nrf_cal_get_time()));
    return cal_string;
}
 
void CAL_RTC_IRQHandler(void)
{
    if(CAL_RTC->EVENTS_COMPARE[0])
    {
        CAL_RTC->EVENTS_COMPARE[0] = 0;
        
        CAL_RTC->TASKS_CLEAR = 1;
        
        m_time += m_rtc_increment;
        if(cal_event_callback) cal_event_callback();
    }
}


