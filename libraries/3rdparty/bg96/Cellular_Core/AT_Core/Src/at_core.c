/**
  ******************************************************************************
  * @file    at_core.c
  * @author  MCD Application Team
  * @brief   This file provides code for AT Core
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
#include <stdio.h>
#include <string.h>
#include "ipc_common.h"
#include "at_core.h"
#include "at_parser.h"
#include "error_handler.h"
#include "cellular_runtime_custom.h"
#include "plf_config.h"
/* following file added to check SID for DATA suspend/resume cases */
#include "cellular_service_int.h"

/* Private typedef -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#if (USE_TRACE_ATCORE == 1U)
#if (USE_PRINTF  == 0U)
#include "trace_interface.h"
#define TRACE_INFO(format, args...) TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P0, "ATCore:" format "\n\r", ## args)
#define TRACE_DBG(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_P1, "ATCore:" format "\n\r", ## args)
#define TRACE_ERR(format, args...)  TRACE_PRINT(DBG_CHAN_ATCMD, DBL_LVL_ERR, "ATCore ERROR:" format "\n\r", ## args)
#else
#define TRACE_INFO(format, args...)  (void) printf("ATCore:" format "\n\r", ## args);
#define TRACE_DBG(...)   __NOP(); /* Nothing to do */
#define TRACE_ERR(format, args...)   (void) printf("ATCore ERROR:" format "\n\r", ## args);
#endif /* USE_PRINTF */
#else
#define TRACE_INFO(...)   __NOP(); /* Nothing to do */
#define TRACE_DBG(...)   __NOP(); /* Nothing to do */
#define TRACE_ERR(...)   __NOP(); /* Nothing to do */
#endif /* USE_TRACE_ATCORE */

#define LOG_ERROR(ErrId, gravity)   ERROR_Handler(DBG_CHAN_ATCMD, (ErrId), (gravity))

/* Private defines -----------------------------------------------------------*/
#define ATCORE_MAX_HANDLES   ((int16_t) DEVTYPE_INVALID)

/* specific debug flags */
#define DBG_DUMP_IPC_RX_QUEUE (0) /* dump the IPC RX queue (advanced debug only) */

#if (RTOS_USED == 1)
/* Private defines -----------------------------------------------------------*/
#define ATCORE_SEM_WAIT_ANSWER_COUNT     (1)
#define ATCORE_SEM_SEND_COUNT            (1)
#define MSG_IPC_RECEIVED_SIZE (uint32_t) (128)
#define SIG_IPC_MSG                      (1U) /* signals definition for IPC message queue */
#define SIG_INTERNAL_EVENT_MODEM         (2U) /* signals definition for internal event from the cellular modem */

/* Global variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* this semaphore is used for waiting for an answer from Modem */
static osSemaphoreId s_WaitAnswer_SemaphoreId = NULL;
/* this semaphore is used to confirm that a msg has been sent (1 semaphore per device) */
#define ATCORE_SEMAPHORE_DEF(name,index)  \
  const osSemaphoreDef_t os_semaphore_def_##name##index = { 0 }
#define ATCORE_SEMAPHORE(name,index)  \
  &os_semaphore_def_##name##index
/* Queues definition */
/* this queue is used by IPC to inform that messages are ready to be retrieved */
static osMessageQId q_msg_IPC_received_Id;

/* Private function prototypes -----------------------------------------------*/
static at_status_t findMsgReceivedHandle(at_handle_t *athandle);
static void ATCoreTaskBody(void const *argument);
#endif /* RTOS_USED == 1 */

/* Private variables ---------------------------------------------------------*/
static uint8_t         AT_Core_initialized = 0U;
static IPC_Handle_t    ipcHandleTab[ATCORE_MAX_HANDLES];
static at_context_t    at_context[ATCORE_MAX_HANDLES];
static urc_callback_t  register_URC_callback[ATCORE_MAX_HANDLES];
static IPC_RxMessage_t msgFromIPC[ATCORE_MAX_HANDLES];        /* array of IPC msg (1 per ATCore handler) */
static __IO uint8_t    MsgReceived[ATCORE_MAX_HANDLES] = {0}; /* array of rx msg counters (1 per ATCore handler) */
static IPC_CheckEndOfMsgCallbackTypeDef custom_checkEndOfMsgCallback = NULL;

#if (RTOS_USED == 0)
static event_callback_t    register_event_callback[ATCORE_MAX_HANDLES];
#endif /* RTOS_USED == 0 */

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void msgReceivedCallback(IPC_Handle_t *ipcHandle);
static void msgSentCallback(IPC_Handle_t *ipcHandle);
static uint8_t find_index(const IPC_Handle_t *ipcHandle);

static at_status_t process_AT_transaction(at_handle_t athandle, at_msg_t msg_in_id, at_buf_t *p_rsp_buf);
static at_status_t allocate_ATHandle(at_handle_t *athandle);
static at_handle_t find_deviceType_ATHandle(sysctrl_device_type_t deviceType);
static at_status_t waitOnMsgUntilTimeout(at_handle_t athandle, uint32_t Tickstart, uint32_t Timeout);
static at_status_t waitFromIPC(at_handle_t athandle,
                               uint32_t tickstart, uint32_t cmdTimeout, IPC_RxMessage_t *p_msg);
static at_action_rsp_t process_answer(at_handle_t athandle, at_action_send_t action_send, uint32_t at_cmd_timeout);
static at_action_rsp_t analyze_action_result(at_handle_t athandle, at_action_rsp_t val);

