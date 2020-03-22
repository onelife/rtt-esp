/* Standard includes. */
#include <stdlib.h>
#include <string.h>
// #include "sdkconfig.h"

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE
#include "esp_newlib.h"
#include "esp_compiler.h"

#include "FreeRTOS.h"
// #include "portmux_impl.h"
#include "include/rthw.h"
#include "include/rtt_port.h"

/*
    esp_timer.c: ~ RTT Semaphore
    - xQueueCreateCountingSemaphore
    - xQueueGiveFromISR
    - xQueueGenericReceive
    - vQueueDelete
    esp_event.c: ~ 
    - xQueueSendToBack (TODO)
    - xQueueSendToBackFromISR (TODO)
    ipc.c: ~ RTT Mutex (Non-recursive) + Semaphore (binary)
    - xQueueCreateMutex
    - xQueueGenericCreate
    - xQueueGenericSend
    - xQueueGenericReceive
    log.c: ~ RTT Mutex
    - xQueueCreateMutex
    - xQueueGenericReceive
    - xQueueGenericSend
    locks.c: ~ RTT Mutex (Recursive) + RTT Mutex (Non-recursive) but in ISR!!!
    - xQueueCreateMutex
    - xQueueTakeMutexRecursive
    - xQueueGiveMutexRecursive
    - xQueueGenericSend
    - xQueueGenericReceive
    - xQueueGiveFromISR
    - xQueueReceiveFromISR
    - xQueueGetMutexHolder
    - vQueueDelete
    cache_utils.c: ~ RTT Mutex (Recursive)
    - xQueueCreateMutex
    - xQueueTakeMutexRecursive
    - xQueueGiveMutexRecursive
    ringbuf.c: ~ RTT Semaphore
    - xQueueGenericReceive
    - xQueueGenericSend
    timers.c
    - xQueueGenericCreate
    - xQueueGenericSendFromISR
    - vQueueWaitForMessageRestricted
    - xQueueGenericSend
    - xQueueGenericReceive
    vfs.c: ~ RTT Semaphore
    - xQueueGiveFromISR
    - xQueueGenericSend
    uart.c: ~ RTT Mutex (Non-recursive) + Semaphore (binary)
    - xQueueGenericReceive
    - xQueueGenericSend
    pthread.c: ~ RTT Mutex (Non-recursive)
    - xQueueCreateMutex
    sys_arch.c:
    - uxQueueMessagesWaiting (TODO)
    sha.c:
    - uxSemaphoreGetCount
    i2s.c
    - uxQueueSpacesAvailable (TODO)
 */

SemaphoreHandle_t wrap_mutex_create(const char *name) {
    SemaphoreHandle_t handle;

    handle = (SemaphoreHandle_t)rt_malloc(sizeof(generic_ipc_t));
    if (!handle) return NULL;

    handle->type = RTT_MUTEX;
    handle->maxCnt = 0;
    handle->cntr = NULL;
    if (RT_EOK != rt_mutex_init(&handle->member.mux, name, RT_IPC_FLAG_FIFO)) {
        rt_free(handle);
        return NULL;
    }

    return handle;
}

SemaphoreHandle_t wrap_bin_sem_create(const char *name,
    const UBaseType_t uxInitialCount) {
    SemaphoreHandle_t handle;

    handle = (SemaphoreHandle_t)rt_malloc(sizeof(generic_ipc_t));
    if (!handle) return NULL;

    handle->type = RTT_SEMAPHORE;
    handle->maxCnt = 1;
    handle->cntr = NULL;
    if (RT_EOK != rt_sem_init(&handle->member.sem, name, uxInitialCount,
        RT_IPC_FLAG_FIFO)) {
        rt_free(handle);
        return NULL;
    }

    return handle;
}

SemaphoreHandle_t wrap_cnt_sem_create(const char *name,
    const UBaseType_t uxMaxCount, const UBaseType_t uxInitialCount) {
    SemaphoreHandle_t handle;

    handle = (SemaphoreHandle_t)rt_malloc(sizeof(generic_ipc_t));
    if (!handle) return NULL;

    handle->type = RTT_SEMAPHORE;
    handle->maxCnt = uxMaxCount;
    handle->cntr = NULL;
    if (RT_EOK != rt_sem_init(&handle->member.sem, name, uxInitialCount,
        RT_IPC_FLAG_FIFO)) {
        rt_free(handle);
        return NULL;
    }

    return handle;
}

SemaphoreHandle_t wrap_cnt_sem_init(const char *name,
    const UBaseType_t uxMaxCount, const UBaseType_t uxInitialCount,
    StaticSemaphore_t *pxStaticQueue) {
    SemaphoreHandle_t handle;

    handle = (SemaphoreHandle_t)pxStaticQueue;
    if (!handle) return NULL;

    handle->type = RTT_SEMAPHORE;
    handle->maxCnt = uxMaxCount;
    handle->cntr = NULL;
    if (RT_EOK != rt_sem_init(&handle->member.sem, name,
        uxInitialCount, RT_IPC_FLAG_FIFO))
        return NULL;

    return handle;
}

BaseType_t wrap_ipc_take(SemaphoreHandle_t handle, TickType_t ticks) {
    rt_err_t ret;

    if (RTT_MUTEX == handle->type) {
        ret = rt_mutex_take(&handle->member.mux, (rt_int32_t)ticks);
    } else if (RTT_SEMAPHORE == handle->type) {
        ret = rt_sem_take(&handle->member.sem, (rt_int32_t)ticks);
    } else {
        ret = RT_EINVAL;
    }

    return (RT_EOK == ret) ? pdTRUE : errQUEUE_EMPTY;
}

