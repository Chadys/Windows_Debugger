#pragma once
#include <windows.h>
#include <winnt.h>
#include <map>
#include <utility> 
#include "disassembler.h"


using std::pair;
using std::map;




class Breakpoint{
public:

	Breakpoint() { }
	void setUser(DWORD address);
	bool setUser(DWORD address, PROCESS_INFORMATION &pi, Disassembler &d);
	void setAllUser(PROCESS_INFORMATION &pi, Disassembler &d);
	bool setInvisible(DWORD address, PROCESS_INFORMATION &pi, Disassembler &d);
	bool addrPresentUser(DWORD address);
	void deleteUser(DWORD address, PROCESS_INFORMATION &pi);
	void deleteInvisible(PROCESS_INFORMATION &pi);
	void DeleteAllUserbp(PROCESS_INFORMATION &pi, bool existDebuggee);
	void listAllUser();
	const map<DWORD,BYTE> GetMap() const;
	const pair<DWORD,BYTE> GetPair() const;

private:
    map<DWORD,BYTE> Userbp; //contient tout les breakpoint set//
	pair<DWORD,BYTE> Bpinvisible;	//contient l'unique breakpoint "invisible" (sert au step)
	bool set(DWORD address, PROCESS_INFORMATION &pi, Disassembler &d, bool user);
};