static void IRQ_DISABLE(void);
static void IRQ_ENABLE(void);

/* Functions Definition ------------------------------------------------------*/
at_status_t  AT_init(void)
{
  at_status_t retval;
  at_handle_t idx;

  /* should be called once */
  if (AT_Core_initialized == 1U)
  {
    LOG_ERROR(1, ERROR_WARNING);
    retval = ATSTATUS_ERROR;
  }
  else
  {
    /* Initialize at_context */
    for (idx = 0; idx < ATCORE_MAX_HANDLES; idx++)
    {
      MsgReceived[idx] = 0U;
      register_URC_callback[idx] = NULL;
#if (RTOS_USED == 0)
      register_event_callback[idx] = NULL;
#endif /* RTOS_USED == 0 */
      at_context[idx].device_type = DEVTYPE_INVALID;
      at_context[idx].in_data_mode = AT_FALSE;
      at_context[idx].processing_cmd = 0U;
      at_context[idx].dataSent = AT_FALSE;
#if (RTOS_USED == 1)
      at_context[idx].action_flags = ATACTION_RSP_NO_ACTION;
      at_context[idx].p_rsp_buf = NULL;
      at_context[idx].s_SendConfirm_SemaphoreId = NULL;
#endif /* RTOS_USED == 1 */

      (void) memset((void *)&at_context[idx].parser, 0, sizeof(atparser_context_t));
    }

    AT_Core_initialized = 1U;
    retval = ATSTATUS_OK;
  }

  return (retval);
}

at_handle_t  AT_open(sysctrl_info_t *p_device_infos, const event_callback_t event_callback, urc_callback_t urc_callback)
{
#if (RTOS_USED == 1)
  UNUSED(event_callback); /* used only for non-RTOS */
#endif /* RTOS_USED == 1 */

  at_handle_t affectedHandle;

  /* Initialize parser for required device type */
  if (ATParser_initParsers(p_device_infos->type) != ATSTATUS_OK)
  {
    affectedHandle = AT_HANDLE_INVALID;
  }
  /* allocate a free handle */
  else if (allocate_ATHandle(&affectedHandle) != ATSTATUS_OK)
  {
    affectedHandle = AT_HANDLE_INVALID;
  }
  else
  {
    /* nothing to do */
  }


  if (affectedHandle != AT_HANDLE_INVALID)
  {
    /* set adapter name for this context */
    at_context[affectedHandle].ipc_handle = &ipcHandleTab[affectedHandle];
    at_context[affectedHandle].device_type = p_device_infos->type;
    at_context[affectedHandle].ipc_device  = p_device_infos->ipc_device;
    if (p_device_infos->ipc_interface == IPC_INTERFACE_UART)
    {
      at_context[affectedHandle].ipc_mode = IPC_MODE_UART_CHARACTER;

      /* start in COMMAND MODE */
      at_context[affectedHandle].in_data_mode = AT_FALSE;

      /* no command actually in progress */
      at_context[affectedHandle].processing_cmd = 0U;

      /* init semaphore for data sent indication */
      at_context[affectedHandle].dataSent = AT_FALSE;

#if (RTOS_USED == 1)
      ATCORE_SEMAPHORE_DEF(ATCORE_SEM_SEND, affectedHandle);
      at_context[affectedHandle].s_SendConfirm_SemaphoreId =
        osSemaphoreCreate(ATCORE_SEMAPHORE(ATCORE_SEM_SEND, affectedHandle), ATCORE_SEM_SEND_COUNT);
      if (at_context[affectedHandle].s_SendConfirm_SemaphoreId != NULL)
      {
        /* init semaphore */
        (void) osSemaphoreWait(at_context[affectedHandle].s_SendConfirm_SemaphoreId, RTOS_WAIT_FOREVER);
#else /* RTOS_USED */
      register_event_callback[affectedHandle] = event_callback;
#endif /* RTOS_USED == 1 */

        /* register client callback for URC */
        register_URC_callback[affectedHandle] = urc_callback;

        /* init the ATParser */
        ATParser_init(&at_context[affectedHandle], &custom_checkEndOfMsgCallback);
      }
      else
      {
        TRACE_ERR("SendSemaphoreId creation error for handle = %d", affectedHandle)
        affectedHandle = AT_HANDLE_INVALID;
      }
    }
    else
    {
      /* Only UART is supported */
      affectedHandle = AT_HANDLE_INVALID;
    }
  }
  return ((at_handle_t) affectedHandle);
}

at_status_t  AT_open_channel(at_handle_t athandle)
{
  at_status_t retval;

  if (athandle != AT_HANDLE_INVALID)
  {
    at_context[athandle].in_data_mode = AT_FALSE;
    at_context[athandle].processing_cmd = 0U;
    at_context[athandle].dataSent = AT_FALSE;
#if (RTOS_USED == 1)
    at_context[athandle].action_flags = ATACTION_RSP_NO_ACTION;
#endif /* RTOS_USED == 1 */

    /* Open the IPC channel */
    if (IPC_open(at_context[athandle].ipc_handle,
                 at_context[athandle].ipc_device,
                 at_context[athandle].ipc_mode,
                 msgReceivedCallback,
                 msgSentCallback,
                 custom_checkEndOfMsgCallback) == IPC_OK)
    {

      /* Select the IPC opened channel as current channel */
      if (IPC_select(at_context[athandle].ipc_handle) == IPC_OK)
      {
        retval = ATSTATUS_OK;
      }
      else
      {
        TRACE_ERR("IPC selection error")
        retval = ATSTATUS_ERROR;
      }
    }
    else
    {
      TRACE_ERR("IPC open error")
      retval = ATSTATUS_ERROR;
    }
  }
  else
  {
    TRACE_ERR("IPC invalid handle")
    retval = ATSTATUS_ERROR;
  }

  return (retval);
}

