@echo off
tasm /ml /m4 /os /q main.asm main.obj
tlink /n /s main.obj, rloader.exe