BaseType_t wrap_ipc_take_in_isr(SemaphoreHandle_t handle,
    BaseType_t * const pxHigherPriorityTaskWoken) {
    rt_err_t ret;

    if (RTT_MUTEX == handle->type) {
        ret = RT_EINVAL;
    } else if (RTT_SEMAPHORE == handle->type) {
        ret = rt_sem_take(&handle->member.sem, 0);
    } else {
        ret = RT_EINVAL;
    }

    *pxHigherPriorityTaskWoken = (RT_EOK == ret) ? pdTRUE : pdFALSE;    // TODO
    return (RT_EOK == ret) ? pdTRUE : errQUEUE_EMPTY;
}

BaseType_t wrap_ipc_give(SemaphoreHandle_t handle) {
    rt_err_t ret;

    if (RTT_MUTEX == handle->type) {
        ret = rt_mutex_release(&handle->member.mux);
    } else if (RTT_SEMAPHORE == handle->type) {
        if (handle->member.sem.value >= handle->maxCnt) {
            ret = RT_EFULL;
        } else {
            ret = rt_sem_release(&handle->member.sem);
        }
    } else {
        ret = RT_EINVAL;
    }

    return (RT_EOK == ret) ? pdTRUE : errQUEUE_FULL;
}

BaseType_t wrap_ipc_give_in_isr(SemaphoreHandle_t handle,
    BaseType_t * const pxHigherPriorityTaskWoken) {
    rt_err_t ret;

    if (RTT_MUTEX == handle->type) {
        ret = RT_EINVAL;
    } else if (RTT_SEMAPHORE == handle->type) {
        if (handle->member.sem.value >= handle->maxCnt) {
            ret = RT_EFULL;
        } else {
            ret = rt_sem_release(&handle->member.sem);
        }
    } else {
        ret = RT_EINVAL;
    }

    *pxHigherPriorityTaskWoken = (RT_EOK == ret) ? pdTRUE : pdFALSE;    // TODO
    return (RT_EOK == ret) ? pdTRUE : errQUEUE_FULL;
}

void wrap_ipc_delete(SemaphoreHandle_t handle) {
    if (RTT_MUTEX == handle->type) {
        (void)rt_mutex_detach(&handle->member.mux);
    } else if (RTT_SEMAPHORE == handle->type) {
        (void)rt_sem_detach(&handle->member.sem);
    } else {
        return;
    }

    rt_free(handle);
}

void *wrap_ipc_get_owner(SemaphoreHandle_t handle) {
    if (RTT_MUTEX == handle->type) {
        return handle->member.mux.owner;
    } else /*if (RTT_SEMAPHORE == handle->type)*/ {
        return NULL;
    }
}

/*
    can.c: ~ RTT MQ
    - xQueueCreate
    - xQueueReset
    - xQueueSend
    - xQueueReceive
    - xQueueSendFromISR
    - xQueueReceiveFromISR
    - vQueueDelete
    i2c.c
    - xQueueOverwriteFromISR
    i2s.c
    - xQueueIsQueueFullFromISR
 */

QueueHandle_t wrap_mq_create(const char *name, const UBaseType_t uxQueueLength,
    const UBaseType_t uxItemSize) {
    QueueHandle_t handle = NULL;
    void *msgpool = NULL;

    do {
        rt_size_t msg_size = RT_ALIGN(uxItemSize, RT_ALIGN_SIZE);
        rt_size_t pool_size = (msg_size + 4) * uxQueueLength;

        if (NULL == (handle = (QueueHandle_t)rt_malloc(sizeof(generic_ipc_t))))
            break;
        if ((msgpool = rt_malloc(pool_size)))
            break;

        handle->type = RTT_QUEUE;
        if (RT_EOK != rt_mq_init(&handle->member.mq, name, msgpool, msg_size, 
            pool_size, RT_IPC_FLAG_FIFO))
            break;
        handle->maxCnt = handle->member.mq.max_msgs;
        handle->cntr = NULL;
        return handle;
    } while (0);

    if (msgpool) rt_free(msgpool);
    if (handle) rt_free(handle);
    return NULL;
}

BaseType_t wrap_mq_reset(QueueHandle_t handle) {
    if (RTT_QUEUE == handle->type) {
        if (RT_EOK != rt_mq_control(&handle->member.mq, RT_IPC_CMD_RESET, NULL))
            return pdFALSE;
        else
            return pdPASS;

    } else {
        return pdFAIL;
    }
}

BaseType_t wrap_mq_send(QueueHandle_t handle, const void * const item,
    TickType_t ticks, uint8_t is_urgent) {
    (void)ticks;

    if (RTT_QUEUE == handle->type) {
        if (!is_urgent) {
            if (RT_EOK != rt_mq_send(
                &handle->member.mq, (void *)item, handle->member.mq.msg_size))
                return errQUEUE_FULL;
            else
                return pdPASS;
        } else {
            if (RT_EOK != rt_mq_urgent(
                &handle->member.mq, (void *)item, handle->member.mq.msg_size))
                return errQUEUE_FULL;
            else
                return pdPASS;
        }
    } else {
        return pdFAIL;
    }
}

