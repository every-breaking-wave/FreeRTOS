/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Standard include. */
#include <stdio.h>


/* The ITM port is used to direct the printf() output to the serial window in  ITM->Instrument Trace Macrocell
the Keil simulator IDE. */
#define mainITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define mainITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define mainDEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define mainTRCENA          0x01000000

/*-----------------------------------------------------------*/


#define UART0_ADDRESS                         ( 0x40004000UL )
#define UART0_DATA                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 0UL ) ) ) )
#define UART0_STATE                           ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 4UL ) ) ) )
#define UART0_CTRL                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 8UL ) ) ) )
#define UART0_BAUDDIV                         ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 16UL ) ) ) )
#define TX_BUFFER_MASK                        ( 1UL )

/*


volatile unsigned long tvalue;

/*
 * Redirects the printf() output to the serial window in the Keil simulator
 * IDE.
 */
int fputc( int iChar, FILE *pxNotUsed ) 
{
	/* Just to avoid compiler warnings. */
	( void ) pxNotUsed;

	if( mainDEMCR & mainTRCENA ) 
	{
		while( mainITM_Port32( 0 ) == 0 );
		mainITM_Port8( 0 ) = iChar;
  	}

  	return( iChar );
}

/*
 * Printf() output is sent to the serial port.  Initialise the serial hardware.
 */
static void prvUARTInit( void );

// void vAssertCalled( const char * pcFile,
//                     unsigned long ulLine )
// {
//     volatile unsigned long ul = 0;

//     ( void ) pcFile;
//     ( void ) ulLine;

//     taskENTER_CRITICAL();
//     {
//         /* Set ul to a non-zero value using the debugger to step out of this
//          * function. */
//         while( ul == 0 )
//         {
//             portNOP();
//         }
//     }
//     taskEXIT_CRITICAL();
// }

/*-----------------------------------------------------------*/
unsigned long ulTaskNumber[ configEXPECTED_NO_RUNNING_TASKS ];

/*-----------------------------------------------------------*/
TaskHandle_t xT1;
TaskHandle_t xT2,xT3,xT4;

void vFullDemoTickHookFunction( void );
void vFullDemoIdleFunction( void );

static void T1( void *pvParameters );
static void T2( void *pvParameters );
static void T3( void *pvParameters );
static void T4( void *pvParameters );



const unsigned long dT1 = 100;
const unsigned long dT2 = 70;
const unsigned long dT3 = 50;
const unsigned long dT4 = 400;
int main(void)
{
	prvUARTInit();
	printf("Starting EDF Scheduler\n");
    xTaskCreate( T1, ( signed char * ) "T1", configMINIMAL_STACK_SIZE, (void *)&dT1, 1 , &xT1 );		
	  xTaskCreate( T2, ( signed char * ) "T2", configMINIMAL_STACK_SIZE, (void *)&dT2, 1 , &xT2 );
  	xTaskCreate( T3, ( signed char * ) "T3", configMINIMAL_STACK_SIZE, (void *)&dT3, 1 , &xT3 );
	  xTaskCreate( T4, ( signed char * ) "T4", configMINIMAL_STACK_SIZE, (void *)&dT4, 1 , &xT4 );
		/* Start the tasks running. */
	vTaskStartScheduler();
	
	for( ;; );
}

/*-----------------------------------------------------------*/

static void T1( void *pvParameters )
{
	unsigned int i = 0;
	while(1)
	{	
		//i = 0xFFFFFFFE + 0xA;
		//printf("%x\n", i);
	 printf("T1 Executing\n");
	 for(i = 0;i < 9000; i++);
	 vTaskDelay( 10 / portTICK_PERIOD_MS );	
  }
}


static void T2( void *pvParameters )
{
	unsigned long i = 0; 
	while(1)
	{
		printf("T2 executing\n");
		for(i = 0;i < 9000; i++);
		vTaskDelay( 20 / portTICK_PERIOD_MS);
	}
}

static void T3( void *pvParameters )
{
	int i = 0;
	while(1)
	{	
	 printf("T3 Executing\n");
	 for(i = 0;i < 9000; i++);
	 vTaskDelay( 30 / portTICK_PERIOD_MS );	
  }
}


