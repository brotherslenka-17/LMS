@echo off
cd /d "%~dp0"
py -3 gui.py
if errorlevel 1 pause
