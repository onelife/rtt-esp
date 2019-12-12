/***************************************************************************//**
 * @file    rtconfig.h
 * @brief   Arduino RT-Thread library config
 * @author  onelife <onelife.real[at]gmail.com>
 ******************************************************************************/
#ifndef __RTCONFIG_H__
#define __RTCONFIG_H__

#include "FreeRTOS.h"

/* User Config */


/* ESP32 Config */

#define ESP32

#define RT_USING_SMP
#define RT_CPUS_NR                      (2)
#define RT_USING_NEWLIB

#define rt_free                         free
// #ifndef CONFIG_PRIORITY_MAX
// # define CONFIG_PRIORITY_MAX            (3)     /* NVIC */
// #endif

// #ifndef CONFIG_KERNEL_PRIORITY
// # define CONFIG_KERNEL_PRIORITY         (2)     /* Platform */
// #endif

#ifndef CONFIG_HEAP_SIZE
# ifdef ARDUINO_ARCH_SAM
#  define CONFIG_HEAP_SIZE              (40 * 1024)
# else
#  define CONFIG_HEAP_SIZE              (20 * 1024)
# endif
#endif


/* Default User Config */

#ifndef CONFIG_USING_RTC
# define CONFIG_USING_RTC               (0)
#endif

#ifndef CONFIG_USART_SPEED
# define CONFIG_USART_SPEED             (115200)
#endif

#ifndef CONFIG_USING_CONSOLE
# define CONFIG_USING_CONSOLE           (1)
#endif

// #ifndef CONFIG_USING_FINSH
// # define CONFIG_USING_FINSH             (CONFIG_USING_CONSOLE)
// #endif

#ifndef CONFIG_USING_MSH
# define CONFIG_USING_MSH               (1)
#endif

#ifndef CONFIG_USING_LOG
# define CONFIG_USING_LOG               (1)
#endif

#ifndef CONFIG_USING_MODULE
# define CONFIG_USING_MODULE            (0)
#endif

#ifndef CONFIG_USING_GUI
# define CONFIG_USING_GUI               (0)
#endif

#ifndef CONFIG_USING_BUTTON
# define CONFIG_USING_BUTTON            (0)
#endif

#ifndef CONFIG_USING_SPISD
# define CONFIG_USING_SPISD             (0)
#endif

#ifndef CONFIG_USING_EXFAT
# define CONFIG_USING_EXFAT             (0)
#endif

#ifndef CONFIG_USING_ILI
# define CONFIG_USING_ILI               (0)
#endif

#ifndef CONFIG_USING_SSD1331
# define CONFIG_USING_SSD1331           (0)
#endif

#ifndef CONFIG_USING_FT6206
# define CONFIG_USING_FT6206            (0)
#endif

#ifndef CONFIG_USING_SPI0
# define CONFIG_USING_SPI0              (0)
#endif

#ifndef CONFIG_USING_SPI1
# define CONFIG_USING_SPI1              (0)
#endif

#ifndef CONFIG_USING_IIC0
# define CONFIG_USING_IIC0              (0)
#endif

#ifndef CONFIG_USING_IIC1
# define CONFIG_USING_IIC1              (0)
#endif


/* Config Check */

#if (CONFIG_USING_CONSOLE)
# ifndef CONFIG_SERIAL_DEVICE
#  define CONFIG_SERIAL_DEVICE          (Serial)
# endif
#endif /* CONFIG_USING_CONSOLE */

#if (CONFIG_USING_LOG)
# define RT_USING_ULOG
# define ULOG_OUTPUT_LVL                (LOG_LVL_DBG) // (LOG_LVL_INFO)
# define ULOG_ASSERT_ENABLE
# define ULOG_USING_COLOR
// # define ULOG_USING_ISR_LOG
// # define ULOG_USING_SYSLOG
// # define ULOG_USING_FILTER
# define ULOG_OUTPUT_TIME
// # define ULOG_TIME_USING_TIMESTAMP
# define ULOG_OUTPUT_LEVEL
# define ULOG_OUTPUT_TAG
# define ULOG_OUTPUT_THREAD_NAME
# if (CONFIG_USING_CONSOLE)
#  define ULOG_BACKEND_USING_CONSOLE
# endif 
#endif /* CONFIG_USING_LOG */

#if (CONFIG_USING_MODULE)
# define RT_USING_MODULE
# define MODULE_THREAD_PRIORITY         (RT_THREAD_PRIORITY_MAX - 1)
# define MODULE_THREAD_STACK_SIZE       (4 * 512)
# define IDLE_THREAD_STACK_SIZE         (1 * 512)
#endif /* CONFIG_USING_MODULE */

#if (CONFIG_USING_SPISD)
# ifndef CONFIG_SD_CS_PIN
#  error "Please define CONFIG_SD_CS_PIN!"
# endif
# if (!defined(CONFIG_USING_SPI0) && !defined(CONFIG_USING_SPI1))
#  error "Please define CONFIG_USING_SPIx!"
# endif
# ifndef CONFIG_SD_SPI_CHANNEL
#  error "Please define CONFIG_SD_SPI_CHANNEL!"
# endif
#endif /* CONFIG_USING_SPISD */

#if (CONFIG_USING_ILI)
# ifndef CONFIG_ILI_CS_PIN
#  error "Please define CONFIG_ILI_CS_PIN"
# endif
# ifndef CONFIG_ILI_DC_PIN
#  error "Please define CONFIG_ILI_DC_PIN"
# endif
# ifndef CONFIG_ILI_SPI_CHANNEL
#  error "Please define CONFIG_ILI_SPI_CHANNEL"
# endif
#endif /* CONFIG_USING_ILI */

