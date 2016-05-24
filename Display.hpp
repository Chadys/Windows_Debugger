#pragma once
#include "disassembler.h"
#include "breakpoint.hpp"
#include <windows.h>
#include <utility>
#include <stdio.h>
#include <string>
#include <list>
#include <map>
using std::list;
using std::map;


/* Standard error macro for reporting API errors */ 
#define PERR(bSuccess, api){if(!(bSuccess)) printf("%s:Error %lu from %s \
    on line %d\n", __FILE__, GetLastError(), api, __LINE__);}

void Prompt();
void DisplayHelp();
void DisplayNInstruction(int nbInstr,DWORD address, PROCESS_INFORMATION &pi,Disassembler &disassembler, const map<DWORD,BYTE> &mapbp, const std::pair<DWORD,BYTE> &pairbp);
std::string GetLastErrorAsString();
void DisplayData(std::string command, std::string s_address, size_t size, PROCESS_INFORMATION &pi);

list<DWORD> getvalueregister(DWORD registre, PROCESS_INFORMATION &pi);
bool DisplayReg(std::string reg, CONTEXT &lcContext, PROCESS_INFORMATION &pi, Disassembler &d);
void DisplayAllReg(CONTEXT &lcContext, PROCESS_INFORMATION &pi, Disassembler &d);



enum Color{
	BLACK = 0,
	RED = FOREGROUND_RED,
	BLUE = FOREGROUND_BLUE,
	GREEN = FOREGROUND_GREEN,
	YELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
	MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
	CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
	WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
};

class ColorChanger{
public:
	static bool Init();
	static void SetColor(Color color, bool intens = false);
	static void Reset();
	static void cls();
private:
	static HANDLE hStdout;
	static WORD wOldColor;
	static bool valid;
	static CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
};