/**
 *
 * Module Description:
 *   Terminology:
 *   Timer list - queue of the running timers with the SAME timeout. For example list of 1 s timers
 *   Set - one or more lists of timers and task handling the lists.
 *       For example set A containing 1s timers, 2s timers and 5s timers
 *       and set B containing 100 ms timers, 50 ms timers and 200 ms timers
 *   Timer task - task that handles one and only one set of lists of timers
 *                      -----------   Design   ---------------
 *   In the system run one or more timers tasks handling different timer sets. Every timer
 *   set contains one or more timer lists.
 *   Function start timer allocates free entry from the stack of free timers and places
 *   the timer in the end of the timer list (FIFO). Time ~O(1)
 *   Timer task waits for the expiration of the nearest timer, calls application handler
 *   TimerExpired, find the next timer to be expired using sequential search in the
 *   set (always the first entry in a timer list). Time ~ O(size of set)
 *   Function stop timer marks a running timer as stopped. Time ~O(1)
 *                      -----------   Reasons  ---------------
 *   1. It is possible that every subsystem will have own timer tasks running in
 *      different priorities
 *   2. Set of long timers and set of short timers can be created and handled by tasks with
 *      different priorities
 *   3. "Timer expired"  application handlers can be called from different tasks. For high
 *      priority short timers such handler should be short - release semaphore for example,
 *      for low priority long timers handler can make long processing like audit in data-base
 *   4. In the system can coexist 1 or 2 short timers - 50 ms - used in call process
 *      and 10 long timers  - 10 s, 1 min, 1 h, etc. - used in the application
 *      sanity checking or management
 *   5. In the system can coexist short - 10 ms - timer that always expired and 10 long
 *      protocol timers that usually stopped by the application before expiration
 *                      -----------   Miscellaneous  ---------------
 *    The application can handle the timers by itself using the function
 *    emTimerHandleExpired()
 *    For example:
 *
 *    void MyTimerTask(emTimerSetHandle MySet)
 *    {
 *      int timeOut = FOREVER;
 *      while (1)
 *      {
 *        semGet(MySemaphore, timeOut);
 *        timeOut = emTimerHandleExpired(MySet);
 *      }
 *    }
 *
 *    void TimerCallback()
 *    {
 *      semGive(MySemaphore);
 *    }
 *
 *    This feature can be useful if for example application wants
 *    Timer handlers to be called in the context of the task which started the timers.
 *
 *
 */

#include <string>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <array>
#include <limits>

using namespace std;

typedef unsigned int emTimerSetHandle;
typedef unsigned int emTimerListHandle;

enum class EmTimerError
{
  EM_TIMER_ERROR_NONE                 ,
  EM_TIMER_ERROR_EXPIRED              ,
  EM_TIMER_ERROR_STOPPED              ,
  EM_TIMER_ERROR_ILLEGAL_HANDLE       ,
  EM_TIMER_ERROR_NO_FREE_TIMER        ,
  EM_TIMER_ERROR_ILLEGAL_ID           ,
} ;

/* ------------------- timer set ------------------------- */

typedef struct /* EmTimerSetParameters */
{
  /*
    maximum number of lists in the set
    specify the EXACT number of lists in the set
    normally 1-5
  */
  int NumberOfLists;

  /* 0 - timer task created by the em, 1 - application supplies the timer task */
  int HandlerType;

  char Name[20];

  union /* Handler */
  {
    struct  /* HandlerType = 0 */
    {
      int Priority; /* priority of the task handling this set */
      char *Name; /* name of the task */
    } TimerTask;

    struct  /* HandlerType = 1 */
    {
      /*
         this function will be called every time the application
         should call emTimerTimeout() and emTimerHandleExpired()
      */
      void (*Callback)(emTimerSetHandle SetHandle);
    } UserTask;
  } Handler;
} EmTimerSetParameters;

/*
  create empty set and timer task handling this set
  the function will return to the application SetHandle - handle to the created set
  return zero if success
*/
int emTimerSetCreate(EmTimerSetParameters *SetParameters, emTimerSetHandle *SetHandle);

/*
  add timer list ListHandle to the set specified by SetHandle
  return zero if success
*/
int emTimerAddList(emTimerSetHandle SetHandle, emTimerListHandle ListHandle);

/*
  remove timer list ListHandle from the set specified by SetHandle
  return zero if success
*/
int emTimerRemoveList(emTimerSetHandle SetHandle, emTimerListHandle ListHandle);