at_status_t  AT_close_channel(at_handle_t athandle)
{
  at_status_t retval;

  if (athandle != AT_HANDLE_INVALID)
  {
    if (IPC_close(at_context[athandle].ipc_handle) == IPC_OK)
    {
      retval = ATSTATUS_OK;
    }
    else
    {
      TRACE_ERR("IPC close error")
      retval = ATSTATUS_ERROR;
    }
  }
  else
  {
    TRACE_ERR("IPC invalid handle")
    retval = ATSTATUS_ERROR;
  }

  return (retval);
}

at_status_t AT_reset_context(at_handle_t athandle)
{
  at_status_t retval;

  if (athandle != AT_HANDLE_INVALID)
  {
    at_context[athandle].in_data_mode = AT_FALSE;
    at_context[athandle].processing_cmd = 0U;
    at_context[athandle].dataSent = AT_FALSE;

#if (RTOS_USED == 1)
    at_context[athandle].action_flags = ATACTION_RSP_NO_ACTION;
#endif /* RTOS_USED == 1 */

    /* reinit IPC channel and select our channel */
    retval = ATSTATUS_ERROR;
    if (IPC_reset(at_context[athandle].ipc_handle) == IPC_OK)
    {
      if (IPC_select(at_context[athandle].ipc_handle) == IPC_OK)
      {
        retval = ATSTATUS_OK;
      }
    }
  }
  else
  {
    /* invalid handle */
    retval = ATSTATUS_ERROR;
  }

  return (retval);
}

at_status_t AT_sendcmd(at_handle_t athandle, at_msg_t msg_in_id, at_buf_t *p_cmd_in_buf, at_buf_t *p_rsp_buf)
{
  /* Sends a service request message to the ATCore.
  *  This is a blocking function.
  *  It returns when the command is fully processed or a timeout expires.
  */
  at_status_t retval;

  if (athandle == AT_HANDLE_INVALID)
  {
    retval = ATSTATUS_ERROR;
    LOG_ERROR(21, ERROR_WARNING);
  }
  else
  {
    /* Check if a command is already ongoing for this handle */
    if (at_context[athandle].processing_cmd == 1U)
    {
      TRACE_ERR("!!!!!!!!!!!!!!!!!! WARNING COMMAND IS UNDER PROCESS !!!!!!!!!!!!!!!!!!")
      retval = ATSTATUS_ERROR;
      LOG_ERROR(2, ERROR_WARNING);
      goto exit_func;
    }

    /* initialize response buffer */
    (void) memset((void *)p_rsp_buf, 0, ATCMD_MAX_BUF_SIZE);

    /* start to process this command */
    at_context[athandle].processing_cmd = 1U;

#if (RTOS_USED == 1)
    /* save ptr on response buffer */
    at_context[athandle].p_rsp_buf = p_rsp_buf;
#endif /* RTOS_USED == 1 */

    /* Check if current mode is DATA mode */
    if (at_context[athandle].in_data_mode == AT_TRUE)
    {
      /* Check if user command is DATA suspend */
      if (msg_in_id == (at_msg_t) SID_CS_DATA_SUSPEND)
      {
        /* restore IPC Command channel to send ESCAPE COMMAND */
        TRACE_DBG("<<< restore IPC COMMAND channel >>>")
        (void) IPC_select(at_context[athandle].ipc_handle);
      }
    }
    /* check if trying to suspend DATA while in command mode */
    else if (msg_in_id == (at_msg_t) SID_CS_DATA_SUSPEND)
    {
      retval = ATSTATUS_ERROR;
      LOG_ERROR(3, ERROR_WARNING);
      TRACE_ERR("DATA not active")
      goto exit_func;
    }
    else
    {
      /* nothing to do */
    }

    /* Process the user request */
    ATParser_process_request(&at_context[athandle], msg_in_id, p_cmd_in_buf);

    /* Start an AT command transaction */
    retval = process_AT_transaction(athandle, msg_in_id, p_rsp_buf);
    if (retval != ATSTATUS_OK)
    {
      TRACE_DBG("AT_sendcmd error: process AT transaction")
      /* retrieve and send error report if exist */
      (void) ATParser_get_error(&at_context[athandle], p_rsp_buf);
      ATParser_abort_request(&at_context[athandle]);
      if (msg_in_id == (at_msg_t) SID_CS_DATA_SUSPEND)
      {
        /* force to return to command mode */
        TRACE_ERR("force to return to COMMAND mode")
        at_context[athandle].in_data_mode = AT_FALSE ;
      }
      goto exit_func;
    }

    /* get command response buffer */
    (void) ATParser_get_rsp(&at_context[athandle], p_rsp_buf);

exit_func:
    /* finished to process this command */
    at_context[athandle].processing_cmd = 0U;
  }

  return (retval);
}

