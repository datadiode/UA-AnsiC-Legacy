/******************************************************************************************************/
/* (c) Copyright 2005-2008 The OPC Foundation                                                         */
/* ALL RIGHTS RESERVED.                                                                               */
/*                                                                                                    */
/* DISCLAIMER:                                                                                        */
/*  This code is provided by the OPC Foundation solely to assist in                                   */
/*  understanding and use of the appropriate OPC Specification(s) and may be                          */
/*  used as set forth in the License Grant section of the OPC Specification.                          */
/*  This code is provided as-is and without warranty or support of any sort                           */
/*  and is subject to the Warranty and Liability Disclaimers which appear                             */
/*  in the printed OPC Specification.                                                                 */
/******************************************************************************************************/
/* Platform Portability Layer                                                                         */
/* Modify the content of this file according to the guid implementation on your system or emulate     */
/* the uuid generation process in this file.                                                          */
/******************************************************************************************************/

/* system includes */
#include <stdlib.h>
#include <stdio.h>

/* platform layer base */
#include <opcua_p_internal.h>

/* all platform modules */
#include <opcua_p_datetime.h>
#include <opcua_p_guid.h>
#include <opcua_p_memory.h>
#include <opcua_p_mutex.h>
#include <opcua_p_semaphore.h>
#include <opcua_p_socket_interface.h>
#include <opcua_p_socket_ssl.h>
#include <opcua_p_string.h>
#include <opcua_p_thread.h>
#include <opcua_p_trace.h>
#include <opcua_p_socket.h>
#include <opcua_p_timer.h>
#include <opcua_p_utilities.h>
#include <opcua_p_cryptofactory.h>
#include <opcua_p_pkifactory.h>
#include <opcua_p_openssl.h>

/*============================================================================
 * g_OpcUa_Port_CallTable
 *===========================================================================*/
/** @brief static calltable for accessing platform layer functions. */
OpcUa_Port_CallTable g_OpcUa_Port_CallTable =
{
    0,                      /* version number */
    OpcUa_Null,

    /* Memory */
    OpcUa_P_Memory_Alloc,
    OpcUa_P_Memory_Free,
    OpcUa_P_Memory_ReAlloc,
    OpcUa_P_Memory_MemCpy,  /* OpcUa_P_Memory_MemCpy may be removed */
    OpcUa_Null,             /* OpcUa_P_Memory_MemSet may be removed */

    /* DateTime */
    OpcUa_P_DateTime_UtcNow,
    OpcUa_P_DateTime_GetTimeOfDay,
    OpcUa_P_DateTime_GetStringFromDateTime,
    OpcUa_P_DateTime_GetDateTimeFromString,

    /* Mutex */
    OpcUa_P_Mutex_CreateImp,
    OpcUa_P_Mutex_DeleteImp,
    OpcUa_P_Mutex_LockImp,
    OpcUa_P_Mutex_UnlockImp,

    /* Guid */
    OpcUa_P_Guid_Create,

    /* Semaphore */
    OpcUa_P_Semaphore_Create,
    OpcUa_P_Semaphore_Delete,
    OpcUa_P_Semaphore_Wait,
    OpcUa_P_Semaphore_TimedWait,
    OpcUa_P_Semaphore_Post,

    /* Thread */
    OpcUa_P_Thread_Create,
    OpcUa_P_Thread_Delete,
    OpcUa_P_Thread_Start,
    OpcUa_P_Thread_Sleep,
    OpcUa_P_Thread_GetCurrentThreadId,

    /* Trace */
    OpcUa_P_Trace,
    OpcUa_P_Trace_Initialize,
    OpcUa_P_Trace_Clear,

    /* String */
    OpcUa_P_String_strncpy,
    OpcUa_P_String_strncat,
    OpcUa_P_String_strlen,
    OpcUa_P_String_strncmp,
    OpcUa_P_String_strnicmp,
    OpcUa_P_String_vsnprintf,

    /* Utilities */
    OpcUa_P_QSort,
    OpcUa_P_BSearch,
    OpcUa_P_GetLastError,
    OpcUa_P_GetTickCount,
    OpcUa_P_CharAToInt,

    /* Network */
    OpcUa_P_Socket_InetAddr,
    OpcUa_P_SocketManager_Create,
    OpcUa_P_SocketManager_Delete,
    OpcUa_P_SocketManager_CreateServer,
    OpcUa_P_SocketManager_CreateClient,
#if OPCUA_P_SOCKETMANAGER_SUPPORT_SSL
    OpcUa_P_SocketManager_CreateSslServer,
    OpcUa_P_SocketManager_CreateSslClient,
#else
    OpcUa_Null,
    OpcUa_Null,
#endif
    OpcUa_P_SocketManager_SignalEvent,
    OpcUa_P_SocketManager_ServeLoop,
    OpcUa_P_Socket_Read,
    OpcUa_P_Socket_Write,
    OpcUa_P_Socket_Close,
    OpcUa_P_Socket_GetPeerInfo,
    OpcUa_P_Socket_GetLastError,
    OpcUa_P_Socket_SetUserData,
    OpcUa_P_Socket_InitializeNetwork,
    OpcUa_P_Socket_CleanupNetwork,

    /* Security */
    OpcUa_P_CryptoFactory_CreateCryptoProvider,
    OpcUa_P_CryptoFactory_DeleteCryptoProvider,
    OpcUa_P_PKIFactory_CreatePKIProvider,
    OpcUa_P_PKIFactory_DeletePKIProvider,

    /* Timer */
    OpcUa_P_Timer_Create,
    OpcUa_P_Timer_Delete,
    OpcUa_P_Timer_CleanupTimers,

    /* OpenSSL */
#if OPCUA_REQUIRE_OPENSSL
    OpcUa_P_OpenSSL_Thread_Cleanup,
    OpcUa_P_OpenSSL_SeedPRNG
#endif
};

