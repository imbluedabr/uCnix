target remote localhost:3333
set mem inaccessible-by-default off
set verbose on
set pagination off
set confirm off
set print pretty on
set breakpoint pending on
define skipbkpt
    set $pc = $pc + 2
end
file kernel.elf
monitor reset halt
monitor set vector-catch all
alias m = monitor
