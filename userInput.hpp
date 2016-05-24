#pragma once
#include <windows.h>
#include "breakpoint.hpp"

//fonction qui gere toutes les commandes du dbg//
unsigned int inputUser(char * nameofprocess, Breakpoint &breakpoint, CONTEXT &lcContext,STARTUPINFO &si, PROCESS_INFORMATION &pi, Disassembler &disassembler, bool debuggee_stopped = false);

DWORD hexatodword(const char * hex);