/*============================================================================
 * OpcUa_P_Initialize
 *===========================================================================*/
/**
 * Description
 * @param ppCallTable Description
 * @return Description
 */
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Initialize(OpcUa_Handle* a_pPlatformLayerHandle)
{
    OpcUa_StatusCode uStatus = OpcUa_Good;

    OpcUa_ReturnErrorIfArgumentNull(a_pPlatformLayerHandle);

    /* was this function called before */
    if(g_OpcUa_Port_CallTable.pReserved != OpcUa_Null)
    {
        return OpcUa_BadInvalidState;
    }

    /* marked as initialized */
    g_OpcUa_Port_CallTable.pReserved = (OpcUa_Void*)1;

    uStatus = OpcUa_P_InitializeTimers();
    OpcUa_ReturnErrorIfBad(uStatus);

#if OPCUA_REQUIRE_OPENSSL
    OpcUa_P_OpenSSL_Initialize();
#endif /* OPCUA_REQUIRE_OPENSSL */

    /* assign call table */
    *a_pPlatformLayerHandle = (OpcUa_Handle)&g_OpcUa_Port_CallTable;

    return uStatus;
}

/*============================================================================
 * OpcUa_P_Clean
 *===========================================================================*/
/**
 * Description
 * @param ppCallTable Description
 * @return Description
 */
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Clean(OpcUa_Handle* a_pPlatformLayerHandle)
{
    if(*a_pPlatformLayerHandle == OpcUa_Null)
    {
        return OpcUa_BadInvalidState;
    }

    if(*a_pPlatformLayerHandle != &g_OpcUa_Port_CallTable)
    {
        return OpcUa_BadInvalidArgument;
    }

    /* check if initialized */
    if(g_OpcUa_Port_CallTable.pReserved == OpcUa_Null)
    {
        return OpcUa_BadInvalidState;
    }

#if 0 /* Removed because it needs to be called by the stack before the platform layer is cleaned up. */
    OpcUa_P_Timer_CleanupTimers();
#endif

#if OPCUA_REQUIRE_OPENSSL
    OpcUa_P_OpenSSL_Cleanup();
#endif /* OPCUA_REQUIRE_OPENSSL */

    /* marked as cleared */
    g_OpcUa_Port_CallTable.pReserved = OpcUa_Null;

    *a_pPlatformLayerHandle = OpcUa_Null;

    return OpcUa_Good;
}