#if (RTOS_USED == 0)
at_status_t  AT_getevent(at_handle_t athandle, at_buf_t *p_rsp_buf)
{
  at_status_t retval = ATSTATUS_OK, retUrc;
  at_action_rsp_t action;

  /* retrieve response */
  if (IPC_receive(at_context[athandle].ipc_handle, &msgFromIPC[athandle]) == IPC_ERROR)
  {
    TRACE_ERR("IPC receive error")
    LOG_ERROR(4, ERROR_WARNING);
    return (ATSTATUS_ERROR);
  }

  /* one message has been read */
  IRQ_DISABLE();
  MsgReceived[athandle]--;
  IRQ_ENABLE();

  /* Parse the response */
  action = ATParser_parse_rsp(&at_context[athandle], &msgFromIPC[athandle]);

  TRACE_DBG("RAW ACTION (get_event) = 0x%x", action)

  /* continue if this is an intermediate response */
  if ((action != ATACTION_RSP_URC_IGNORED) &&
      (action != ATACTION_RSP_URC_FORWARDED))
  {
    TRACE_INFO("this is not an URC (%d)", action)
    return (ATSTATUS_ERROR);
  }

  if (action == ATACTION_RSP_URC_FORWARDED)
  {
    /* notify user with callback */
    if (register_URC_callback[athandle] != NULL)
    {
      /* get URC response buffer */
      do
      {
        retUrc = ATParser_get_urc(&at_context[athandle], p_rsp_buf);
        if ((retUrc == ATSTATUS_OK) || (retUrc == ATSTATUS_OK_PENDING_URC))
        {
          /* call the URC callback */
          (* register_URC_callback[athandle])(p_rsp_buf);
        }
      } while (retUrc == ATSTATUS_OK_PENDING_URC);
    }
  }

  return (retval);
}
#endif /* RTOS_USED == 0 */

void AT_internalEvent(sysctrl_device_type_t deviceType)
{
  /* add internal event for the deviceType (supports only cellular modem actually) */
  if (deviceType == DEVTYPE_MODEM_CELLULAR)
  {
    if (osMessagePut(q_msg_IPC_received_Id, SIG_INTERNAL_EVENT_MODEM, 0U) != osOK)
    {
      TRACE_ERR("q_msg_IPC_received_Id error for SIG_INTERNAL_EVENT_MODEM")
    }
  }
}

/* Private function Definition -----------------------------------------------*/
static uint8_t find_index(const IPC_Handle_t *ipcHandle)
{
  at_handle_t idx    = 0;
  uint8_t found  = 0U;
  uint8_t retval = 0U;

  do
  {
    if (&ipcHandleTab[idx] == ipcHandle)
    {
      found = 1U;
      retval = idx;
    }
    idx++;
  } while ((idx < ATCORE_MAX_HANDLES) && (found == 0U));

  if (found == 0U)
  {
    LOG_ERROR(5, ERROR_FATAL);
  }
  return (retval);
}

static void msgReceivedCallback(IPC_Handle_t *ipcHandle)
{
  /* Warning ! this function is called under IT */
  uint8_t index = find_index(ipcHandle);

#if (RTOS_USED == 1)
  /* disable irq not required, we are under IT */
  MsgReceived[index]++;

  if (osMessagePut(q_msg_IPC_received_Id, SIG_IPC_MSG, 0U) != osOK)
  {
    TRACE_ERR("q_msg_IPC_received_Id error for SIG_IPC_MSG")
  }

#else
  MsgReceived[index]++;
  /* received during in IDLE state ? this is an URC */
  if (at_context[index].processing_cmd == 0)
  {
    if (register_event_callback[index] != NULL)
    {
      (* register_event_callback[index])();
    }
  }
#endif /* RTOS_USED == 1 */
}

static void msgSentCallback(IPC_Handle_t *ipcHandle)
{
  /* Warning ! this function is called under IT */
  uint8_t index = find_index(ipcHandle);
  at_context[index].dataSent = AT_TRUE;

#if (RTOS_USED == 1)
  (void) osSemaphoreRelease(at_context[index].s_SendConfirm_SemaphoreId);
#endif /* RTOS_USED == 1 */

}

static at_status_t allocate_ATHandle(at_handle_t *athandle)
{
  at_status_t retval = ATSTATUS_OK;
  at_handle_t idx = 0;

  /* find first free handle */
  while ((idx < ATCORE_MAX_HANDLES) && (at_context[idx].device_type != DEVTYPE_INVALID))
  {
    idx++;
  }

  if (idx == ATCORE_MAX_HANDLES)
  {
    LOG_ERROR(6, ERROR_WARNING);
    retval = ATSTATUS_ERROR;
  }
  *athandle = (at_handle_t) idx;

  return (retval);
}

static at_handle_t find_deviceType_ATHandle(sysctrl_device_type_t deviceType)
{
  at_handle_t retval = AT_HANDLE_INVALID; /* default value */
  at_handle_t idx = 0;

  /* find handle */
  while (idx < ATCORE_MAX_HANDLES)
  {
    if (at_context[idx].device_type == deviceType)
    {
      /* return corresponding athandle */
      retval = idx;
      /* leave loop */
      break;
    }
    idx++;
  }

  return (retval);
}

