/**
  ******************************************************************************
  * @file    dc_common.h
  * @author  MCD Application Team
  * @brief   Header for dc_common.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DC_COMMON_H
#define DC_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup CELLULAR_INTERFACE
  * @{
  */

/**
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### How to use Data Cache module #####
  ==============================================================================

  Data Cache allows the sharing of data and events by X-Cube-Cellular components.

  A software component (producer) creates a data entry and writes data in it.
  Each data entry is associated to an identifier.
  Creation of a data entry is done by calling dc_com_register_serv() service.
  Update of data entry value is done by calling dc_com_write() service.

  The other components (consumers) can read the data by means of the identifier.
  A component can subscribe a callback in order to be informed
  when a Data Cache data entry has been updated.
  Subscription is done through dc_com_register_gen_event_cb() service
  Read of data entry value is done by calling dc_com_read() service.

  The Data Cache structure includes the rt_state field.
  This field contains the state of service and the validity of entry data.
  e.g:
    - DC_SERVICE_UNAVAIL: field values of structure not significant
    - DC_SERVICE_ON: service started (field values of structure are significant)
    - Other value are entry dependent

  @endverbatim
  */

/** @defgroup DC Data Cache module
  * @{
  */

/** @defgroup DC_COMMON Common services
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup DC_COMMON_Constants Constants
  * @{
  */

/** @brief Number max of Data Cache subscriber */
#define DC_COM_MAX_NB_USERS 10U

/** @brief Number max of Data Cache entries */
#define DC_COM_SERV_MAX     35U

/** @brief Invalid entry */
#define DC_COM_INVALID_ENTRY    0xFFFFU

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup DC_COMMON_Types Types
  * @{
  */

typedef uint16_t  dc_com_reg_id_t;

typedef uint16_t  dc_com_res_id_t; /*!< type of Data Cache entry */


typedef uint16_t  dc_com_event_id_t; /*!< type of Data Cache event */


typedef uint32_t  dc_com_status_t;
#define   DC_COM_OK     (dc_com_status_t)0U
#define   DC_COM_ERROR  (dc_com_status_t)1U


typedef enum
{
  DC_SERVICE_UNAVAIL = 0x00, /*!< Service is unavailable. HW and/or SW driver are not present. */
  DC_SERVICE_RESET,          /*!< Service is resetting. When reset is complete, the Service enters in ON or READY state */
  DC_SERVICE_CALIB,          /*!< Service is under calibration procedure */
  DC_SERVICE_OFF,            /*!< Service is OFF */
  DC_SERVICE_SHUTTING_DOWN,  /*!< Service is being shutdown */
  DC_SERVICE_STARTING,       /*!< Service is starting but not fully operational */
  DC_SERVICE_RUN,            /*!< Service is ON (functional) but not Calibrated or not Initialized */
  DC_SERVICE_ON,             /*!< Service is ON and fully operational and calibrated */
  DC_SERVICE_FAIL            /*!< Service is Failed */
} dc_service_rt_state_t;

typedef struct
{
  dc_com_res_id_t res_id;
  uint32_t size;
} dc_service_rt_header_t;


typedef struct
{
  dc_service_rt_header_t header;
  dc_service_rt_state_t rt_state;
  uint8_t data;
} dc_base_rt_info_t;


typedef void (*dc_com_gen_event_callback_t)(
  const dc_com_event_id_t event_id,
  const void *private_user_data);

typedef struct
{
  dc_com_reg_id_t user_reg_id;
  dc_com_gen_event_callback_t notif_cb;
  const void *private_user_data;
} dc_com_user_info_t;

typedef struct
{
  dc_com_reg_id_t   user_number;
  dc_com_res_id_t   serv_number;
  dc_com_user_info_t user_info[DC_COM_MAX_NB_USERS];
  void *dc_db[DC_COM_SERV_MAX];
  uint16_t dc_db_len[DC_COM_SERV_MAX];
} dc_com_db_t;

/**
  * @}
  */

/* External variables --------------------------------------------------------*/
/** @defgroup DC_COMMON_Variables Variables
  * @{
  */

extern dc_com_db_t dc_com_db; /*!< Data Cache Database Identifier */

/**
  * @}
  */


/* Exported macros -----------------------------------------------------------*/
/** @defgroup DC_COMMON_Macros Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions ------------------------------------------------------- */
/** @defgroup DC_COMMON_Functions Functions
  * @brief    Data cache entry management (Register, Read, Write)
  * @{
  */

/**
  * @brief  Register a new entry in DC
  * @param  dc_db               (in) data base reference
  * @param  data                (in) data structure
  * @param  len                 (in) data len
  * @retval res_id              (out) res id
  */
dc_com_res_id_t dc_com_register_serv(dc_com_db_t *dc_db,
                                     void *data, uint16_t len);

/**
  * @brief  Register a generic cb to a given dc_db
  * @param  dc_db             (in)   data base reference
  * @param  notif_cb          (in)   user callback
  * @param  private_data      (in)   user context
  * @retval dc_com_reg_id_t   (out)  created identifier
  */
dc_com_reg_id_t dc_com_register_gen_event_cb(dc_com_db_t *dc_db,
                                             dc_com_gen_event_callback_t notif_cb,
                                             const void *private_data);

/**
  * @brief  Update a data info in the DC
  * @param  dc                  data base reference
  * @param  res_id              identifier of the Data Cache entry to write
  * @param  data                data to write
  * @param  len                 len of data to write
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_write(void *dc,
                             dc_com_res_id_t res_id,
                             void *data, uint32_t len);

/**
  * @brief  Read current data info in the DC
  * @param  dc                  data base reference
  * @param  res_id              identifier of the Data Cache entry to read
  * @param  data                data to read
  * @param  len                 len of data to read
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_read(void *dc,
                            dc_com_res_id_t res_id,
                            void *data, uint32_t len);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/*** Component Initialization/Start *******************************************/
/*** Used by Cellular-Service - Not an User Interface *************************/

/**
  * @brief  Create the dc and load the data cache to RAM (BACKUPSRAM)
  *         with default value if specific setting does not exist.
  * @param  -
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_init(void);

/**
  * @brief  dc data base start
  * @param  -
  * @retval -
  */

void dc_com_start(void);

/**
  * @brief  unload the DC from RAM or BACKUP SRAM (INIT IMPLEMENTED).
  * @param  init_db             input data base
  * @param  dc                  output data base
  * @retval dc_com_status_t     return status
  */
dc_com_status_t dc_com_deinit(const void *init_db, const void *dc);

/**
  * @brief  send an event to DC
  * @param  dc                  data base reference
  * @param  event_id            identifier of the Data Cache event to send
  * @retval dc_com_status_t     return status
  */

dc_com_status_t dc_com_write_event(void *dc, dc_com_event_id_t event_id);


#ifdef __cplusplus
}
#endif


#endif /* DC_COMMON_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
