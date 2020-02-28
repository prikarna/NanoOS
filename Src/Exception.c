/*
 * File    : Exception.c
 * Remark  : Contains exception and interrupt table plus some exceptions 
 *           and interrupt handler.
 *
 */

#include "Sys\CortexM\Scb.h"
#include "Sys\CortexM\Nvic.h"

#include "Exception.h"
#include "NanoOS.h"
#include "Address.h"
#include "Thread.h"
#include "Interrupt.h"

extern UINT32_PTR_T				gpLastDbgStack;
extern UINT32_PTR_T				gpLastIntStack;

static INTERRUPT_HANDLER_TYPE	spGlobalIntHandler = 0;
static UINT32_T					suIntType = 0;
static UINT32_T					suFault;
static UINT32_T					suISR;

__attribute__ ((section(".nanoos_isr")))
void (* const gHandlers[])(void) =
{
	/* 0. Stack Address		*/	(void *) (SYSTEM_STACK_ADDRESS),
	/* 1. ISR 1, Prio -3	*/	ExcReset,
	/* 2. ISR 2, Prio -2	*/	ExcNonMaskableInterrupt,
	/* 3. ISR 3, Prio -1	*/	ExcHardFault,
	/* 4. ISR 4				*/	ExcGeneralFault,
	/* 5. ISR 5				*/	ExcGeneralFault,
	/* 6. ISR 6				*/	ExcGeneralFault,
	/* 7. ISR 7				*/	ExcReserved,
	/* 8. ISR 8				*/	ExcReserved,
	/* 9. ISR 9				*/	ExcReserved,
	/* 10. ISR 10			*/	ExcReserved,
	/* 11. ISR 11			*/	ExcSupervisorCall,
	/* 12. ISR 12			*/	ExcDebugMonitor,
	/* 13. ISR 13			*/	ExcReserved,
	/* 14. ISR 14			*/	ExcReserved,		// Pend SV Call
	/* 15. ISR 15			*/	ExcSystemTick,
	/* 16. ISR 16, Irq 0	*/	IntDefault,
	/* 17. ISR 17, Irq 1	*/	IntDefault,
	/* 18. ISR 18, Irq 2	*/	IntDefault,
	/* 19. ISR 19, Irq 3	*/	IntDefault,
	/* 20. ISR 20, Irq 4	*/	IntDefault,
	/* 21. ISR 21, Irq 5	*/	IntDefault,
	/* 22. ISR 22, Irq 6	*/	IntDefault,
	/* 23. ISR 23, Irq 7	*/	IntDefault,
	/* 24. ISR 24, Irq 8	*/	IntDefault,
	/* 25. ISR 25, Irq 9	*/	IntDefault,
	/* 26. ISR 26, Irq 10	*/	IntDefault,
	/* 27. ISR 27, Irq 11	*/	IntDefault,
	/* 28. ISR 28, Irq 12	*/	IntDefault,
	/* 29. ISR 29, Irq 13	*/	IntDefault,
	/* 30. ISR 30, Irq 14	*/	IntDefault,
	/* 31. ISR 31, Irq 15	*/	IntDefault,
	/* 32. ISR 32, Irq 16	*/	IntDefault,
	/* 33. ISR 33, Irq 17	*/	IntDefault,
	/* 34. ISR 34, Irq 18	*/	IntDefault,
	/* 35. ISR 35, Irq 19	*/	IntDefault,
	/* 36. ISR 36, Irq 20	*/	IntUSBLowPriorityOrCAN1_RX0,		// USB low prio. int.
	/* 37. ISR 37, Irq 21	*/	IntDefault,
	/* 38. ISR 38, Irq 22	*/	IntDefault,
	/* 39. ISR 39, Irq 23	*/	IntDefault,
	/* 40. ISR 40, Irq 24	*/	IntDefault,
	/* 41. ISR 41, Irq 25	*/	IntDefault,
	/* 42. ISR 42, Irq 26	*/	IntDefault,
	/* 43. ISR 43, Irq 27	*/	IntDefault,
	/* 44. ISR 44, Irq 28	*/	IntTimer2,
	/* 45. ISR 45, Irq 29	*/	IntDefault,
	/* 46. ISR 46, Irq 30	*/	IntDefault,
	/* 47. ISR 47, Irq 31	*/	IntDefault,
	/* 48. ISR 48, Irq 32	*/	IntDefault,
	/* 49. ISR 49, Irq 33	*/	IntDefault,
	/* 50. ISR 50, Irq 34	*/	IntDefault,
	/* 51. ISR 51, Irq 35	*/	IntDefault,
	/* 52. ISR 52, Irq 36	*/	IntDefault,
	/* 53. ISR 53, Irq 37	*/	IntDefault,
	/* 54. ISR 54, Irq 38	*/	IntDefault,
	/* 55. ISR 55, Irq 39	*/	IntUSART3,
	/* 56. ISR 56, Irq 40	*/	IntDefault,
	/* 57. ISR 57, Irq 41	*/	IntDefault,
	/* 58. ISR 58, Irq 42	*/	IntDefault,
	/* 59. ISR 59, Irq 43	*/	IntDefault,
	/* 60. ISR 60, Irq 44	*/	IntDefault,
	/* 61. ISR 61, Irq 45	*/	IntDefault,
	/* 62. ISR 62, Irq 46	*/	IntDefault,
	/* 63. ISR 63, Irq 47	*/	IntDefault,
	/* 63. ISR 63, Irq 48	*/	IntDefault,
	/* 64. ISR 64, Irq 49	*/	IntDefault,
	/* 65. ISR 65, Irq 50	*/	IntDefault,
	/* 66. ISR 66, Irq 51	*/	IntDefault,
	/* 67. ISR 67, Irq 52	*/	IntDefault,
	/* 68. ISR 68, Irq 53	*/	IntDefault,
	/* 69. ISR 69, Irq 54	*/	IntDefault,
	/* 70. ISR 70, Irq 55	*/	IntDefault,
	/* 71. ISR 71, Irq 56	*/	IntDefault,
	/* 72. ISR 72, Irq 57	*/	IntDefault,
	/* 73. ISR 73, Irq 58	*/	IntDefault,
	/* 74. ISR 74, Irq 59	*/	IntDefault,
	/* 75. ISR 75			*/	ExcReserved,
	/* 76. ISR 76			*/	ExcReserved,
	/* 77. ISR 77			*/	ExcReserved,
	/* 78. ISR 78			*/	ExcReserved,
	/* 79. ISR 79			*/	ExcReserved,
	/* 80. ISR 80			*/	ExcReserved,
	/* 81. ISR 81			*/	ExcReserved,
	/* 82. ISR 82			*/	ExcReserved,
	/* 83. ISR 83			*/	ExcReserved
};