BaseType_t wrap_mq_send_in_isr(QueueHandle_t handle, const void * const item,
    BaseType_t * const pxHigherPriorityTaskWoken) {
    BaseType_t ret;

    if (RTT_QUEUE == handle->type) {
        if (RT_EOK != rt_mq_send(
            &handle->member.mq, (void *)item, handle->member.mq.msg_size)) {
            // *pxHigherPriorityTaskWoken
            ret = errQUEUE_FULL;
        } else {
            ret = pdPASS;
        }
    } else {
        ret = pdFAIL;
    }

    *pxHigherPriorityTaskWoken = (RT_EOK == ret) ? pdTRUE : pdFALSE;    // TODO
    return ret;
}

BaseType_t wrap_mq_recv(QueueHandle_t handle, void * const buffer,
    TickType_t ticks) {
    BaseType_t ret;

    if (RTT_QUEUE == handle->type) {
        if (RT_EOK != rt_mq_recv(
            &handle->member.mq, buffer, handle->member.mq.msg_size, ticks))
            ret = errQUEUE_EMPTY;
        else
            ret = pdPASS;
    } else {
        ret = pdFAIL;
    }

    return ret;
}

BaseType_t wrap_mq_recv_in_isr(QueueHandle_t handle, void * const buffer,
    BaseType_t * const pxHigherPriorityTaskWoken) {
    BaseType_t ret;

    if (RTT_QUEUE == handle->type) {
        if (RT_EOK != rt_mq_recv(
            &handle->member.mq, buffer, handle->member.mq.msg_size, 0))
            ret = errQUEUE_EMPTY;
        else
            ret = pdPASS;

    } else {
        ret = pdFAIL;
    }

    *pxHigherPriorityTaskWoken = ret;   // TODO
    return ret;
}

BaseType_t wrap_mq1_replace_in_isr(QueueHandle_t handle,
    const void * const item, BaseType_t * const pxHigherPriorityTaskWoken) {
    BaseType_t ret;

    if (RTT_QUEUE == handle->type) {
        if (handle->member.mq.max_msgs != 1) {
            ret = pdFAIL;
        } else {
            register rt_ubase_t temp;

            /* disable interrupt */
            temp = rt_hw_interrupt_disable();
            if (!handle->member.mq.entry) {
                /* empty */
                /* enable interrupt */
                rt_hw_interrupt_enable(temp);
                return wrap_mq_send_in_isr(handle, item,
                    pxHigherPriorityTaskWoken);
            } else {
                /* full then replace */
                void *msg;
                /* get the first item */
                msg = handle->member.mq.msg_queue_head;
                rt_memcpy(msg + 1, item, handle->member.mq.msg_size);
                rt_hw_interrupt_enable(temp);
                ret = pdPASS;
            }
        }
    } else {
        ret = pdFAIL;
    }

    *pxHigherPriorityTaskWoken = ret;   // TODO
    return ret;
}

void wrap_mq_delete(QueueHandle_t handle) {
    if (RTT_QUEUE == handle->type) {
        (void)rt_mq_detach(&handle->member.mq);
    }

    rt_free(handle->member.mq.msg_pool);
    rt_free(handle);
}

UBaseType_t wrap_mq_entries(const QueueHandle_t handle) {
    BaseType_t ret;

    if (RTT_QUEUE == handle->type) {
        register rt_ubase_t temp;
        /* disable interrupt */
        temp = rt_hw_interrupt_disable();
        ret = handle->member.mq.entry;
        /* enable interrupt */
        rt_hw_interrupt_enable(temp);
    } else {
        ret = 0;
    }

    return ret;
}

BaseType_t wrap_mq_is_full(const QueueHandle_t handle) {
    BaseType_t ret;

    if (RTT_QUEUE == handle->type) {
        register rt_ubase_t temp;
        /* disable interrupt */
        temp = rt_hw_interrupt_disable();
        ret = (NULL == handle->member.mq.msg_queue_free) ? pdTRUE : pdFALSE;
        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

    } else {
        ret = pdFAIL;
    }

    return ret;
}


QueueSetHandle_t wrap_qset_create(const UBaseType_t length) {
    QueueSetHandle_t handle;

    RT_DEBUG_NOT_IN_INTERRUPT;

    handle = rt_malloc(sizeof(QueueSet_t));
    if (!handle) return NULL;

    rt_slist_init(&(handle->member));
    handle->length = length;

    return handle;
}

/*
    ringbuf.c
    - xQueueAddToSet
    - xQueueRemoveFromSet
 */
BaseType_t wrap_qset_add(QueueSetMemberHandle_t qOrSem_handle,
    QueueSetHandle_t handle) {
    register rt_base_t temp;

    /* parameter check */
    RT_ASSERT(qOrSem_handle != RT_NULL);
    RT_ASSERT(handle != RT_NULL);

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    RT_DEBUG_LOG(RT_DEBUG_IPC, ("thread %s add %s(%d) to qset:%p",
        rt_thread_self()->name,
        ((struct rt_object *)&(qOrSem_handle->member))->name,
        qOrSem_handle->type,
        handle));

    /* Cannot add a queue/semaphore to more than one queue set. */
    if (qOrSem_handle->cntr) {
        rt_hw_interrupt_enable(temp);
        return pdFAIL;
    }

    /* Cannot add a queue/semaphore to a queue set if there are already
    items in the queue/semaphore. */
    if (RTT_SEMAPHORE == qOrSem_handle->type) {
        /* no way to know */
    } else if (RTT_QUEUE == qOrSem_handle->type) {
        if (qOrSem_handle->member.mq.entry) {
            rt_hw_interrupt_enable(temp);
            return pdFAIL;
        }
    } else {
        /* wrong type */
        rt_hw_interrupt_enable(temp);
        return pdFAIL;
    }

    qOrSem_handle->cntr = handle;

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);
    return pdPASS;
}