static at_status_t waitOnMsgUntilTimeout(at_handle_t athandle, uint32_t Tickstart, uint32_t Timeout)
{
  at_status_t retval = ATSTATUS_OK;
#if (RTOS_USED == 1)
  UNUSED(athandle);
  UNUSED(Tickstart);

  TRACE_DBG("**** Waiting Sema (to=%lu) *****", Timeout)
  if (osSemaphoreWait(s_WaitAnswer_SemaphoreId, Timeout) != ((int32_t)osOK))
  {
    TRACE_DBG("**** Sema Timeout (=%ld) !!! *****", Timeout)
    retval = ATSTATUS_TIMEOUT;
  }
  TRACE_DBG("**** Sema Freed *****")

#else
  if (athandle == AT_HANDLE_INVALID)
  {
    retvalk = ATSTATUS_ERROR;
  }
  else
  {
    /* Wait until flag is set */
    while (!(MsgReceived[athandle]))
    {
      /* Check for the Timeout */
      if (Timeout != ATCMD_MAX_DELAY)
      {
        if ((Timeout == 0) || ((HAL_GetTick() - Tickstart) > Timeout))
        {
          retval = ATSTATUS_TIMEOUT;
          break;
        }
      }
    }
  }
#endif /* RTOS_USED == 1 */
  return (retval);
}

static at_action_rsp_t process_answer(at_handle_t athandle, at_action_send_t action_send, uint32_t at_cmd_timeout)
{
  at_action_rsp_t  action_rsp;
  at_status_t  waitIPCstatus;

  /* init tickstart for a full AT transaction */
  uint32_t tickstart = HAL_GetTick();

  do
  {
    /* Wait for response from IPC */
    waitIPCstatus = waitFromIPC(athandle, tickstart, at_cmd_timeout, &msgFromIPC[athandle]);
    if (waitIPCstatus != ATSTATUS_OK)
    {
      /* No response received before timeout */
      if ((action_send & ATACTION_SEND_WAIT_MANDATORY_RSP) != 0U)
      {
        /* Waiting for a response (mandatory) */
        TRACE_ERR("AT_sendcmd error: wait from ipc")

#if (DBG_DUMP_IPC_RX_QUEUE == 1)
        /* in case of advanced debug of IPC RX queue only */
        if (waitIPCstatus == ATSTATUS_TIMEOUT)
        {
          IPC_dump_RX_queue(at_context[athandle].ipc_handle, 1);
        }
#endif /* (DBG_DUMP_IPC_RX_QUEUE == 1) */

        LOG_ERROR(10, ERROR_WARNING);
        action_rsp = ATACTION_RSP_ERROR;
      }
      else /* ATACTION_SEND_TEMPO */
      {
        /* Temporisation (was waiting for a non-mandatory event)
        *  now that timer has expired, proceed to next action if needed
        */
        if ((action_send & ATACTION_SEND_FLAG_LAST_CMD) != 0U)
        {
          action_rsp = ATACTION_RSP_FRC_END;
        }
        else
        {
          action_rsp = ATACTION_RSP_FRC_CONTINUE;
        }
      }
    }
    else
#if (RTOS_USED == 1)
    {
      /* Treat the received response */
      /* Retrieve the action which has been set on IPC msg reception in ATCoreTaskBody
      *  More than one action could has been set
      */
      if ((at_context[athandle].action_flags & ATACTION_RSP_FRC_END) != 0U)
      {
        action_rsp = ATACTION_RSP_FRC_END;
        /* clean flag */
        at_context[athandle].action_flags &= ~((at_action_rsp_t) ATACTION_RSP_FRC_END);
      }
      else if ((at_context[athandle].action_flags & ATACTION_RSP_FRC_CONTINUE) != 0U)
      {
        action_rsp = ATACTION_RSP_FRC_CONTINUE;
        /* clean flag */
        at_context[athandle].action_flags &= ~((at_action_rsp_t) ATACTION_RSP_FRC_CONTINUE);
      }
      else if ((at_context[athandle].action_flags & ATACTION_RSP_ERROR) != 0U)
      {
        /* clean flag */
        at_context[athandle].action_flags &= ~((at_action_rsp_t) ATACTION_RSP_ERROR);
        TRACE_ERR("AT_sendcmd error: parse from rsp")
        LOG_ERROR(11, ERROR_WARNING);
        action_rsp = ATACTION_RSP_ERROR;
      }
      else
      {
        /* all other actions are ignored */
        action_rsp = ATACTION_RSP_IGNORED;
      }
    }
#else /* RTOS_USED == 0 */
    {
      /* Treat the received response */
      /* Parse the response */
      action_rsp = ATParser_parse_rsp(&at_context[athandle], &msgFromIPC[athandle]);

      /* analyze the response (check data mode flag) */
      action_rsp = analyze_action_result(athandle, action_rsp);
      TRACE_DBG("RAW ACTION (AT_sendcmd) = 0x%x", action_rsp)
    }

    /* URC received during AT transaction */
    if (action_rsp == ATACTION_RSP_URC_FORWARDED)
    {
      /* notify user with callback */
      if (register_URC_callback[athandle] != NULL)
      {
        at_status_t retUrc;
        do
        {
          /* get URC response buffer */
          retUrc = ATParser_get_urc(&at_context[athandle], p_rsp_buf);
          if ((retUrc == ATSTATUS_OK) || (retUrc == ATSTATUS_OK_PENDING_URC))
          {
            /* call the URC callback */
            (* register_URC_callback[athandle])(p_rsp_buf);
          }
        } while (retUrc == ATSTATUS_OK_PENDING_URC);
      }
      else
      {
        TRACE_ERR("No valid callback to forward URC")
      }
    }
#endif /* RTOS_USED == 1 */

    /* exit loop if action_rsp = ATACTION_RSP_ERROR */
  } while ((action_rsp == ATACTION_RSP_INTERMEDIATE) ||
           (action_rsp == ATACTION_RSP_IGNORED) ||
           (action_rsp == ATACTION_RSP_URC_FORWARDED) ||
           (action_rsp == ATACTION_RSP_URC_IGNORED));

  return (action_rsp);
}

