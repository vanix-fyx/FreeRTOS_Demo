#include "freertos_hooks.h"
#include "uart.h"



void vApplicationMallocFailedHook(void)
{
     printf("内存申请失败!剩余:%d\r\n", xPortGetFreeHeapSize());
}