BaseType_t wrap_qset_remove(QueueSetMemberHandle_t qOrSem_handle,
    QueueSetHandle_t handle) {
    register rt_base_t temp;

    /* parameter check */
    RT_ASSERT(qOrSem_handle != RT_NULL);
    RT_ASSERT(handle != RT_NULL);

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    RT_DEBUG_LOG(RT_DEBUG_IPC, ("thread %s remove %s(%d) from qset:%p",
        rt_thread_self()->name,
        ((struct rt_object *)&(qOrSem_handle->member))->name,
        qOrSem_handle->type,
        handle));

    /* The queue was not a member of the set. */
    if (qOrSem_handle->cntr != handle) {
        rt_hw_interrupt_enable(temp);
        return pdFAIL;
    }

    /* It is dangerous to remove a queue from a set when the queue is
    not empty because the queue set will still hold pending events for
    the queue. */
    if (RTT_SEMAPHORE == qOrSem_handle->type) {
        /* no way to know */
    } else if (RTT_QUEUE == qOrSem_handle->type) {
        if (qOrSem_handle->member.mq.entry) {
            rt_hw_interrupt_enable(temp);
            return pdFAIL;
        }
    } else {
        /* wrong type */
        rt_hw_interrupt_enable(temp);
        return pdFAIL;
    }

    qOrSem_handle->cntr = NULL;

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);
    return pdPASS;
}





void vTaskStartScheduler(void) {
    /* disable interrupt*/
    rt_hw_interrupt_disable();

    /* init tick */
    rt_system_tick_init();

    /* init kernel object */
    rt_system_object_init();

    /* init timer */
    rt_system_timer_init();

    /* init scheduler */
    rt_system_scheduler_init();

    /* init signal */
    #ifdef RT_USING_SIGNALS
    rt_system_signal_init();
    #endif

    /* init driver */
    // rt_driver_init();

    /* show version */
    rt_show_version();

    /* init timer thread */
    rt_system_timer_thread_init();

    /* init idle thread */
    rt_thread_idle_init();

    /* init application */
    // rt_application_init();

    /* start scheduler */
    rt_system_scheduler_start();
}


int rt_hw_cpu_id(void) {
    return xPortGetCoreID();
}

// void rt_thread_yield(void) {
//     if (port_interruptNesting) {
//         _frxt_setup_switch();
//         // portYIELD_FROM_ISR();
//     } else {
//         portYIELD();
//     }
// }

rt_base_t rt_hw_local_irq_disable(void) {
    return XTOS_SET_INTLEVEL(XCHAL_EXCM_LEVEL);
}

void rt_hw_local_irq_enable(rt_base_t level) {
    XTOS_RESTORE_JUST_INTLEVEL(level);
}

void rt_hw_spin_lock(rt_hw_spinlock_t *lock) {
    vPortCPUAcquireMutex(lock);
}
void rt_hw_spin_unlock(rt_hw_spinlock_t *lock) {
    vPortCPUReleaseMutex(lock);
}

void rt_hw_secondary_cpu_idle_exec(void) {
    // TODO:
 }

/**
 * This function will initialize thread stack
 *
 * @param tentry the entry of thread
 * @param parameter the parameter of entry
 * @param stack_addr the beginning stack address
 * @param texit the function will be called when thread exit
 *
 * @return stack address
 */
rt_uint8_t *rt_hw_stack_init(void *entry, void *parameter,
    rt_uint8_t *topOfStack, void *exit_) {
    // TODO：exit_, xRunPrivileged
    #if (portUSING_MPU_WRAPPERS == 1)
    {
        return pxPortInitialiseStack(topOfStack, entry, parameter, 0);
    }
    #else /* portUSING_MPU_WRAPPERS */
    {
        return pxPortInitialiseStack(topOfStack, entry, parameter);
    }
    #endif /* portUSING_MPU_WRAPPERS */
}

struct rt_thread * volatile pxCurrentTCB[ portNUM_PROCESSORS ];

void rt_hw_ipi_send(int ipi_vector, unsigned int cpu_mask) {
    unsigned int id;

    for (id = 0; id < portNUM_PROCESSORS; id++) {
        if (cpu_mask & (1 << id)) {
            esp_crosscore_int_send_yield(id);
            break;
        }
    }
}

BaseType_t xPortSysTickHandler(void) {
    portbenchmarkIntLatency();
    traceISR_ENTER(SYSTICK_INTR_ID);

    // if ( xPortInIsrContext() ) {
    //     #if ( configUSE_TICK_HOOK == 1 )
    //     vApplicationTickHook();
    //     #endif /* configUSE_TICK_HOOK */
    //     #if ( CONFIG_FREERTOS_LEGACY_HOOKS == 1 )
    //     esp_vApplicationTickHook();
    //     #endif /* CONFIG_FREERTOS_LEGACY_HOOKS */
    // }

    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();

    return 0;
}

