
#include "rtthread.h"

// TODO:
#define vQueueWaitForMessageRestricted(a, b)


enum ipc_type {
    RTT_MUTEX,
    RTT_SEMAPHORE,
    RTT_QUEUE,
};

union ipc_union {
    struct rt_mutex mux;
    struct rt_semaphore sem;
    struct rt_messagequeue mq;
};

struct ipc_wrap {
    enum ipc_type type;
    unsigned int maxCnt;
    struct ipc_qset *cntr;
    union ipc_union member;
};

struct ipc_qset {
    unsigned int length;
    rt_slist_t member;
};

// struct ipc_qset_item {
//     struct ipc_wrap item;
// };


typedef struct ipc_wrap generic_ipc_t;
typedef struct ipc_wrap * generic_handle_t;

typedef struct ipc_wrap StaticSemaphore_t;
typedef struct ipc_wrap * SemaphoreHandle_t;
typedef struct ipc_wrap * QueueHandle_t;

typedef struct ipc_qset QueueSet_t;
typedef struct ipc_qset * QueueSetHandle_t;
typedef struct ipc_wrap QueueSetMember_t;
typedef struct ipc_wrap * QueueSetMemberHandle_t;


SemaphoreHandle_t wrap_mutex_create(const char *name);
SemaphoreHandle_t wrap_bin_sem_create(const char *name,
    const UBaseType_t uxInitialCount);
SemaphoreHandle_t wrap_cnt_sem_create(const char *name,
    const UBaseType_t uxMaxCount, const UBaseType_t uxInitialCount);
SemaphoreHandle_t wrap_cnt_sem_init(const char *name,
    const UBaseType_t uxMaxCount, const UBaseType_t uxInitialCount,
    StaticSemaphore_t *pxStaticQueue);
BaseType_t wrap_ipc_take(SemaphoreHandle_t handle, TickType_t ticks);
BaseType_t wrap_ipc_take_in_isr(SemaphoreHandle_t handle,
    BaseType_t * const pxHigherPriorityTaskWoken);
BaseType_t wrap_ipc_give(SemaphoreHandle_t handle);
BaseType_t wrap_ipc_give_in_isr(SemaphoreHandle_t handle,
    BaseType_t * const pxHigherPriorityTaskWoken);
void wrap_ipc_delete(SemaphoreHandle_t handle);
void *wrap_ipc_get_owner(SemaphoreHandle_t handle);

QueueHandle_t wrap_mq_create(const char *name, const UBaseType_t uxQueueLength,
    const UBaseType_t uxItemSize);
BaseType_t wrap_mq_reset(QueueHandle_t handle);
BaseType_t wrap_mq_send(QueueHandle_t handle, const void * const item,
    TickType_t ticks, uint8_t is_urgent);
BaseType_t wrap_mq_send_in_isr(QueueHandle_t handle, const void * const item,
    BaseType_t * const pxHigherPriorityTaskWoken);
BaseType_t wrap_mq_recv(QueueHandle_t handle, void * const buffer,
    TickType_t ticks);
BaseType_t wrap_mq_recv_in_isr(QueueHandle_t handle, void * const buffer,
    BaseType_t * const pxHigherPriorityTaskWoken);
BaseType_t wrap_mq1_replace_in_isr(QueueHandle_t handle,
    const void * const item, BaseType_t * const pxHigherPriorityTaskWoken);
void wrap_mq_delete(QueueHandle_t handle);
UBaseType_t wrap_mq_entries(const QueueHandle_t handle);
BaseType_t wrap_mq_is_full(const QueueHandle_t handle);

QueueSetHandle_t wrap_qset_create(const UBaseType_t length);
BaseType_t wrap_qset_add(QueueSetMemberHandle_t qOrSem_handle,
    QueueSetHandle_t handle);
BaseType_t wrap_qset_remove(QueueSetMemberHandle_t qOrSem_handle,
    QueueSetHandle_t handle);
// QueueSetHandle_t xQueueCreateSet( const UBaseType_t uxEventQueueLength ) 


#define TO_STRING(a)                    # a
#define CONCATE(a, b)                   TO_STRING(a ## b)
#define APPEND_COUNTER(name)            CONCATE(name, __COUNTER__)


