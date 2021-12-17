//=============================================================================
// system.h                                                        CHG 20060222
//
//=============================================================================
// Various system types and constants
//-----------------------------------------------------------------------------
#ifndef _SYSTEM_
#define _SYSTEM_

#define MINIMUM(x,y)	((x)<(y)?(x):(y))


#if defined _KEIL_ARTX_
#include <RTL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <debug.h>

#elif defined _WINDOWS || defined WIN32
#include <process.h>
#include <crtdbg.h>
#include <assert.h>
#include <afx.h>
#endif


#define FALSE 0
#define TRUE  1

#ifndef NULL
#define NULL  ((void*)0)
#endif

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned int   BOOL;


#if defined _KEIL_ARTX_
  #define SYS_TICK 10
  #define TH_PARAM void                        // Parameter list to a function defined as a thread 
//  #define START_THREAD(a) os_tsk_create (a, 0) // Start a thread
  #define START_THREAD_EX(a,b) os_tsk_create_ex (a,0,b) // Start a thread
  #define START_THREAD_USER_EX(a,b,c,d) {memset(c, 0xCC, d); os_tsk_create_user_ex(a,0,c,d,b);}
  #define TRACE(...) messageDebug(1, __MODULE__, __LINE__, __VA_ARGS__) //TRACE always will result in a DBG_INFO

//  #define START_THREAD(a) os_tsk_create (a, 1) // Start a thread
  #define START_THREAD_USR(a,b,c,d) {memset(c, 0xCC, d); os_tsk_create_user(a,b,c,d);} // Start a thread, set the stack to 0xCC initially
  #define INIT_THREAD_USR(a,b,c,d) {memset(c, 0xCC, d); os_sys_init_user(a,b,c,d);} // Start a thread, set the stack to 0xCC initially

  
  #define SEM OS_SEM                           // Declare a semaphore
  #define SEM_INIT(a) os_sem_init((a), 0)     // Init semaphore, set count to 1
  #define SEM_WAIT(a,t) os_sem_wait((a),(t)==-1?-1:(t)/10)    // Wait for a semaphore t=-1 is forever
  #define SEM_SIGNAL(a) os_sem_send((a))      // signal semaphore
  #define SEM_SIGNAL_ISR(a) isr_sem_send((a)) // signal semaphore (must only be called from an ISR !)
  
  #define MUTEX OS_MUT                            // Declare a Mutex
  #define MUTEX_INIT(a) os_mut_init((a))            // Init mutex
  #define MUTEX_ACQUIRE(a) os_mut_wait((a),0xFFFF)  // Wait forever for mutex
  #define MUTEX_RELEASE(a) os_mut_release((a))      // Release mutex
  
  #define MAILBOX(a,s) os_mbx_declare(a,s)            // Define a mailbox, name is a, size is s 
  #define MAILBOX_INIT(a) os_mbx_init(a,sizeof(a))    // Initialize mailbox
  #define MAILBOX_SEND(a,buf,t) os_mbx_send(a,buf,t)  // Insert an element in mailbox
  #define MAILBOX_WAIT(a,e,t) os_mbx_wait(a,e,t)      // Wait for an element to become available in mailbox
  #define MAILBOX_RC_TMO OS_R_TMO                     // Returncode from MAILBOX_WAIT if timeout wating for element
  
  #define OS_WAIT(a) os_dly_wait((a)/10)              // Wait a number of mSecs (assuming that OS_TICK is set to 10000 in RTX_Config.C)

#if defined DEBUG
//  #define DBG(format, ...) printf(format, __VA_ARGS__)
 #define DBG printf                            // Sends messages to debug output
#else
  #define DBG
#endif
  #define ASSERT


  #define ALLOC(a) malloc(a)                          // Allocate a chunk of memory
  #define DEALLOC(a) free(a)                          // Free a chunk of memory

#elif defined _WINDOWS || defined WIN32

  #define TH_PARAM void* arg                          // Parameter list to a function defined as a thread
  #define START_THREAD(a) _beginthread(a,0,NULL)      // Start a thread
  #define __task
  #define SEM HANDLE                                  // Declare a semaphore
  #define SEM_INIT(a) a=CreateSemaphore(0,NULL,1,NULL)// Init semaphore, set count to 0
  #define SEM_WAIT(a,t) WaitForSingleObject(a,t)      // Wait for a semaphore t=-1 is forever
  #define SEM_SIGNAL(a) ReleaseSemaphore(a,1,NULL)    // Signal semaphore
  
  #define MUTEX CRITICAL_SECTION                      // Declare a Mutex
  #define MUTEX_INIT(a) InitializeCriticalSection(&a)  // Init mutex
  #define MUTEX_ACQUIRE(a) EnterCriticalSection(&a)    // Wait for for mutex (forever)
  #define MUTEX_RELEASE(a) LeaveCriticalSection(&a)    // Release mutex
  
  #define MAILBOX(a,s) XXXXX            // Define a mailbox, name is a, size is s 
  #define MAILBOX_INIT(a) XXXX(a,sizeof(a))    // Initialize mailbox
  #define MAILBOX_SEND(a,buf,t) XXXXX(a,buf,t)  // Insert an element in mailbox
  #define MAILBOX_WAIT(a,e,t) XXXXX(a,e,t)      // Wait for an element to become available in mailbox
  #define MAILBOX_RC_TMO XXXXX                     // Returncode from MAILBOX_WAIT if timeout wating for element
  
  #define OS_WAIT(a) Sleep(a)                         // Wait a number of mSecs
#ifdef DEBUG
  #define DBG(a) TRACE(a)                             // Sends messages to debug output
#else
#define DBG(a)
#endif
  #define ALLOC(a) malloc(a)                          // Allocate a chunk of memory
  #define DEALLOC(a) free(a)                          // Free a chunk of memory
#endif

#endif