/*
    rt_tick_increase
    - cpu->tick++
    - if (thread->remaining_tick == 0) rt_thread_yield();
    - rt_timer_check();
*/


// TODO: change value when scheduler started
unsigned rttSchedulerState = taskSCHEDULER_NOT_STARTED;

BaseType_t xTaskGetSchedulerState(void) {
    if (rttSchedulerState == taskSCHEDULER_NOT_STARTED) {
        return taskSCHEDULER_NOT_STARTED;
    }
    if (rt_thread_self()->scheduler_lock_nest == 0) {
        return taskSCHEDULER_RUNNING;
    }
    return taskSCHEDULER_SUSPENDED;
}

TaskHandle_t xTaskGetCurrentTaskHandle(void) {
    return rt_thread_self();
}

TaskHandle_t xTaskGetCurrentTaskHandleForCPU(BaseType_t cpuid) {
    rt_base_t lock;
    rt_thread_t self;

    lock = rt_hw_local_irq_disable();
    self = rt_cpu_index(cpuid)->current_thread;
    rt_hw_local_irq_enable(lock);
    return self;
}

UBaseType_t uxTaskPriorityGet(TaskHandle_t xTask) {
    return (UBaseType_t)(rt_thread_self()->current_priority);  // rt_uint8_t
}

// rt_err_t rt_thread_control(rt_thread_t thread, int cmd, void *arg);
void vTaskPrioritySet(TaskHandle_t xTask, UBaseType_t uxNewPriority) {
    (void)rt_thread_control(xTask, RT_THREAD_CTRL_CHANGE_PRIORITY,
        &uxNewPriority);
}

char *pcTaskGetTaskName(TaskHandle_t xTaskToQuery) {
    return &(rt_thread_self()->name[0]);
}


/* --- contex switch --- */

// Called by "rt_schedule2()" with interrupt disabled
// Set pxCurrentTCB
void rt_hw_context_switch(rt_ubase_t from_sp, rt_ubase_t to_sp,
    struct rt_thread *to_thread) {
    (void)from_sp;
    (void)to_sp;
    // rt_hw_spin_lock(&_rt_critical_lock);
    pxCurrentTCB[xPortGetCoreID()] = to_thread;
    // rt_hw_spin_unlock(&_rt_critical_lock);
    portYIELD_WITHIN_API();
}

// Called by "rt_system_scheduler_start()" with interrupt disabled
void rt_hw_context_switch_to(rt_ubase_t to_sp, struct rt_thread *to_thread) {
    rt_hw_context_switch(0, to_sp, to_thread);
    // __asm__ volatile ("call0    _frxt_dispatch\n");
}

// This function is not used
void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from_sp,
    rt_ubase_t to_sp, struct rt_thread *to_thread) {
    (void)context;
    (void)from_sp;
    (void)to_sp;
    (void)to_thread;
    RT_ASSERT(0);
}

void rt_schedule(void) {
    portYIELD_WITHIN_API();
}

extern void rt_schedule2(void);

// Called by "_frxt_dispatch()"
// Call "rt_schedule2()" to set pxCurrentTCB
void vTaskSwitchContext(void) {
    rt_schedule2();
}


/* --- task / thread --- */

/* Value that can be assigned to the eNotifyState member of the TCB. */
typedef enum {
    eNotWaitingNotification = 0,
    eWaitingNotification,
    eNotified
} eNotifyValue;

/*
 * Task control block.  A task control block (TCB) is allocated for each task,
 * and stores task state information, including a pointer to the task's context
 * (the task's run time environment, including register values)
 */
typedef struct tskTaskControlBlock {
    volatile StackType_t    *pxTopOfStack;  /*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

    #if (portUSING_MPU_WRAPPERS == 1)
        xMPU_SETTINGS   xMPUSettings;       /*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE TCB STRUCT. */
    #endif

    StackType_t         *pxStack;           /*< Points to the start of the stack. */

    #if (configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0)
        void *pvThreadLocalStoragePointers[configNUM_THREAD_LOCAL_STORAGE_POINTERS];
    # if (configTHREAD_LOCAL_STORAGE_DELETE_CALLBACKS)
        TlsDeleteCallbackFunction_t pvThreadLocalStoragePointersDelCallback[configNUM_THREAD_LOCAL_STORAGE_POINTERS];
    # endif
    #endif

    #if (configUSE_NEWLIB_REENTRANT == 1)
        struct  _reent xNewLib_reent;
    #endif

    rt_thread_t pxThread;

    #if (configUSE_TASK_NOTIFICATIONS == 1)
        rt_sem_t pxNotify;
        // volatile uint32_t ulNotifiedValue;
        volatile eNotifyValue eNotifyState;
    #endif
} tskTCB;

typedef tskTCB TCB_t;


// TODO: PRIVILEGED_DATA static portMUX_TYPE xTaskQueueMutex = portMUX_INITIALIZER_UNLOCKED;

#define tcbOfThread(t)  rt_container_of(t, TCB_t, pxThread)

#define prvGetTCBFromHandle(pxHandle) (((pxHandle) == NULL) ? tcbOfThread(rt_thread_self()) : (TCB_t *)(pxHandle))

/* --- configUSE_NEWLIB_REENTRANT --- */

