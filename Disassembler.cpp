#include "disassembler.h"
#include "display.hpp"
#include <string>

Disassembler::Disassembler(cs_arch archi, cs_mode mode, bool detail)
  : init(false){

    if(cs_open(archi, mode, &handle) != CS_ERR_OK){
    	ColorChanger::SetColor(RED, true);
      	printf("CAPSTONE ERROR: API Fail !\n");
    	ColorChanger::Reset();
      	return;
    }
    this->insn = cs_malloc(this->handle);
    if(!this->insn){
    	ColorChanger::SetColor(RED, true);
      	printf("CAPSTONE ERROR: CS_MALLOC Fail !\n");
    	ColorChanger::Reset();
      	cs_close(&this->handle);
      	return;
    }


    this->init = true;
    if(detail)
      	cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
}


Disassembler::~Disassembler(){
  if(this->init){
    cs_close(&this->handle);
    cs_free(this->insn, 1);
  }
}


void Disassembler::Display(BYTE code[], size_t code_size, size_t count, uint64_t address, const std::map<DWORD,BYTE> &mapbp, const std::pair<DWORD,BYTE> &pairbp){
  	if(!init){
    	ColorChanger::SetColor(RED, true);
    	printf("CAPSTONE ERROR: Bad Disassembler initialisation, display failed !\n");
    	ColorChanger::Reset();
    	return;
  	}
  	ColorChanger::SetColor(BLUE);
  	printf("\n----------------------------------------------------------------\n\n");
  	uint64_t lastaddress = address;
  	size_t firstcount = count;
  	for(; count ; --count){
	  	if (code[0] == 0xCC){
	  		if(pairbp.first == address)
	  			code[0] = pairbp.second;
	  		else{
	  			std::map<DWORD,BYTE>::const_iterator it = mapbp.find(address);
	  			if (it != mapbp.end())
	  				code[0] = it->second;
	  		}
	  	}
  		const uint8_t* codebis = code;
  		if(!cs_disasm_iter(this->handle, &codebis, &code_size, &address, this->insn))
  			break;
  		code += address - lastaddress;
  		lastaddress = address;
  		std::string db(".byte");
  		if(!db.compare(insn->mnemonic)){
  			count++;
  			continue;
  		}
  	  	ColorChanger::SetColor(YELLOW, !(firstcount-count));
  	  	printf("0x%lX:", (DWORD)insn->address);
  	  	ColorChanger::SetColor(CYAN, !(firstcount-count));
  	  	printf("\t%s\t\t%s\n", insn->mnemonic,insn->op_str);
	}
  	ColorChanger::SetColor(BLUE);
  	printf("\n----------------------------------------------------------------\n\n");
  	ColorChanger::Reset();
}

uint64_t Disassembler::GetNextLineCall(const uint8_t* code, uint64_t address){
	if(!init){
    	ColorChanger::SetColor(RED, true);
    	printf("CAPSTONE ERROR: Bad Disassembler initialisation, display failed !\n");
    	ColorChanger::Reset();
    	return 0;
  	}
  	size_t code_size = 2*15;
  	cs_disasm_iter(this->handle, &code, &code_size, &address, this->insn);
  	std::string call("call");
  	if(!call.compare(insn->mnemonic)){
  		cs_disasm_iter(this->handle, &code, &code_size, &address, this->insn);
  		return insn->address;
  	}
  	else
  		return 0;
}

void Disassembler::DisplayInstruction(const uint8_t* code, uint64_t address){
  ColorChanger::SetColor(MAGENTA);
  std::cout << "0x" << std::hex << std::uppercase << address;
  ColorChanger::SetColor(BLACK, true);
  if (this->isValid(code, address))
    std::cout << " ( " << insn->mnemonic << "\t" << insn->op_str << " )" << std::endl;
}


bool Disassembler::isValid(const uint8_t* code, uint64_t address){
	if(!init){
    	ColorChanger::SetColor(RED, true);
    	printf("CAPSTONE ERROR: Bad Disassembler initialisation, display failed !\n");
    	ColorChanger::Reset();
    	return false;
  	}
  	size_t code_size = 15;
  	return cs_disasm_iter(this->handle, &code, &code_size, &address, this->insn);
}