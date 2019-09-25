@echo on
encode
@for %%c in (*.txt) do zint -b58 --scale=4 -o %%c.png -i %%c
@echo off
pause