__attribute__((naked))
void ExcReserved()
{
#ifdef _DEBUG
	GET_STACK_POINTER(gpLastDbgStack);
#endif

	__asm volatile 
		(
		"MOV.W SP, %0;"
		"SUB SP, #16;"
		"MOV.W R7, SP;"
		: : "r" (SYSTEM_STACK_ADDRESS)
		);

	GET_IPSR(suISR);
	TRAP_EXEC((1), TRAP__RESERVED_EXCEPTION, suISR, 0);
}

__attribute__((weak)) 
void ExcDebugMonitor()
{
	TRAP_EXEC(1, TRAP__DEBUG_EXCEPTION, 0, 0);
}


BOOL ExcSetInterrupt(INTERRUPT_HANDLER_TYPE IntHandler)
{
	spGlobalIntHandler = IntHandler;
	return TRUE;
}

__attribute__((naked))
void IntDefault()
{
	BEGIN_EXCEPTION();

	GET_STACK_POINTER(gpLastIntStack);

#ifdef _DEBUG
	gpLastDbgStack = gpLastIntStack;
#endif

	SCB_CLR_GENERAL_FAULT_STATUS();

	GET_IPSR(suIntType);
	suIntType -= INTERRUPT_OFFSET;

	if (spGlobalIntHandler) {
		(* spGlobalIntHandler)(suIntType);
	} else {
		TRAP_EXEC(1, TRAP__UNHANDLED_INTERRUPT, suIntType, 0);
	}

	suFault = SCB_GET_GENERAL_FAULT_STATUS();
	if (suFault == 0) {
		gpLastIntStack = 0;
	}

	END_EXCEPTION();
}