static at_status_t process_AT_transaction(at_handle_t athandle, at_msg_t msg_in_id, at_buf_t *p_rsp_buf)
{
#if (RTOS_USED == 1)
  UNUSED(p_rsp_buf);
#endif /* RTOS_USED == 1 */

  /* static variables (do not use stack) */
  static AT_CHAR_t build_atcmd[ATCMD_MAX_CMD_SIZE] = {0};

  /* local variables */
  at_status_t retval = ATSTATUS_OK;
  uint32_t at_cmd_timeout = 0U;
  at_action_send_t action_send;
  uint16_t build_atcmd_size;
  uint8_t another_cmd_to_send;
  at_action_rsp_t action_rsp = ATACTION_RSP_NO_ACTION;

  /* reset at cmd buffer */
  (void) memset((void *) build_atcmd, 0, ATCMD_MAX_CMD_SIZE);

  do
  {
    another_cmd_to_send = 0U; /* default value: this is the last command (will be changed if this is not the case) */

    (void) memset((void *)&build_atcmd[0], 0, sizeof(AT_CHAR_t) * ATCMD_MAX_CMD_SIZE);
    build_atcmd_size = 0U;

    /* Get command to send */
    action_send = ATParser_get_ATcmd(&at_context[athandle],
                                     (uint8_t *)&build_atcmd[0],
                                     (uint16_t)(sizeof(AT_CHAR_t) * ATCMD_MAX_CMD_SIZE),
                                     &build_atcmd_size, &at_cmd_timeout);
    if ((action_send & ATACTION_SEND_ERROR) != 0U)
    {
      TRACE_DBG("AT_sendcmd error: get at command")
      LOG_ERROR(7, ERROR_WARNING);
      retval = ATSTATUS_ERROR;
    }
    else
    {
      /* Send AT command through IPC if a valid command is available */
      if (build_atcmd_size > 0U)
      {
        /* Before to send a command, check if current mode is DATA mode
        *  (exception if request is to suspend data mode)
        */
        if ((at_context[athandle].in_data_mode == AT_TRUE) && (msg_in_id != (at_msg_t) SID_CS_DATA_SUSPEND))
        {
          /* impossible to send a CMD during data mode */
          TRACE_ERR("DATA ongoing, can not send a command")
          LOG_ERROR(8, ERROR_WARNING);
          retval = ATSTATUS_ERROR;
        }
        else
        {
          retval = sendToIPC(athandle, (uint8_t *)&build_atcmd[0], build_atcmd_size);
          if (retval != ATSTATUS_OK)
          {
            TRACE_ERR("AT_sendcmd error: send to ipc")
            LOG_ERROR(9, ERROR_WARNING);
            retval = ATSTATUS_ERROR;
          }
        }
      }

      if (retval != ATSTATUS_ERROR)
      {
        /* Wait for a response or a delay (which could be = 0)*/
        if (((action_send & ATACTION_SEND_WAIT_MANDATORY_RSP) != 0U) ||
            ((action_send & ATACTION_SEND_TEMPO) != 0U))
        {
          action_rsp = process_answer(athandle, action_send, at_cmd_timeout);
          if (action_rsp == ATACTION_RSP_FRC_CONTINUE)
          {
            /* this is no the last command */
            another_cmd_to_send = 1U;
          }
        }
        else
        {
          TRACE_ERR("Invalid action code")
          LOG_ERROR(13, ERROR_WARNING);
          retval = ATSTATUS_ERROR;
        }
      }
    }

    /* check if an error occured */
    if (retval == ATSTATUS_ERROR)
    {
      another_cmd_to_send = 0U;
    }

  } while (another_cmd_to_send == 1U);

#if (RTOS_USED == 1)
  /* clear all flags*/
  at_context[athandle].action_flags = ATACTION_RSP_NO_ACTION;
#endif /* RTOS_USED == 1 */

  TRACE_DBG("action_rsp value = %d", action_rsp)

  /* check if an error occured during the answer processing */
  if (action_rsp == ATACTION_RSP_ERROR)
  {
    LOG_ERROR(14, ERROR_WARNING);
    retval = ATSTATUS_ERROR;
  }

  return (retval);
}


at_status_t sendToIPC(at_handle_t athandle,
                             uint8_t *cmdBuf, uint16_t cmdSize)
{
  at_status_t retval;

  /* Send AT command */
  if (IPC_send(at_context[athandle].ipc_handle, cmdBuf, cmdSize) == IPC_ERROR)
  {
    TRACE_ERR(" IPC send error")
    LOG_ERROR(15, ERROR_WARNING);
    retval = ATSTATUS_ERROR;
  }
  else
  {
#if (RTOS_USED == 1)
    (void) osSemaphoreWait(at_context[athandle].s_SendConfirm_SemaphoreId, RTOS_WAIT_FOREVER);
#else
    /* waiting TX confirmation, done by callback from IPC */
    while (at_context[athandle].dataSent == AT_FALSE)
    {
    }
    at_context[athandle].dataSent = AT_FALSE;
#endif /* RTOS_USED == 1 */
    retval = ATSTATUS_OK;
  }

  return (retval);
}

