target remote localhost:2331
set mem inaccessible-by-default off
set verbose on
set pagination off
set confirm off
set print pretty on
set breakpoint pending on

define skipbkpt
    set $pc = $pc + 2
end
break hardfault_handler
monitor reset
monitor halt