#if (configUSE_NEWLIB_REENTRANT == 1)
//Return global reent struct if FreeRTOS isn't running,
struct _reent* __getreent(void) {
    //No lock needed because if this changes, we won't be running anymore.
    rt_thread_t self = rt_thread_self();    // TODO: when return NULL?
    if (self == NULL) {
        //No task running. Return global struct.
        return _GLOBAL_REENT;
    } else {
        //We have a task; return its reentrant struct.
        return &(tcbOfThread(self)->xNewLib_reent);
    }
}
#endif

// rt_thread_t rt_thread_create(const char *name,
//                              void (*entry)(void *parameter),
//                              void       *parameter,
//                              rt_uint32_t stack_size,
//                              rt_uint8_t  priority,
//                              rt_uint32_t tick)
// rt_sem_t rt_sem_create(const char *name, rt_uint32_t value, rt_uint8_t flag)
BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t pxTaskCode,
    const char * const pcName,
    const uint32_t usStackDepth,
    void * const pvParameters,
    UBaseType_t uxPriority,
    TaskHandle_t * const pxCreatedTask,
    const BaseType_t xCoreID) {
    TCB_t *tcb;
    UBaseType_t x;
    rt_uint32_t tick = 100;  // TODO:

    if (NULL == (tcb = (TCB_t *)rt_malloc(sizeof(TCB_t)))) {
        return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    }
    (void)rt_memset(tcb, 0x00, sizeof(TCB_t));

    #if (configNUM_THREAD_LOCAL_STORAGE_POINTERS != 0)
    {
        for (x = 0; x < (UBaseType_t) configNUM_THREAD_LOCAL_STORAGE_POINTERS; x++) {
            tcb->pvThreadLocalStoragePointers[x] = NULL;
            #if (configTHREAD_LOCAL_STORAGE_DELETE_CALLBACKS == 1)
                tcb->pvThreadLocalStoragePointersDelCallback[x] = NULL;
            #endif
        }
    }
    #endif

    if (NULL == (tcb->pxNotify = rt_sem_create(pcName, 0, RT_IPC_FLAG_FIFO))) {
        vTaskDelete((TaskHandle_t)tcb);
        return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    }
    tcb->eNotifyState = eNotWaitingNotification;

    #if (configUSE_NEWLIB_REENTRANT == 1)
        /* Initialise this task's Newlib reent structure. */
        esp_reent_init(&(tcb->xNewLib_reent));
    #endif

    if (NULL == (tcb->pxThread = rt_thread_create(
        pcName, pxTaskCode, pvParameters, usStackDepth, uxPriority, tick))) {
        vTaskDelete((TaskHandle_t)tcb);
        return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    }

    if (RT_EOK != rt_thread_control(
        tcb->pxThread, RT_THREAD_CTRL_BIND_CPU, (void *)xCoreID)) {
        vTaskDelete((TaskHandle_t)tcb);
        return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    }
 
    *pxCreatedTask = (TaskHandle_t)tcb;
    return pdPASS;
}

// rt_err_t rt_thread_delete(rt_thread_t thread)
void vTaskDelete(TaskHandle_t xTaskToDelete) {
    TCB_t *tcb = (TCB_t *)xTaskToDelete;

    if (NULL != tcb->pxThread) {
        (void)rt_thread_delete(tcb->pxThread);
    }

    if (NULL != tcb->pxNotify) {
        (void)rt_sem_delete(tcb->pxNotify);
    }

    #if (configUSE_NEWLIB_REENTRANT == 1)
        _reclaim_reent(&(tcb->xNewLib_reent));
    #endif

    rt_free(tcb);
}

// rt_err_t rt_thread_suspend(rt_thread_t thread)
void vTaskSuspend(TaskHandle_t xTaskToSuspend) {
    TCB_t *tcb;

    tcb = prvGetTCBFromHandle(xTaskToSuspend);
    (void)rt_thread_suspend(tcb->pxThread);
}

void vTaskResume(TaskHandle_t xTaskToResume) {
    TCB_t *tcb;

    tcb = (TCB_t *)xTaskToResume;
    (void)rt_thread_resume(tcb->pxThread);
}

// rt_base_t rt_cpus_lock(void)
// void rt_enter_critical(void)
void vTaskSuspendAll(void) {
    rt_enter_critical();
}

// void rt_exit_critical(void);
BaseType_t xTaskResumeAll(void) {
    rt_exit_critical();

    return pdFALSE; // TODO: return ?
}

