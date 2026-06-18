#!/bin/sh
JLinkExe -device MCXA153 -if SWD -speed 4000 -CommanderScript ./scripts/flash.jlink
