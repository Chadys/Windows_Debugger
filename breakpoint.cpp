#include <windows.h>
#include <map>
#include <stdio.h>
#include <winnt.h>
#include "breakpoint.hpp"
#include "Display.hpp"
using std::map;


void Breakpoint::setUser(DWORD address){
	Userbp[address]=0xCC;
}

bool Breakpoint::setUser(DWORD address, PROCESS_INFORMATION &pi, Disassembler &d){
	return this->set(address, pi, d, true);
}

bool Breakpoint::setInvisible(DWORD address, PROCESS_INFORMATION &pi, Disassembler &d){
	return this->set(address, pi, d, false);
}

bool Breakpoint::set(DWORD address, PROCESS_INFORMATION &pi, Disassembler &d, bool user){
    BYTE cInstruction[15], bp;
    DWORD dwReadBytes;

    if(!ReadProcessMemory(pi.hProcess, (void*)address, &cInstruction, 15, &dwReadBytes)){
    	ColorChanger::SetColor(RED, true);
    	std::cout << "BREAKPOINT ERROR: Not able to put breakpoint at 0x" << std::hex << std::uppercase << address << std::endl;
    	std::cout << GetLastErrorAsString() << std::endl;
    	ColorChanger::Reset();
    	return false;
    }
    if(!d.isValid(cInstruction, address)){
    	ColorChanger::SetColor(RED, true);
    	std::cout << "BREAKPOINT ERROR: Not able to put breakpoint at 0x" << std::hex << std::uppercase << address << std::endl;
    	std::cout << "This adress isn't an instruction." << std::endl;
    	ColorChanger::Reset();
    	return false;
    }
    if(*cInstruction== 0xCC)
    	return true;
 
    bp = 0xCC;
    if(!WriteProcessMemory(pi.hProcess, (void*)address,&bp, 1, &dwReadBytes)
    || !FlushInstructionCache(pi.hProcess,(void*)address,1)){
    	ColorChanger::SetColor(RED, true);
    	std::cout << "BREAKPOINT ERROR: Not able to put breakpoint at 0x" << std::hex << std::uppercase << address << std::endl;
    	std::cout << GetLastErrorAsString() << std::endl;
    	ColorChanger::Reset();
    	return false;
    }

    if(user)
		Userbp[address]=*cInstruction;
	else
		Bpinvisible = std::make_pair(address,*cInstruction);

    return true;
}

void Breakpoint::setAllUser(PROCESS_INFORMATION &pi, Disassembler &d){
	for (map<DWORD,BYTE>::iterator x=Userbp.begin(); x!=Userbp.end();){
    	if(this->set(x->first, pi, d, true)){
    		Prompt();
    	    printf("Breakpoint at 0x%lX was set.\n",x->first);
    	    ++x;
    	}
    	else
    		Userbp.erase(x++);
    }
}

bool Breakpoint::addrPresentUser(DWORD address){
       	return  Userbp.count(address);
}

void Breakpoint::deleteUser(DWORD address, PROCESS_INFORMATION &pi){
    BYTE cInstruction;
    DWORD dwReadBytes;

    cInstruction = Userbp[address];
    if(!WriteProcessMemory(pi.hProcess, (void*)address,&cInstruction, 1, &dwReadBytes)
    || !FlushInstructionCache(pi.hProcess,(void*)address,1)){
    	ColorChanger::SetColor(RED, true);
    	std::cout << "BREAKPOINT ERROR: Not able to delete breakpoint at 0x" << std::hex << std::uppercase << address << std::endl;
    	std::cout << GetLastErrorAsString() << std::endl;
    	ColorChanger::Reset();
    }
}	

void Breakpoint::deleteInvisible(PROCESS_INFORMATION &pi){
    if(Bpinvisible.first){
    	BYTE cInstruction;
    	DWORD dwReadBytes;
		 
		//Decremente eip et place une exeption single step//
    	DWORD address = Bpinvisible.first;
    	Bpinvisible.first = 0;
	
    	cInstruction = Bpinvisible.second;
    	WriteProcessMemory(pi.hProcess, (void*)address,&cInstruction, 1, &dwReadBytes);
    	FlushInstructionCache(pi.hProcess,(void*)address,1);
	}
}

void Breakpoint::DeleteAllUserbp(PROCESS_INFORMATION &pi, bool existDebuggee){
	BYTE cInstruction;
    DWORD dwReadBytes;
    DWORD address;
    if(Userbp.empty()){
    	Prompt();
    	printf("No breakpoint was initialized.\n");
    	return;    	
    }
    if (!existDebuggee){
    	Userbp.clear();
    	Prompt();
    	printf("All breakpoint were deleted.\n");
    	return;
    }
	for (map<DWORD,BYTE>::iterator x=Userbp.begin(); x!=Userbp.end();){
		address=x->first;
		cInstruction = x->second;
    	if(!WriteProcessMemory(pi.hProcess, (void*)address,&cInstruction, 1, &dwReadBytes)
    	|| !FlushInstructionCache(pi.hProcess,(void*)address,1)){
    		ColorChanger::SetColor(RED, true);
    		printf("BREAKPOINT ERROR: Not able to delete breakpoint at %lX\n%s\n",address,GetLastErrorAsString().c_str());
    		ColorChanger::Reset();
    		++x;
    		continue;
    	}
    	Userbp.erase(x++);
    	Prompt();
		printf("Breakpoint at address 0x%lX was deleted.\n",address);
	}
}

void Breakpoint::listAllUser(){
	int compteur=0;
	printf("\n");
	ColorChanger::SetColor(MAGENTA, true);
    for (map<DWORD,BYTE>::iterator x=Userbp.begin(); x!=Userbp.end();x++,++compteur){
        DWORD address=x->first;
        printf(" Breakpoint %d at address 0x%lX.\n",compteur,address);
    }
    if(compteur==0){
    	ColorChanger::SetColor(MAGENTA);
        printf(" No Breakpoint was set yet.\n");
    }
    printf("\n");
	ColorChanger::Reset();
}

const map<DWORD,BYTE> Breakpoint::GetMap() const{
	return this->Userbp;
}
const pair<DWORD,BYTE> Breakpoint::GetPair() const{
	return this->Bpinvisible;
}