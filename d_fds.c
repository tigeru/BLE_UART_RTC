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
 
#include "d_fds.h"
#include "nrf.h"

// Add Duc's code

// End Duc's code

const char *fds_err_str(ret_code_t ret)
{
    /* Array to map FDS return values to strings. */
    static char const * err_str[] =
    {
        "FDS_ERR_OPERATION_TIMEOUT",
        "FDS_ERR_NOT_INITIALIZED",
        "FDS_ERR_UNALIGNED_ADDR",
        "FDS_ERR_INVALID_ARG",
        "FDS_ERR_NULL_ARG",
        "FDS_ERR_NO_OPEN_RECORDS",
        "FDS_ERR_NO_SPACE_IN_FLASH",
        "FDS_ERR_NO_SPACE_IN_QUEUES",
        "FDS_ERR_RECORD_TOO_LARGE",
        "FDS_ERR_NOT_FOUND",
        "FDS_ERR_NO_PAGES",
        "FDS_ERR_USER_LIMIT_REACHED",
        "FDS_ERR_CRC_CHECK_FAILED",
        "FDS_ERR_BUSY",
        "FDS_ERR_INTERNAL",
    };

    return err_str[ret - NRF_ERROR_FDS_ERR_BASE];
}
// End Duc's code

// Add Duc's code

/**@brief   Sleep until an event is received. */
static void power_manage(void)
{
#ifdef SOFTDEVICE_PRESENT
    (void) sd_app_evt_wait();
#else
    __WFE();
#endif
}

/**@brief   Wait for fds to initialize. */
void wait_for_fds_ready(void)
{
    while (!m_fds_initialized)
    {
        power_manage();
    }
		m_fds_initialized = false;
}

/**@brief   Wait until writing completed. */
void wait_for_writing(void)
{
    while (!md_fds_written)
    {
        power_manage();
    }
		
		md_fds_written = false;
}

/**@brief   Wait until writing completed. */
void wait_for_updating(void)
{
    while (!md_fds_updated)
    {
        power_manage();
    }
		md_fds_updated = false;
}

static void fds_evt_handler(fds_evt_t const * p_evt)
{
    if (p_evt->result == NRF_SUCCESS)
    {
        NRF_LOG_GREEN("Event: %s received (NRF_SUCCESS)",
                      fds_evt_str[p_evt->id]);
    }
    else
    {
        NRF_LOG_GREEN("Event: %s received (%s)",
                      fds_evt_str[p_evt->id],
                      fds_err_str(p_evt->result));
    }

    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_evt->result == NRF_SUCCESS)
            {
                m_fds_initialized = true;
            }
            break;

        case FDS_EVT_WRITE:
        {
            if (p_evt->result == NRF_SUCCESS)
            {
                NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->write.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->write.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->write.record_key);
							
								md_fds_written = true;
            }
						
						// Confirm the completed writing to continue nxt operation
						
        } break;
				
				case FDS_EVT_UPDATE:
        {
					
					  if (p_evt->result == NRF_SUCCESS)
            {
								NRF_LOG_INFO("Update successfully");
								// Confirm the completed updating to continue nxt operation
								md_fds_updated = true;
            }		
						
        } break;

        case FDS_EVT_DEL_RECORD:
        {
            if (p_evt->result == NRF_SUCCESS)
            {
                NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->del.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->del.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->del.record_key);
            }
            m_delete_all.pending = false;
        } break;

        default:
            break;
    }
}

ret_code_t d_fds_init(void)
{
		ret_code_t rc;
			
	  NRF_LOG_INFO("FDS example started.");

    /* Register first to receive an event when initialization is complete. */
    (void) fds_register(fds_evt_handler);

    NRF_LOG_INFO("Initializing fds...");

    rc = fds_init();
    APP_ERROR_CHECK(rc);
		
		/* Wait for fds to initialize. */
    wait_for_fds_ready();
			
		fds_stat_t stat = {0};

    rc = fds_stat(&stat);
    APP_ERROR_CHECK(rc);

    NRF_LOG_INFO("Found %d valid records.", stat.valid_records);
    NRF_LOG_INFO("Found %d dirty records (ready to be garbage collected).", stat.dirty_records);
		
		// Garbage collection
		rc = fds_gc();
		if( rc !=NRF_SUCCESS )
		{
			NRF_LOG_INFO("There are some error during garbage collection.");
		}

		NRF_LOG_INFO("FDS init successfully.");
			
//					static uint32_t   const d_deadbeef = 0xDAADBEEF;
//					fds_record_t        record;
//					fds_record_desc_t   record_desc ={0};
//					fds_find_token_t   	d_ftok = {0};
//					// Set up record.
//					record.file_id           = D_FILE_ID;
//					record.key               = D_RECORD_KEY_OFFSET;
//					record.data.p_data       = &d_deadbeef;
//					record.data.length_words = 1;   /* one word is four bytes. */
//					rc = fds_record_write(&record_desc, &record);
//					if (rc != NRF_SUCCESS)
//					{
//							/* Handle error. */
//					}
					
//					NRF_LOG_INFO("Start find function");
//					ret_code_t ret = fds_record_find(D_FILE_ID, D_RECORD_KEY_OFFSET, &record_desc, &d_ftok);
//					
//					if (ret == NRF_SUCCESS)
//					{						
//							ret = fds_record_update(&record_desc, &record);
//							NRF_LOG_INFO("Logger configuration file updated with result:%d", ret);
//						
//					}
//					else if (ret == FDS_ERR_NOT_FOUND)
//					{
//							ret = fds_record_write(&record_desc, &record);
//							NRF_LOG_INFO("Logger configuration file written with result:%d", ret);
//						
//							wait_for_writing();
//					}
//					else
//					{	
//							NRF_LOG_INFO("There are some error.");
//							ret = NRF_ERROR_INTERNAL;
//							NRF_LOG_INFO("Error:%d", ret);
//					}
//		
//					
//					fds_record_desc_t   d_desc = {0};
//					fds_flash_record_t  flash_record;
//					fds_find_token_t    ftok;
//					/* It is required to zero the token before first use. */
//					memset(&ftok, 0x00, sizeof(fds_find_token_t));
//					
//				//	rc = fds_record_find(FILE_ID, RECORD_KEY_1, &d_desc, &ftok);
//					while (fds_record_find(D_FILE_ID, D_RECORD_KEY_OFFSET, &d_desc, &ftok) == NRF_SUCCESS)
//					{
//								if (fds_record_open(&d_desc, &flash_record) != NRF_SUCCESS)
//								{
//										/* Handle error. */
//								}
//								/* Access the record through the flash_record structure. */
//								uint32_t d_tmp_uint32;
//								memcpy(&d_tmp_uint32, flash_record.p_data, sizeof(d_tmp_uint32) );
//								printf("Data read in init: %4x \r\n", d_tmp_uint32);
//								/* Close the record when done. */
//								if (fds_record_close(&d_desc) != NRF_SUCCESS)
//								{
//										/* Handle error. */
//								}
//					}

		return rc;
	
}
// End Duc's code
