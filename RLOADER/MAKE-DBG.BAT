@echo off
tasm /ml /m4 /os /zi main.asm main.obj
tlink /n /v /s main.obj, rloader.exe
