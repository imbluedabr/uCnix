target remote localhost:1234
set mem inaccessible-by-default off
set verbose on
set pagination off
set confirm off
set print pretty on

define skipbkpt
    set $pc = $pc + 2
end

file kernel.elf
b hardfault_handler
monitor reset halt
alias m = monitor