static at_status_t waitFromIPC(at_handle_t athandle,
                               uint32_t tickstart, uint32_t cmdTimeout, IPC_RxMessage_t *p_msg)
{
#if (RTOS_USED == 1)
  UNUSED(p_msg);
#endif /* RTOS_USED == 1 */

  at_status_t retval;

  /* wait for complete message */
  retval = waitOnMsgUntilTimeout(athandle, tickstart, cmdTimeout);
  if (retval != ATSTATUS_OK)
  {
    if (cmdTimeout != 0U)
    {
      TRACE_INFO("TIMEOUT EVENT(%ld ms)", cmdTimeout)
    }
  }

#if (RTOS_USED == 0)
  /* retrieve response */
  if (IPC_receive(at_context[athandle].ipc_handle, p_msg) == IPC_ERROR)
  {
    TRACE_ERR("IPC receive error")
    LOG_ERROR(16, ERROR_WARNING);
    retval = ATSTATUS_ERROR;
  }
  else
  {
    /* one message has been read */
    IRQ_DISABLE();
    MsgReceived[athandle]--;
    IRQ_ENABLE();
  }
#endif /* RTOS_USED == 0 */

  return (retval);
}

static at_action_rsp_t analyze_action_result(at_handle_t athandle, at_action_rsp_t val)
{
  at_action_rsp_t action;

  /* retrieve DATA flag value */
  at_bool_t data_mode = ((val & ATACTION_RSP_FLAG_DATA_MODE) != 0U) ? AT_TRUE : AT_FALSE;

  /* clean DATA flag value */
  action = (at_action_rsp_t)(val & ~(at_action_rsp_t)ATACTION_RSP_FLAG_DATA_MODE);

  TRACE_DBG("RAW ACTION (analyze_action_result) = 0x%x", val)
  TRACE_DBG("CLEANED ACTION=%d (data mode=%d)", action, data_mode)

  if (data_mode == AT_TRUE)
  {
    /* DATA MODE has been activated */
    if (at_context[athandle].in_data_mode == AT_FALSE)
    {
      IPC_Handle_t *h_other_ipc = IPC_get_other_channel(at_context[athandle].ipc_handle);
      if (h_other_ipc != NULL)
      {
        (void) IPC_select(h_other_ipc);
        at_context[athandle].in_data_mode = AT_TRUE;
        TRACE_INFO("<<< DATA MODE SELECTED >>>")
      }
      else
      {
        TRACE_ERR("<<< ERROR WHEN SELECTING DATA MODE >>>")
        action = ATACTION_RSP_IGNORED;
      }
    }
  }
  else
  {
    /* COMMAND MODE is active
    */
    if (at_context[athandle].in_data_mode == AT_TRUE)
    {
      at_context[athandle].in_data_mode = AT_FALSE;

      TRACE_INFO("<<< COMMAND MODE SELECTED >>>")
    }
  }

  /* return action cleaned from DATA flag value */
  return (action);
}

static void IRQ_DISABLE(void)
{
  __disable_irq();
}

static void IRQ_ENABLE(void)
{
  __enable_irq();
}

#if (RTOS_USED == 1)
at_status_t atcore_task_start(osPriority taskPrio, uint16_t stackSize)
{
  at_status_t retval;

  /* ATCore task handler */
  static osThreadId atcoreTaskId = NULL;

  /* check if AT_init has been called before */
  if (AT_Core_initialized != 1U)
  {
    TRACE_ERR("error, ATCore is not initialized")
    LOG_ERROR(17, ERROR_WARNING);
    retval = ATSTATUS_ERROR;
  }
  else
  {
    /* semaphores creation */
    osSemaphoreDef(ATCORE_SEM_WAIT_ANSWER);
    s_WaitAnswer_SemaphoreId = osSemaphoreCreate(osSemaphore(ATCORE_SEM_WAIT_ANSWER), ATCORE_SEM_WAIT_ANSWER_COUNT);
    if (s_WaitAnswer_SemaphoreId == NULL)
    {
      TRACE_ERR("s_WaitAnswer_SemaphoreId creation error")
      LOG_ERROR(18, ERROR_WARNING);
      retval = ATSTATUS_ERROR;
    }
    else
    {
      /* init semaphore */
      (void) osSemaphoreWait(s_WaitAnswer_SemaphoreId, RTOS_WAIT_FOREVER);

      /* queues creation */
      osMessageQDef(IPC_MSG_RCV, MSG_IPC_RECEIVED_SIZE, uint16_t); /* Define message queue */
      q_msg_IPC_received_Id = osMessageCreate(osMessageQ(IPC_MSG_RCV), NULL); /* create message queue */

      /* start driver thread */
      osThreadDef(atcoreTask, ATCoreTaskBody, taskPrio, 0, stackSize);
      atcoreTaskId = osThreadCreate(osThread(atcoreTask), NULL);
      if (atcoreTaskId == NULL)
      {
        TRACE_ERR("atcoreTaskId creation error")
        LOG_ERROR(19, ERROR_WARNING);
        retval = ATSTATUS_ERROR;
      }
      else
      {
        retval = ATSTATUS_OK;
#if (STACK_ANALYSIS_TRACE == 1)
        (void) stackAnalysis_addStackSizeByHandle(atcoreTaskId, stackSize);
#endif /* STACK_ANALYSIS_TRACE */
      }
    }
  }

  return (retval);
}

