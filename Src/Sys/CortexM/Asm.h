#ifndef ASM_H
#define ASM_H

#define _ASM_VOLATILE	__asm volatile
#define _ASM			__asm

#define ASM_NOP()					_ASM_VOLATILE ("NOP")
#define ASM_BREAK()					_ASM_VOLATILE ("BKPT #255")
#define ASM_WAIT_FOR_INT()			_ASM_VOLATILE ("WFI")
#define ASM_GET_STACK_PTR(PtrVar)	_ASM_VOLATILE ("MOV.W %0, SP;" : "=r" (PtrVar))
#define ASM_GET_PC(PCVar)			_ASM_VOLATILE ("MOV.W %0, PC;" : "=r" (PCVar))
#define ASM_GET_IPSR(Var)			_ASM_VOLATILE ("MRS %0, IPSR" : "=r" (Var))
#define ASM_GET_CONTROL(Var)		_ASM_VOLATILE ("MRS %0, CONTROL" : "=r" (Var))
#define ASM_SET_CONTROL(Var)		_ASM_VOLATILE ("MSR CONTROL, %0" : : "r" (Var))
#define ASM_GET_PSP(Var)			_ASM_VOLATILE ("MRS %0, PSP" : "=r" (Var))
#define ASM_SET_PSP(Var)			_ASM_VOLATILE ("MSR PSP, %0" : : "r" (Var))
#define ASM_GET_MSP(Var)			_ASM_VOLATILE ("MRS %0, MSP" : "=r" (Var))
#define ASM_SET_MSP(Var)			_ASM_VOLATILE ("MSR MSP, %0" : : "r" (Var))
#define ASM_CLEAR_EXCLUSIVE()		_ASM_VOLATILE ("CLREX")
#define ASM_PUSH_VALUE(Var)			_ASM_VOLATILE ("MOV.W R4, %0; PUSH {R4};" : : "r" (Var))
#define ASM_POP_VALUE(Var)			_ASM_VOLATILE ("POP {R4}; MOV.W R4, %0;" : "=r" (Var))

#endif  // End of ASM_H