// rt_err_t rt_sem_take(rt_sem_t sem, rt_int32_t time)
uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit, TickType_t xTicksToWait) {
    register rt_base_t temp;
    TCB_t *tcb;
    uint32_t value;

    tcb = tcbOfThread(rt_thread_self());

    // ENTER_CRITICAL
    temp = rt_hw_interrupt_disable();
    value = tcb->pxNotify->value;
    if (value == 0UL) {
        tcb->eNotifyState = eWaitingNotification;
    }
    // EXIT_CRITICAL
    rt_hw_interrupt_enable(temp);

    if ((value == 0UL) && (xTicksToWait > (TickType_t)0)) {
        rt_sem_take(tcb->pxNotify, xTicksToWait);
    }

    // ENTER_CRITICAL
    temp = rt_hw_interrupt_disable();
    value = tcb->pxNotify->value;
    tcb->eNotifyState = eNotWaitingNotification;
    // EXIT_CRITICAL
    rt_hw_interrupt_enable(temp);

    if (xClearCountOnExit != pdFALSE) {
        (void)rt_sem_control(tcb->pxNotify, RT_IPC_CMD_RESET, 0);
    }

    return value;
}

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit, uint32_t *pulNotificationValue, TickType_t xTicksToWait) {
    register rt_base_t temp;
    TCB_t *tcb;
    eNotifyValue notifyState;
    uint32_t value;
    BaseType_t xReturn;

    tcb = tcbOfThread(rt_thread_self());

    // taskENTER_CRITICAL(&xTaskQueueMutex);
    temp = rt_hw_interrupt_disable();
    notifyState = tcb->eNotifyState;
    tcb->eNotifyState = eWaitingNotification;
    value = tcb->pxNotify->value;
    // taskEXIT_CRITICAL(&xTaskQueueMutex);
    rt_hw_interrupt_enable(temp);

    if (notifyState != eNotified) {
        (void)rt_sem_control(tcb->pxNotify, RT_IPC_CMD_RESET, (void *)(value & ~ulBitsToClearOnEntry));
        if (xTicksToWait > (TickType_t)0) {
            rt_sem_take(tcb->pxNotify, xTicksToWait);
        }
    }

    // taskENTER_CRITICAL(&xTaskQueueMutex);
    temp = rt_hw_interrupt_disable();
    notifyState = tcb->eNotifyState;
    tcb->eNotifyState = eNotWaitingNotification;
    value = tcb->pxNotify->value;
    // taskEXIT_CRITICAL(&xTaskQueueMutex);
    rt_hw_interrupt_enable(temp);

    if (pulNotificationValue != NULL) {
        /* Output the current notification value, which may or may not
        have changed. */
        *pulNotificationValue = value;
    }

    /* If eNotifyValue is set then either the task never entered the
    blocked state (because a notification was already pending) or the
    task unblocked because of a notification.  Otherwise the task
    unblocked because of a timeout. */
    if (notifyState == eWaitingNotification) {
        /* A notification was not received. */
        xReturn = pdFALSE;
    } else {
        /* A notification was already pending or a notification was
        received while the task was waiting. */
        (void)rt_sem_control(tcb->pxNotify, RT_IPC_CMD_RESET, (void *)(value & ~ulBitsToClearOnExit));
        xReturn = pdTRUE;
    }

    return xReturn;
}

BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction) {
    register rt_base_t temp;
    TCB_t *tcb = (TCB_t *)xTaskToNotify;
    eNotifyValue eOriginalNotifyState;
    BaseType_t xReturn = pdPASS;
    uint32_t value;

    // taskENTER_CRITICAL(&xTaskQueueMutex);
    temp = rt_hw_interrupt_disable();
    eOriginalNotifyState = tcb->eNotifyState;
    tcb->eNotifyState = eNotified;
    value = tcb->pxNotify->value;
    // taskEXIT_CRITICAL(&xTaskQueueMutex);
    rt_hw_interrupt_enable(temp);

    switch (eAction) {
    case eSetBits:
        (void)rt_sem_control(tcb->pxNotify, RT_IPC_CMD_RESET, (void *)(value | ulValue));
        break;

    case eIncrement :
        (void)rt_sem_release(tcb->pxNotify);
        break;

    case eSetValueWithOverwrite :
        (void)rt_sem_control(tcb->pxNotify, RT_IPC_CMD_RESET, (void *)ulValue);
        break;

    case eSetValueWithoutOverwrite :
        if (eOriginalNotifyState != eNotified) {
            (void)rt_sem_control(tcb->pxNotify, RT_IPC_CMD_RESET, (void *)ulValue);
        } else {
            /* The value could not be written to the task. */
            xReturn = pdFAIL;
        }
        break;

    case eNoAction:
        /* The task is being notified without its notify value being
        updated. */
        break;
    }

    return xReturn;
}

void vTaskNotifyGiveFromISR(TaskHandle_t xTaskToNotify, BaseType_t *pxHigherPriorityTaskWoken) {
    register rt_base_t temp;
    TCB_t *tcb = (TCB_t *)xTaskToNotify;
    rt_thread_t currentThread;
    
    currentThread = rt_thread_self();

    // ENTER_CRITICAL
    temp = rt_hw_interrupt_disable();
    if (tcb->pxThread->current_priority > currentThread->current_priority) {
        /* The notified task has a priority above the currently executing task
           so a yield is required. */
        if (pxHigherPriorityTaskWoken != NULL) {
            *pxHigherPriorityTaskWoken = pdTRUE;
        }

    }
    // EXIT_CRITICAL
    rt_hw_interrupt_enable(temp);
}

#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS != 0)

#if (configTHREAD_LOCAL_STORAGE_DELETE_CALLBACKS)

void vTaskSetThreadLocalStoragePointerAndDelCallback(TaskHandle_t xTaskToSet, BaseType_t xIndex, void *pvValue , TlsDeleteCallbackFunction_t xDelCallback) {
    register rt_base_t temp;
    TCB_t *pxTCB;

    if (xIndex < configNUM_THREAD_LOCAL_STORAGE_POINTERS) {
        // taskENTER_CRITICAL(&xTaskQueueMutex);
        temp = rt_hw_interrupt_disable();
        pxTCB = prvGetTCBFromHandle(xTaskToSet);
        pxTCB->pvThreadLocalStoragePointers[xIndex] = pvValue;
        pxTCB->pvThreadLocalStoragePointersDelCallback[xIndex] = xDelCallback;
        // taskEXIT_CRITICAL(&xTaskQueueMutex);
        rt_hw_interrupt_enable(temp);
    }
}