/*
  delete created by emTimerSetCreate() set
  all lists should be removed from the set
  return zero if success
*/
int emTimerSetDelete(emTimerSetHandle SetHandle);

/* --------------------- timer list --------------------- */

typedef struct /* emTimerListParameters */
{
  int Timeout; /* in ticks */
  int NumberOfTimers;
  int CallExpiredForStoppedTimers;
} emTimerListParameters;

/*
  create empty timer list and stack of Parameters::NumberOfTimers free timers
  the function will return handle to the created list
  return zero if success
*/
int emTimerListCreate(emTimerListParameters *ListParameters, emTimerListHandle *ListHandle);

/*
  change Timeout for already created list
  IF NEW TIMEOUT IS SHORTER THEN THE PREVIOUS ALL RUNNING TIMERS IN THE LIST WILL
  EXPIRE IMMEDIATELY
*/
int emTimerSetTimeout(emTimerListHandle ListHandle, int Timeout);

/*
  delete created by emTimerCreateList() list of timers and stack of free timers
  return zero if success
*/
int emTimerListDelete(emTimerListHandle ListHandle);

/* --------------------- timer --------------------- */

typedef struct /* emTimerT */
{
  /* --- filled by the application --- */

  /*
    this function will be called by the timer task if a timer expired
    can be 0
    Timer contains (emTimerT*)
  */
  void (*TimerExpired)(void *Timer);

  /*
    em will provide this field as an argument in the timer expiration handler
  */
  void *ApplicationData;

  /*
    unique 32-bits Id of the timer
    this field initialized by emTimerStart()
    can be used to solve the race condition between stopTimer and timerExpired -
    application can make sure that expired timer has not been stopped a moment before
    it's expiration
  */
  unsigned int Id;

  /*
    pointer used to stop the timer
    this field initialized by emTimerStart()
  */
  unsigned int TimerHandle;

} emTimerT;

/*
  ALL LISTS SHOULD BE ADDED TO THE SET BEFORE THE APPLICATION CAN CALL THIS FUNCTION

  this fuction will set fields Id and TimerHandle in the emTimerT struct provided
  by the application
  return zero if success
  return em_TIMER_ERROR_NO_FREE_TIMER if no free timer available
*/
EmTimerError emTimerStart(emTimerListHandle ListHandle, emTimerT *Timer);

/*
  this fuction will remove the timer from the list of the running timers
  TimerHandle - the field TimerHandle in the emTimerT set by emTimerStart()
  TimerId - the field Id in the emTimerT set by emTimerStart()
  return 0 if success
  return em_TIMER_ERROR_EXPIRED if the timer already expired -
    legal race condition
  return em_TIMER_ERROR_STOPPED if the timer already stopped - can be legal in some
    circumstances
  return em_TIMER_ERROR_ILLEGAL_ID if Id is not correct - legal race condition
    for example, if timer already expired and started again - the system
    can use the same handle for the new timer (the Id of course IS still unique)
    - application tries to stop it
*/
EmTimerError emTimerStop(unsigned int TimerHandle, unsigned int TimerId);


typedef enum
{
  em_TIMER_STATE_FREE,    /* DO NOT USE FUNCTION emTimerState        */
  em_TIMER_STATE_STOPPED, /* FOR RUNNING TIMERS TO AVOID RACE         */
  em_TIMER_STATE_EXPIRED, /* CONDITIONS                               */
  em_TIMER_STATE_RUNNING,
  em_TIMER_STATE_ILLEGAL,
} emTimerStateT;

/*
  this function will return state of the timer
  WARNING ! TO AVOID RACE CONDITIONS USE THIS FUNCTION ONLY IN THE
  CONTEXT OF TimerExpired FUNCTION. THE FUNCTION WILL BE USED
  IN CONJUNCTION WITH CALLEXPIREDFORSTOPPEDTIMERS != 0
  return the state of the timer
*/
emTimerStateT emTimerState(unsigned int TimerHandle);

/* --------------------- timer task --------------------- */


/*
  call emTimerT::TimerExpired() for all expired timers in the specified set
*/
int emTimerHandleExpired(emTimerSetHandle SetHandle);


/* --------------------- debug -------------------------- */

/*
  this function prints out the table of created sets, the lists contained
  in the sets and some additional debug information
*/
void emTimerPrintSets();


