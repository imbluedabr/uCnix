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

define get_preempt_frame
    if $argc < 1
        printf "Usage: check_frame <sp>\n"
        end
    end


    # Read the stacked registers
    set $a   = *(uint32_t*)($arg0 + 0)

    printf " r0   = 0x%x\n", $a
end

file kernel.elf
monitor reset halt
monitor set vector-catch all
alias m = monitor
