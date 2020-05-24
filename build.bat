@echo off
rasm86 pcetime.asm $pzsz
linkcmd pcetime
rasm86 pceinit.asm $pzsz
linkcmd pceinit
rasm86 pcever.asm $pzsz
linkcmd pcever
rasm86 pcemnt.asm $pzsz
linkcmd pcemnt
rasm86 cls.asm $pzsz
linkcmd cls
rasm86 ver.asm $pzsz
linkcmd ver