#if (CONFIG_USING_FT6206)
# ifndef CONFIG_FT6206_IIC_CHANNEL
#  error "Please define CONFIG_FT6206_IIC_CHANNEL"
# endif
#endif /* CONFIG_USING_FT6206 */

#if (CONFIG_USING_GUI)
# ifndef CONFIG_GUI_WIDTH
#  error "Please define CONFIG_GUI_WIDTH"
# endif
# ifndef CONFIG_GUI_HIGH
#  error "Please define CONFIG_GUI_HIGH"
# endif
#endif /* CONFIG_USING_GUI */

#if CONFIG_USING_RTC
# define RT_USING_RTC
# define RT_USING_ALARM
#endif


/* Debug Config */

#define RT_DEBUG
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_MEMTRACE
// #define RT_DEBUG_INIT                   (1)
// #define RT_DEBUG_MEM                    (1)
// #define RT_DEBUG_SCHEDULER              (1)
// #define RT_DEBUG_IPC                    (1)
// #define RT_DEBUG_TIMER                  (1)
// #define RT_DEBUG_THREAD                 (1)


/* System Config */

#define RT_NAME_MAX                     (16)
#define RT_ALIGN_SIZE                   (4)
#define RT_THREAD_PRIORITY_MAX          (32)
#define RT_TICK_PER_SECOND              (100)
// #define RT_TICK_PER_SECOND              (XT_TICK_PER_SEC)


/* Arduino Thread Config */

#ifndef CONFIG_ARDUINO_STACK_SIZE
# define CONFIG_ARDUINO_STACK_SIZE      (3 * 512)
#endif
#ifndef CONFIG_ARDUINO_PRIORITY
# define CONFIG_ARDUINO_PRIORITY        (RT_THREAD_PRIORITY_MAX >> 1)
#endif
#ifndef CONFIG_ARDUINO_TICK
# define CONFIG_ARDUINO_TICK            (15)
#endif


/* Timer Config */

// #define RT_USING_TIMER_SOFT
// #define RT_TIMER_THREAD_PRIO            (4)
// #define RT_TIMER_THREAD_STACK_SIZE      (512)


/* Utility Config */

#define RT_USING_DEVICE                 /* Required by IPC, DRV */
#define RT_USING_MUTEX                  /* Required by DFS, DRV */
#define RT_USING_SEMAPHORE              /* Required by FINSH */
#define RT_USING_MAILBOX                /* Required by GUI */
#define RT_USING_MESSAGEQUEUE           /* Required by ? */
// #define RT_USING_EVENT                  /* Required by ? */
// #define RT_USING_SIGNALS                /* Required by ? */
// #define RT_USING_HOOK
// #define RT_USING_IDLE_HOOK


/* Memory Management Config */

#define RT_USING_MEMPOOL                /* Required by SIG, GUI */
// #define RT_USING_MEMHEAP
#define RT_USING_HEAP
#define RT_USING_SMALL_MEM


/* Console Config */

#if (CONFIG_USING_CONSOLE)
# define RT_USING_CONSOLE
# define RT_CONSOLEBUF_SIZE             (128)
#endif /* CONFIG_USING_CONSOLE */


/* FinSH Config */

#if (CONFIG_USING_FINSH)
#define RT_USING_FINSH
# if (CONFIG_USING_MSH)
#  define FINSH_USING_MSH
#  define FINSH_USING_MSH_ONLY
# endif
#define FINSH_USING_DESCRIPTION
#define FINSH_USING_HISTORY
#define FINSH_THREAD_PRIORITY           ((RT_THREAD_PRIORITY_MAX >> 1) + (RT_THREAD_PRIORITY_MAX >> 3))
#define FINSH_THREAD_STACK_SIZE         (2 * 512)
#endif /* CONFIG_USING_FINSH */


/* File System Config */

#if (CONFIG_USING_SPISD)
# define RT_USING_DFS
// # define RT_USING_DFS_MNTTABLE          /* Mount table */
# define RT_USING_DFS_ELMFAT
# define DFS_USING_WORKDIR
# define DFS_FILESYSTEMS_MAX            (1)     /* Max number of fs */
# define DFS_FD_MAX                     (4)     /* Max number of open file */
# define RT_DFS_ELM_CODE_PAGE           437     /* (xxx) is wrong -_-! */
// # define RT_DFS_ELM_CODE_PAGE           936     /* (936) is wrong -_-! */
# if (CONFIG_USING_EXFAT)
#  define RT_DFS_ELM_USE_EXFAT
# endif
# if defined(RT_DFS_ELM_USE_EXFAT) || (RT_DFS_ELM_CODE_PAGE >= 900)
#  define RT_DFS_ELM_USE_LFN            (2)
#  define RT_DFS_ELM_MAX_LFN            (255)
// #  define RT_DFS_ELM_LFN_UNICODE
# endif /* defined(RT_DFS_ELM_USE_EXFAT) || (RT_DFS_ELM_CODE_PAGE >= 900) */
#endif /* CONFIG_USING_SPISD */


/* Unsupported Config */

#ifdef FINSH_USING_SYMTAB
# undef FINSH_USING_SYMTAB              /* Reason: no access to linker script */
#endif
#ifdef RT_USING_COMPONENTS_INIT
# undef RT_USING_COMPONENTS_INIT        /* Reason: no access to linker script */
#endif

#endif /* __RTCONFIG_H__ */