void vTaskSetThreadLocalStoragePointer(TaskHandle_t xTaskToSet, BaseType_t xIndex, void *pvValue) {
    vTaskSetThreadLocalStoragePointerAndDelCallback(xTaskToSet, xIndex, pvValue, (TlsDeleteCallbackFunction_t)NULL);
}

#else
    void vTaskSetThreadLocalStoragePointer(TaskHandle_t xTaskToSet, BaseType_t xIndex, void *pvValue) {
    register rt_base_t temp;
    TCB_t *pxTCB;

        if (xIndex < configNUM_THREAD_LOCAL_STORAGE_POINTERS) {
            // taskENTER_CRITICAL(&xTaskQueueMutex);
            temp = rt_hw_interrupt_disable();
            pxTCB = prvGetTCBFromHandle(xTaskToSet);
            pxTCB->pvThreadLocalStoragePointers[xIndex] = pvValue;
            // taskEXIT_CRITICAL(&xTaskQueueMutex);
            rt_hw_interrupt_enable(temp);
        }
    }
#endif /* configTHREAD_LOCAL_STORAGE_DELETE_CALLBACKS */

void *pvTaskGetThreadLocalStoragePointer(TaskHandle_t xTaskToQuery, BaseType_t xIndex) {
    void *pvReturn = NULL;
    TCB_t *tcb;

    if (xIndex < configNUM_THREAD_LOCAL_STORAGE_POINTERS) {
        tcb = prvGetTCBFromHandle(xTaskToQuery);
        pvReturn = tcb->pvThreadLocalStoragePointers[xIndex];
    } else {
        pvReturn = NULL;
    }

    return pvReturn;
}

#endif /* configNUM_THREAD_LOCAL_STORAGE_POINTERS */


void vTaskDelay(const TickType_t xTicksToDelay) {
    (void)rt_thread_sleep(xTicksToDelay);
}

void vTaskDelayUntil(TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement) {
    register rt_base_t temp;
    TickType_t xTimeToWake;
    TickType_t xTimeToSleep = 0;

    RT_ASSERT(pxPreviousWakeTime);
    RT_ASSERT((xTimeIncrement > 0U));
    // RT_ASSERT( xTaskGetSchedulerState() != taskSCHEDULER_SUSPENDED );

    // taskENTER_CRITICAL(&xTaskQueueMutex);
    temp = rt_hw_interrupt_disable();
    {
        /* Minor optimisation.  The tick count cannot change in this
        block. */
        const TickType_t xConstTickCount = rt_tick_get();

        /* Generate the tick time at which the task wants to wake. */
        xTimeToWake = *pxPreviousWakeTime + xTimeIncrement;

        if (xConstTickCount < *pxPreviousWakeTime) {
            /* The tick count has overflowed since this function was
            lasted called.  In this case the only time we should ever
            actually delay is if the wake time has also overflowed,
            and the wake time is greater than the tick time.  When this
            is the case it is as if neither time had overflowed. */
            if ((xTimeToWake < *pxPreviousWakeTime) && (xTimeToWake > xConstTickCount)) {
                xTimeToSleep = xTimeToWake - xConstTickCount;
            }
        } else {
            /* The tick time has not overflowed.  In this case we will
            delay if either the wake time has overflowed, and/or the
            tick time is less than the wake time. */
            if ((xTimeToWake < *pxPreviousWakeTime) || (xTimeToWake > xConstTickCount)) {
                xTimeToSleep = xTimeToWake - xConstTickCount;
            }
        }

        /* Update the wake time ready for the next call. */
        *pxPreviousWakeTime = xTimeToWake;
    }
    // taskEXIT_CRITICAL(&xTaskQueueMutex);
    rt_hw_interrupt_enable(temp);

    if (xTimeToSleep != 0) {
        (void)rt_thread_sleep(xTimeToSleep);
    }
}

#define tskSTACK_FILL_BYTE  (0x23U)


static uint32_t prvTaskCheckFreeStackSpace(const uint8_t * pucStackByte) {
    uint32_t ulCount = 0U;

    while (*pucStackByte == (uint8_t )tskSTACK_FILL_BYTE) {
        pucStackByte -= portSTACK_GROWTH;
        ulCount++;
    }

    ulCount /= (uint32_t)sizeof(StackType_t); /*lint !e961 Casting is not redundant on smaller architectures. */

    return (uint32_t)ulCount;
}

UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t xTask) {
    TCB_t *tcb;
    uint8_t *pucEndOfStack;
    UBaseType_t uxReturn;

    tcb = prvGetTCBFromHandle(xTask);

    #if portSTACK_GROWTH < 0
    // TODO: #ifndef ARCH_CPU_STACK_GROWS_UPWARD
    {
        pucEndOfStack = (uint8_t *)tcb->pxThread->stack_addr;
    }
    #else
    {
        pucEndOfStack = (uint8_t *)tcb->pxThread->stack_addr + tcb->pxThread->stack_size - sizeof(rt_ubase_t);
    }
    #endif

    uxReturn = (UBaseType_t)prvTaskCheckFreeStackSpace(pucEndOfStack);

    return uxReturn;
}


BaseType_t xTaskGetAffinity(TaskHandle_t xTask) {
    TCB_t *tcb;

    tcb = prvGetTCBFromHandle(xTask);

    return tcb->pxThread->oncpu;
}
