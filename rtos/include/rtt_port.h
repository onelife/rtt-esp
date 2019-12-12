
#include "rtthread.h"

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
    enum ipc_type typ;
    unsigned int maxCnt;
    union ipc_union mem;
};

typedef struct ipc_wrap generic_ipc_t;
typedef struct ipc_wrap * generic_handle_t;

typedef generic_handle_t SemaphoreHandle_t;
typedef generic_ipc_t StaticSemaphore_t;
typedef generic_handle_t QueueHandle_t;

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
    TickType_t ticks);
BaseType_t wrap_mq_send_in_isr(QueueHandle_t handle, const void * const item,
    BaseType_t * const pxHigherPriorityTaskWoken);
BaseType_t wrap_mq_recv(QueueHandle_t handle, void * const buffer,
    TickType_t ticks);
BaseType_t wrap_mq_recv_in_isr(QueueHandle_t handle, void * const buffer,
    BaseType_t * const pxHigherPriorityTaskWoken);
BaseType_t wrap_mq1_replace_in_isr(QueueHandle_t handle,
    const void * const item, BaseType_t * const pxHigherPriorityTaskWoken);
void wrap_mq_delete(QueueHandle_t handle);

BaseType_t xQueueAddToSet( QueueSetMemberHandle_t xQueueOrSemaphore, QueueSetHandle_t xQueueSet );
BaseType_t xQueueRemoveFromSet( QueueSetMemberHandle_t xQueueOrSemaphore, QueueSetHandle_t xQueueSet );

#define TO_STRING(a)                    # a
#define CONCATE(a, b)                   TO_STRING(a ## b)
#define APPEND_COUNTER(name)            CONCATE(name, __COUNTER__)

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

#define vSemaphoreDelete(handle)        wrap_ipc_delete((handle))

#define xSemaphoreTakeFromISR(handle, pxHigherPriorityTaskWoken) \
    wrap_ipc_take_in_isr((handle), (pxHigherPriorityTaskWoken))

#define xSemaphoreGiveFromISR(handle, pxHigherPriorityTaskWoken) \
    wrap_ipc_give_in_isr((handle), (pxHigherPriorityTaskWoken))

#define xSemaphoreGetMutexHolder(handle) \
    wrap_ipc_get_owner((handle))

#define xQueueCreate(uxQueueLength, uxItemSize) \
    wrap_mq_create(APPEND_COUNTER(mqD_),  (uxQueueLength), (uxItemSize))

#define xQueueReset(handle)             wrap_mq_reset((handle))

#define xQueueSend(handle, item, ticks) wrap_mq_send((handle), (item), (ticks))

#define xQueueReceive(handle, buffer, ticks) \
    wrap_mq_recv((handle), (buffer), (ticks))

#define xQueueSendFromISR(handle, item, pxHigherPriorityTaskWoken ) \
    wrap_mq_send_in_isr((handle), (item), (pxHigherPriorityTaskWoken))

#define xQueueReceiveFromISR(handle, buffer, pxHigherPriorityTaskWoken ) \
    wrap_mq_recv_in_isr((handle), (buffer), (pxHigherPriorityTaskWoken))

#define vQueueDelete(handle)            wrap_mq_delete((handle))

#define xQueueOverwriteFromISR(handle, item, pxHigherPriorityTaskWoken ) \
    wrap_mq1_replace_in_isr((handle), (item), (pxHigherPriorityTaskWoken))

#define xQueueIsQueueFullFromISR(handle) \
    wrap_mq_is_full(handle)