static void T4( void *pvParameters )
{
	unsigned long i = 0; 
	while(1)
	{
		printf("T4 executing\n");
	  for(i = 0;i < 9000; i++);
		vTaskDelay( 40 / portTICK_PERIOD_MS);
	}
}



void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
     * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
     * function that will get called if a call to pvPortMalloc() fails.
     * pvPortMalloc() is called internally by the kernel whenever a task, queue,
     * timer or semaphore is created using the dynamic allocation (as opposed to
     * static allocation) option.  It is also called by various parts of the
     * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
     * size of the	heap available to pvPortMalloc() is defined by
     * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
     * API function can be used to query the size of free heap space that remains
     * (although it does not provide information on how the remaining heap might be
     * fragmented).  See http://www.freertos.org/a00111.html for more
     * information. */
    printf( "\r\n\r\nMalloc failed\r\n" );
    portDISABLE_INTERRUPTS();

    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
     * task.  It is essential that code added to this hook function never attempts
     * to block in any way (for example, call xQueueReceive() with a block time
     * specified, or call vTaskDelay()).  If application tasks make use of the
     * vTaskDelete() API function to delete themselves then it is also important
     * that vApplicationIdleHook() is permitted to return to its calling function,
     * because it is the responsibility of the idle task to clean up memory
     * allocated by the kernel to any task that has since deleted itself. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask,
                                    char * pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
     * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     * function is called if a stack overflow is detected. */
    printf( "\r\n\r\nStack overflow in %s\r\n", pcTaskName );
    portDISABLE_INTERRUPTS();

    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    /* This function will be called by each tick interrupt if
    * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    * added here, but the tick hook is called from an interrupt context, so
    * code must not attempt to block, and only the interrupt safe FreeRTOS API
    * functions can be used (those that end in FromISR()). */

    #if ( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY != 1 )
    {
        extern void vFullDemoTickHookFunction( void );

        vFullDemoTickHookFunction();
    }
    #endif /* mainCREATE_SIMPLE_BLINKY_DEMO_ONLY */
}
/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
    /* This function will be called once only, when the daemon task starts to
     * execute (sometimes called the timer task).  This is useful if the
     * application includes initialisation code that would benefit from executing
     * after the scheduler has been started. */
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char * pcFileName,
                    uint32_t ulLine )
{
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Called if an assertion passed to configASSERT() fails.  See
     * http://www.freertos.org/a00110.html#configASSERT for more information. */

    printf( "ASSERT! Line %d, file %s\r\n", ( int ) ulLine, pcFileName );

    taskENTER_CRITICAL();
    {
        /* You can step out of this function to debug the assertion by using
         * the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
         * value. */
        while( ulSetToNonZeroInDebuggerToContinue == 0 )
        {
            __asm volatile ( "NOP" );
            __asm volatile ( "NOP" );
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/

static void prvUARTInit( void )
{
    UART0_BAUDDIV = 16;
    UART0_CTRL = 1;
}
// /*-----------------------------------------------------------*/

int __write( int iFile,
             char * pcString,
             int iStringLength )
{
    int iNextChar;

    /* Avoid compiler warnings about unused parameters. */
    ( void ) iFile;

    /* Output the formatted string to the UART. */
    for( iNextChar = 0; iNextChar < iStringLength; iNextChar++ )
    {
        while( ( UART0_STATE & TX_BUFFER_MASK ) != 0 )
        {
        }

        UART0_DATA = *pcString;
        pcString++;
    }

    return iStringLength;
}
/*-----------------------------------------------------------*/

void * malloc( size_t size )
{
    ( void ) size;

    /* This project uses heap_4 so doesn't set up a heap for use by the C
     * library - but something is calling the C library malloc().  See
     * https://freertos.org/a00111.html for more information. */
    printf( "\r\n\r\nUnexpected call to malloc() - should be usine pvPortMalloc()\r\n" );
    portDISABLE_INTERRUPTS();

    for( ; ; )
    {
    }
}
