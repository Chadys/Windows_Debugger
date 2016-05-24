#pragma once
#include "capstone/include/capstone.h"
#include <windows.h>
#include <iostream>
#include <map>

class Disassembler{
public:
	Disassembler(cs_arch archi = CS_ARCH_X86, cs_mode mode = CS_MODE_32, bool detail = false);
	~Disassembler();
	void Display(BYTE code[], size_t code_size, size_t count, uint64_t address, const std::map<DWORD,BYTE> &mapbp, const std::pair<DWORD,BYTE> &pairbpp);
	uint64_t GetNextLineCall(const uint8_t* code, uint64_t address);
	void DisplayInstruction(const uint8_t* code, uint64_t address);
	bool isValid(const uint8_t* code, uint64_t address);
private:
	cs_insn *insn;
	bool init;
	csh handle;
};