@echo off
rem Builds src\CPROJ.EXE with JWasm, a free MASM-compatible assembler:
rem https://github.com/Baron-von-Riedesel/JWasm  (put JWasm.exe on the PATH)
rem Run the result in DOSBox or DOSBox-X.
cd /d "%~dp0src"
jwasm -mz -Fo CPROJ.EXE CPROJ.ASM
