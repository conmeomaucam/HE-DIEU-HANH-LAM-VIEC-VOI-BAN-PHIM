set architecture armv7-m
set pagination off
set disassembly-flavor intel
target remote localhost:1234
break _reset_handler
break SysTick_Handler
break PendSV_Handler
break scheduler_priority_based
continue