#define xQueueSendToBack( xQueue, pvItemToQueue, xTicksToWait ) (0)
#define xQueueSendToBackFromISR( xQueue, pvItemToQueue, pxHigherPriorityTaskWoken ) (0)
#define uxSemaphoreGetCount( xSemaphore ) (0)
#define uxQueueSpacesAvailable( xQueue ) (4)

#define xQueueGenericSend(handle, item, ticks, pos) \
    ((queueSEND_TO_BACK == pos) ? wrap_mq_send((handle), (item), (ticks), 0) : \
    ((queueSEND_TO_FRONT == pos) ? wrap_mq_send((handle), (item), (ticks), 1) : pdFAIL))

#define xSemaphoreCreateRecursiveMutex() \
    wrap_mutex_create(APPEND_COUNTER(rMutD_))

#define xSemaphoreTakeRecursive(handle, ticks) \
    wrap_ipc_take((handle), (ticks))

#define xSemaphoreGiveRecursive(handle) \
    wrap_ipc_give((handle))

#define xSemaphoreCreateMutex()         \
    wrap_mutex_create(APPEND_COUNTER(mutD_))

#define xSemaphoreCreateBinary()        \
    wrap_bin_sem_create(APPEND_COUNTER(bSemD_), 0)

#define xSemaphoreCreateCounting(uxMaxCount, uxInitialCount) \
    wrap_cnt_sem_create(APPEND_COUNTER(cSemD_), (uxMaxCount), (uxInitialCount))

#define xSemaphoreCreateCountingStatic(uxMaxCount, uxInitialCount, pxSemaphoreBuffer) \
    wrap_cnt_sem_init(APPEND_COUNTER(cSemS_), (uxMaxCount), (uxInitialCount), (pxSemaphoreBuffer))

#define xQueueCreateMutex(type)         \
    ((queueQUEUE_TYPE_RECURSIVE_MUTEX == type) ? wrap_mutex_create(APPEND_COUNTER(rMutD_)) : \
    ((queueQUEUE_TYPE_MUTEX == type) ? wrap_bin_sem_create(APPEND_COUNTER(mSemD_), 1) : NULL))

#define xSemaphoreTake(handle, ticks)   wrap_ipc_take((handle), (ticks))

#define xSemaphoreGive(handle)          wrap_ipc_give((handle))

#define vSemaphoreDelete(handle)        wrap_ipc_delete((SemaphoreHandle_t)(handle))

#define xSemaphoreTakeFromISR(handle, pxHigherPriorityTaskWoken) \
    wrap_ipc_take_in_isr((handle), (pxHigherPriorityTaskWoken))

#define xSemaphoreGiveFromISR(handle, pxHigherPriorityTaskWoken) \
    wrap_ipc_give_in_isr((handle), (pxHigherPriorityTaskWoken))

#define xSemaphoreGetMutexHolder(handle) \
    wrap_ipc_get_owner((handle))

#define xQueueCreate(uxQueueLength, uxItemSize) \
    wrap_mq_create(APPEND_COUNTER(mqD_),  (uxQueueLength), (uxItemSize))

#define xQueueReset(handle)             wrap_mq_reset((handle))

#define xQueueSend(handle, item, ticks) wrap_mq_send((handle), (item), (ticks), 0)

#define xQueueReceive(handle, buffer, ticks) \
    wrap_mq_recv((handle), (buffer), (ticks))

#define xQueueSendFromISR(handle, item, pxHigherPriorityTaskWoken ) \
    wrap_mq_send_in_isr((handle), (item), (pxHigherPriorityTaskWoken))

#define xQueueReceiveFromISR(handle, buffer, pxHigherPriorityTaskWoken ) \
    wrap_mq_recv_in_isr((handle), (buffer), (pxHigherPriorityTaskWoken))

// #define vQueueDelete(handle)            wrap_mq_delete((handle))
#define vQueueDelete                    wrap_mq_delete

#define xQueueOverwriteFromISR(handle, item, pxHigherPriorityTaskWoken ) \
    wrap_mq1_replace_in_isr((handle), (item), (pxHigherPriorityTaskWoken))

#define uxQueueMessagesWaiting          wrap_mq_entries

#define xQueueIsQueueFullFromISR(handle) \
    wrap_mq_is_full(handle)

#define xQueueAddToSet(qOrSem_handle, handle) \
    wrap_qset_add((qOrSem_handle), (handle))

#define xQueueRemoveFromSet(qOrSem_handle, handle) \
    wrap_qset_remove((qOrSem_handle), (handle))
