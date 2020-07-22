/**
  ******************************************************************************
  * @file    dc_common.c
  * @author  MCD Application Team
  * @brief   Code of Data Cache common services
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "cmsis_os_misrac2012.h"
#include "dc_common.h"
#include "dc_control.h"
#if (USE_RTC == 1)
#include "dc_time.h"
#endif /* (USE_RTC == 1) */

#include "error_handler.h"


/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
dc_com_db_t dc_com_db;

/* Private function prototypes -----------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static osMutexId dc_common_mutex = NULL;

/* Functions Definition ------------------------------------------------------*/



/**
  * @brief  to register a generic cb to a given dc_db
  * @param  dc_db               reference to the Data Cache used. Must be set to &dc_com_db
  * @param  notif_cb            address of callback.
  *                             This callback is called when a Data Cache entry
  *                             has been updated. The callback is executed in
  *                             the writing task context
  * @param  private_data        address of user private context.
  *                             This address is passed as a parameter of the callback
  * @retval dc_com_reg_id_t     created entry identifier
  */
dc_com_reg_id_t dc_com_register_gen_event_cb(dc_com_db_t *dc_db,
                                             dc_com_gen_event_callback_t notif_cb,
                                             const void *private_data)
{
  dc_com_reg_id_t user_id;
  if (dc_db->user_number < DC_COM_MAX_NB_USERS)
  {
    user_id = dc_com_db.user_number;
    dc_db->user_info[user_id].user_reg_id       = user_id;
    dc_db->user_info[user_id].notif_cb          = notif_cb;
    dc_db->user_info[user_id].private_user_data = private_data;
    dc_db->user_number++;
  }
  else
  {
    user_id = DC_COM_INVALID_ENTRY;
  }

  return user_id;
}

/**
  * @brief  to register ser service in dc
  * @param  dc_db               (in) data base reference
  * @param  data                (in) data structure
  * @param  len                 (in) data len
  * @retval res_id              (out) res id
  */
dc_com_res_id_t dc_com_register_serv(dc_com_db_t *dc_db,  void *data, uint16_t len)
{
  dc_com_res_id_t res_id;
  dc_base_rt_info_t *base_rt;

  if (dc_db->serv_number < DC_COM_SERV_MAX)
  {
    res_id = dc_com_db.serv_number;
    dc_db->dc_db[dc_com_db.serv_number]     = data;
    dc_db->dc_db_len[dc_com_db.serv_number] = len;
    base_rt = (dc_base_rt_info_t *)data;

    base_rt->header.res_id = res_id;
    base_rt->header.size   = len;
    base_rt->rt_state      = DC_SERVICE_OFF;

    dc_db->serv_number++;
  }
  else
  {
    res_id = DC_COM_INVALID_ENTRY;
  }

  return res_id;
}

/**
  * @brief  update a data info in the DC
  * @param  dc                  data base reference
  * @param  res_id              resource id
  * @param  data                data to write
  * @param  len                 len of data to write
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_write(void *dc, dc_com_res_id_t res_id, void *data, uint32_t len)
{
  dc_com_reg_id_t reg_id;
  dc_com_event_id_t event_id = (dc_com_event_id_t)res_id;
  dc_base_rt_info_t *dc_base_rt_info;
  dc_com_status_t res;

  dc_com_db_t *com_db = (dc_com_db_t *)dc;
  if (res_id < com_db->serv_number)
  {
    /* avoid to be interrupted bvy another event before the end of first event processing */
    (void)osMutexWait(dc_common_mutex, RTOS_WAIT_FOREVER);

    (void)memcpy((void *)(com_db->dc_db[res_id]), data, (uint32_t)len);
    dc_base_rt_info = (dc_base_rt_info_t *)(com_db->dc_db[res_id]);
    dc_base_rt_info->header.res_id = (dc_com_res_id_t)event_id;
    dc_base_rt_info->header.size   = len;

    for (reg_id = 0; reg_id < DC_COM_MAX_NB_USERS; reg_id++)
    {
      const dc_com_user_info_t *user_info ;
      user_info = &(com_db->user_info[reg_id]);

      if (user_info->notif_cb != NULL)
      {
        user_info->notif_cb(event_id, user_info->private_user_data);
      }
    }

    (void)osMutexRelease(dc_common_mutex);
    res = DC_COM_OK;
  }
  else
  {
    res = DC_COM_ERROR;
  }

  return res;
}

/**
  * @brief  read current data info in the DC
  * @param  dc                  data base reference
  * @param  res_id              resource id
  * @param  data                data to read
  * @param  len                 len of data to read
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_read(void *dc, dc_com_res_id_t res_id, void *data, uint32_t len)
{
  dc_com_status_t res;
  dc_com_db_t *com_db = (dc_com_db_t *)dc;
  if (res_id < com_db->serv_number)
  {
    (void)memcpy(data, (void *)com_db->dc_db[res_id], (uint32_t)len);
    res = DC_COM_OK;
  }
  else
  {
    (void)memset(data, 0, (uint32_t)len); /* data->rt_state == 0 is DC_SERVICE_UNAVAIL */
    res = DC_COM_ERROR;
  }
  return res;
}

/**
  * @brief  send an event to DC
  * @param  dc                  data base reference
  * @param  event_id            event id
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_write_event(void *dc, dc_com_event_id_t event_id)
{
  dc_com_reg_id_t reg_id;
  const dc_com_db_t *com_db = (dc_com_db_t *)dc;

  /* avoid to be interrupted bvy another event before the end of first event processing */
  (void)osMutexWait(dc_common_mutex, RTOS_WAIT_FOREVER);
  for (reg_id = 0; reg_id < DC_COM_MAX_NB_USERS; reg_id++)
  {
    const dc_com_user_info_t *user_info ;
    user_info = &(com_db->user_info[reg_id]);

    if (user_info->notif_cb != NULL)
    {
      user_info->notif_cb(event_id, user_info->private_user_data);
    }
  }
  (void)osMutexRelease(dc_common_mutex);
  return DC_COM_OK;
}

/**
  * @brief  create the dc and load the data cache to RAM with default value if specific setting does not exist.
  * @param -
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_init(void)
{
  (void)memset(&dc_com_db, 0, sizeof(dc_com_db));

  (void)dc_ctrl_init();

#if (USE_RTC == 1)
  dc_time_init();
#endif /* (USE_RTC == 1) */

  osMutexDef(dc_common_mutex_def);
  dc_common_mutex = osMutexCreate(osMutex(dc_common_mutex_def));
  if (dc_common_mutex == NULL)
  {
    ERROR_Handler(DBG_CHAN_DATA_CACHE, 1, ERROR_FATAL);
  }

  return DC_COM_OK;
}

/**
  * @brief  dc data base start
  * @param  -
  * @retval -
  */

void dc_com_start(void)
{
  /* In this function, we start all DC tasks */
  (void)dc_ctrl_start();
}

/**
  * @brief  unload the DC from RAM or BACKUP SRAM (INIT IMPLEMENTED).
  * @param  init_db             input data base
  * @param  dc                  output data base
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_deinit(const void *init_db, const void *dc)
{
  UNUSED(dc);
  UNUSED(init_db);
  return DC_COM_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

