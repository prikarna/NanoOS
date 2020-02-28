# NanoOS

Tiny OS for Cortex-M based chipset with multithread and fault handling features. 
For now only support for STM32F103XXX chipset only, STM32F103C8 chipset for specific.

-----------------

Brief :

	Memory :
	- NanoOS devide flash and sram memory into two section each, one for the NanoOS itself 
	  and other for the application that run on the top of it. From 64K bytes total flash, 
	  32K bytes used by NanoOS, 31K reserved for the application and 1K reserved for 
	  configuration. From 20K bytes total sram, 10K bytes used by NanoOS and another 10K 
	  bytes reserved for application. So application can have 31K bytes flash and 10K bytes 
	  sram. Note that this may change in the future.
	  
	Thread :
	- For now only support for 4 active threads that can run concurrently and this is a 
	  'stack based' thread (a classic method for multithread mechanism). Each stack has 2K 
	  bytes size but not all of it can be used effectively by application because NanoOS 
	  need to compensate small amount of stack to do some stack protection. Note that this 
	  kind of protection may not suitable for all situations. This stack is a just memory 
	  (sram) and it is a fixed allocation, application can't change it.
	- Use system tick exception, which is available on ARM Cortex-M processor, with specific 
	  priority to drive thread operation.
	  
	Interfaces :
	- NanoOS has two interfaces to communicate to the outside world. One use USART3 (which 
	  available in STM32F103C8 chipset). This interface is used by NanoOS to interract with
	  user via built-in console. This console is simple console to do some administation tasks 
	  available on NanoOS. Note that this is simple console and don't provide a security. USART3
	  also can be used by application to output a debug message while developing an application.
	  The secound interfaec is USB. This interface can be used by application to communicate
	  with the outside world like a PC (Personal Computer). This interface is a simple serial
	  (COM) port to a PC point of view.
	- These two interfaces can be accessed via a terminal program, e.g. Hyperterminal, Putty and
	  NanoOS terminal.
	  
	NanoOS Terminal :
	- A PC program running on Windows, (for now only tested on Windows XP SP 2), specifically for 
	  NanoOS to control some aspect of NanoOS via built-in console. This program can be used to 
	  install a NanoOS application too.
	- This program has built-in virtual display to simulate a LCD touch screen. Can be usefull to
	  build a user interface in NanoOS application. Note that this is just simulation and use USB
	  interface to communicate with.
	  
	Nano OS API :
	- A simple application programming interface or API. For now this only tested on C language.
	  The main purpose of this API is actually not to 'serve' an application, but it rather for
	  synchronization so that the application can access the NanoOS resource in multhreaded 
	  environment safer. Note that an application can access NanoOS resource or function directly
	  without this API if it wanted to.
	  