static at_status_t findMsgReceivedHandle(at_handle_t *athandle)
{
  at_status_t retval = ATSTATUS_ERROR;
  bool leave_loop = false;
  at_handle_t i = 0;

  do
  {
    if (MsgReceived[i] != 0U)
    {
      *athandle = (at_handle_t) i;
      retval = ATSTATUS_OK;
      leave_loop = true;
    }
    i++;
  } while ((leave_loop == false) && (i < ATCORE_MAX_HANDLES));

  return (retval);
}

static void ATCoreTaskBody(void const *argument)
{
  UNUSED(argument);

  at_status_t retUrc;
  at_handle_t athandle;
  at_status_t ret;
  at_action_rsp_t action;
  osEvent event;

  static at_buf_t urc_buf[ATCMD_MAX_BUF_SIZE]; /* buffer size not optimized yet */

  TRACE_DBG("<start ATCore TASK>")

  /* Infinite loop */
  for (;;)
  {
    /* waiting IPC message received event (message) */
    event = osMessageGet(q_msg_IPC_received_Id, RTOS_WAIT_FOREVER);
    if (event.status == osEventMessage)
    {
      TRACE_DBG("<ATCore TASK> - received msg event= 0x%lx", event.value.v)

      if (event.value.v == (SIG_IPC_MSG))
      {
        /* a message has been received from IPC, retrieve its handle */
        ret = findMsgReceivedHandle(&athandle);
        if (ret != ATSTATUS_OK)
        {
          /* skip this loop iteration */
          continue;
        }

        /* retrieve message from IPC */
        if (IPC_receive(&ipcHandleTab[athandle], &msgFromIPC[athandle]) == IPC_ERROR)
        {
          TRACE_ERR("IPC receive error")
          ATParser_abort_request(&at_context[athandle]);
          TRACE_DBG("**** Sema Released on error 1 *****")
          (void) osSemaphoreRelease(s_WaitAnswer_SemaphoreId);
          /* skip this loop iteration */
          continue;
        }

        /* one message has been read */
        IRQ_DISABLE();
        MsgReceived[athandle]--;
        IRQ_ENABLE();

        /* Parse the response */
        action = ATParser_parse_rsp(&at_context[athandle], &msgFromIPC[athandle]);

        /* analyze the response (check data mode flag) */
        action = analyze_action_result(athandle, action);

        /* add this action to action flags */
        at_context[athandle].action_flags |= action;
        TRACE_DBG("add action 0x%x (flags=0x%x)", action, at_context[athandle].action_flags)
        if (action == ATACTION_RSP_ERROR)
        {
          TRACE_ERR("AT_sendcmd error")
          ATParser_abort_request(&at_context[athandle]);
          TRACE_DBG("**** Sema Released on error 2 *****")
          (void) osSemaphoreRelease(s_WaitAnswer_SemaphoreId);
          continue;
        }

        /* check if this is an URC to forward */
        if (action == ATACTION_RSP_URC_FORWARDED)
        {
          /* notify user with callback */
          if (register_URC_callback[athandle] != NULL)
          {
            /* get URC response buffer */
            do
            {
              (void) memset((void *) urc_buf, 0, ATCMD_MAX_BUF_SIZE);
              retUrc = ATParser_get_urc(&at_context[athandle], urc_buf);
              if ((retUrc == ATSTATUS_OK) || (retUrc == ATSTATUS_OK_PENDING_URC))
              {
                /* call the URC callback */
                (* register_URC_callback[athandle])(urc_buf);
              }
            } while (retUrc == ATSTATUS_OK_PENDING_URC);
          }
        }
        else if ((action == ATACTION_RSP_FRC_CONTINUE) ||
                 (action == ATACTION_RSP_FRC_END) ||
                 (action == ATACTION_RSP_ERROR))
        {
          TRACE_DBG("**** Sema released *****")
          (void) osSemaphoreRelease(s_WaitAnswer_SemaphoreId);
        }
        else
        {
          /* nothing to do */
        }

      }
      else if (event.value.v == (SIG_INTERNAL_EVENT_MODEM))
      {
        /* An internal event has been received (ie not coming from IPC: could be an interrupt from modem,...)
         * Do not call IPC_receive in this case
         */
        TRACE_INFO("!!! an internal event has been received !!!")
        athandle = find_deviceType_ATHandle(DEVTYPE_MODEM_CELLULAR);
        if (athandle != AT_HANDLE_INVALID)
        {
          if (register_URC_callback[athandle] != NULL)
          {
            do
            {
              (void) memset((void *) urc_buf, 0, ATCMD_MAX_BUF_SIZE);
              retUrc = ATParser_get_urc(&at_context[athandle], urc_buf);
              if ((retUrc == ATSTATUS_OK) || (retUrc == ATSTATUS_OK_PENDING_URC))
              {
                /* call the URC callback */
                (* register_URC_callback[athandle])(urc_buf);
              }
            } while (retUrc == ATSTATUS_OK_PENDING_URC);
          }
        }
      }
      else
      {
        /* should not happen */
        TRACE_INFO("an unexpected event has been received !!!")
      }
    }
  }
}
#endif /* RTOS_USED == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

