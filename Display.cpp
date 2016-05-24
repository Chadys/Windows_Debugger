#include "Display.hpp"
#include "userInput.hpp"
#include <algorithm>

void Prompt(){
    ColorChanger::SetColor(BLUE, true);
    printf("[dbg]: ");
    ColorChanger::Reset();
}

void ColorHelp(const char* com, const char* expl){
    ColorChanger::SetColor(RED, true);
    printf("%s", com);
    ColorChanger::SetColor(WHITE);
    printf("%s\n", expl);
}

void DisplayHelp(){
    ColorChanger::SetColor(RED, true);
    printf("\n--------------HELP--------------\n\n");
    ColorChanger::SetColor(RED);
    printf("\tCommandes :\n\n");
    ColorHelp(" bc -- ", "Delete all breakpoints.\n");
    ColorHelp(" bl -- ", "List all recorded breakpoints.\n");
    ColorHelp(" bp [address] -- ", "Put a breakpoint at the specified address.\n");
    ColorHelp(" db, dd, dw [address] -- ", "Display respectively the BYTE/WORD/DWORD located at the specified address.\n");
    ColorHelp(" h -- ", "Display this help.\n");
    ColorHelp(" g -- ", "Continue program execution.\n");
    ColorHelp(" p -- ", "Step over : Execute next instruction without entering eventual subprocedure (step over).\n");
    ColorHelp(" s ([arg1] ([arg2] (...))) -- ", "Start the program (with eventual arguments).\n");
    ColorHelp(" r ([register]) -- ", "Display the content of all registers of the running program (or of only one register if specified).\n");
    ColorHelp(" t ([number]) -- ", "Step into : Execute \'number\' instruction (one if \'number\' isn\'t specified).\n");
    ColorHelp(" q -- ", "Quit the debugger.\n");
    ColorChanger::SetColor(RED, true);
    printf("\n--------------------------------\n\n");
    ColorChanger::Reset();
}


void DisplayNInstruction(int nbInstr,DWORD address, PROCESS_INFORMATION &pi, Disassembler &disassembler, const map<DWORD,BYTE> &mapbp, const std::pair<DWORD,BYTE> &pairbp){
  DWORD dwReadBytes;
  BYTE cInstruction[nbInstr*15]; //la longueur max d'une instruction en assembleur x86 est de 15 octets
  ReadProcessMemory(pi.hProcess, (void*)address, cInstruction, nbInstr*15, &dwReadBytes);
  disassembler.Display(cInstruction,dwReadBytes,nbInstr,address, mapbp, pairbp);     
}

std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return "No error message found"; //No error message has been recorded

    LPSTR messageBuffer = (LPSTR)0;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

void DisplayData(std::string command, std::string s_address, size_t size, PROCESS_INFORMATION &pi){
    DWORD address = hexatodword(s_address.c_str());
    if (address){
        DWORD cInstruction,dwReadBytes;
        if(ReadProcessMemory(pi.hProcess, (void*)address, &cInstruction, size, &dwReadBytes)){
    		ColorChanger::SetColor(MAGENTA, true);
            printf("\n%s: 0x%lX\n\n",command.c_str(), cInstruction); //AFICHAGE ICI//
            ColorChanger::Reset();
        }
        else{
            std::string error = GetLastErrorAsString();
    		ColorChanger::SetColor(RED, true);
            printf("DATA ERROR: Can't read at address 0x%lX \n",address);
    		ColorChanger::Reset();
        }
    }
    else{
    	Prompt();
        printf("Invalid address %s\n",s_address.c_str());
    }
}




/*-------------------------------REGISTERS---------------------------*/

bool DisplayReg(std::string reg, CONTEXT &lcContext, PROCESS_INFORMATION &pi, Disassembler &d){
    std::transform(reg.begin(), reg.end(), reg.begin(), ::toupper);
    list<DWORD> l;
    if (!reg.compare("EIP")){
        DWORD dwReadBytes;
        BYTE cInstruction[15];

        ColorChanger::SetColor(GREEN, true);
        std::cout << reg;
        ColorChanger::SetColor(WHITE);
        std::cout<< ": ";

        if (ReadProcessMemory(pi.hProcess, (void*)lcContext.Eip, cInstruction, 15, &dwReadBytes))
            d.DisplayInstruction(cInstruction,lcContext.Eip);
        return true;
    }
    if (!reg.compare("EFLAGS")){
        std::string list[] = {"carry","parity", "adjust", "zero", "sign", "trap", "interrupt", "direction", "overflow"};
        bool bright[9] = {false};
        ColorChanger::SetColor(GREEN,true);
        std::cout << reg ;
        ColorChanger::SetColor(WHITE);
        std::cout<< ":";
        ColorChanger::SetColor(BLACK, true);
        std::cout<<" 0x" << std::hex << std::uppercase << lcContext.EFlags;
        ColorChanger::SetColor(WHITE);
        std::cout << " (";
        int i, j;
        bool flag;
        for (i = 0, j=0 ; i < 12 ; ++i, j++){
            flag = lcContext.EFlags >> i & 1;
            if (flag){
            	bright[j] = true;
                std::transform(list[j].begin(), list[j].end(), list[j].begin(), ::toupper);
            }
            if (i<6)
                ++i;
        }
        std::string sep = " ";
        for (i = 0 ; i < 9; i++){
        	ColorChanger::SetColor(RED, bright[i]);
        	if(i==8)
        		sep = "";
            std::cout << list[i] << sep;
        }
        ColorChanger::SetColor(WHITE);
        std::cout << ")" << std::endl;
        return true;
    }
    if (!reg.compare("EAX"))
        l = getvalueregister(lcContext.Eax, pi);
    else if (!reg.compare("EBX"))
        l = getvalueregister(lcContext.Ebx, pi);
    else if (!reg.compare("ECX"))
        l = getvalueregister(lcContext.Ecx, pi);
    else if (!reg.compare("EDX"))
        l = getvalueregister(lcContext.Edx, pi);
    else if (!reg.compare("ESI"))
        l = getvalueregister(lcContext.Esi, pi);
    else if (!reg.compare("EDI"))
        l = getvalueregister(lcContext.Edi, pi);
    else if (!reg.compare("EBP"))
        l = getvalueregister(lcContext.Ebp, pi);
    else if (!reg.compare("ESP"))
        l = getvalueregister(lcContext.Esp, pi);
    else return false;

    ColorChanger::SetColor(GREEN, true);
    std::cout << reg;
    ColorChanger::SetColor(WHITE);
    std::cout<<": ";
    int cmp=0;
    for (list<DWORD>::iterator it = l.begin() ; it != l.end() ; it++, cmp++){
        if(cmp>5){
        	ColorChanger::SetColor(WHITE);
            std::cout << " --> ...";
            break;
        }
        if(it != l.begin()){
        	ColorChanger::SetColor(WHITE);
        	std::cout<<" --> ";
        	if (*it == *(l.begin())){
        		ColorChanger::SetColor(GREEN, true);
        		std::cout << reg ;
        	    break;
        	}
        }
        ColorChanger::SetColor(GREEN);
        std::cout << "0x" << std::hex << std::uppercase << *it;
    }
    std::cout << std::endl;
    ColorChanger::Reset();
    return true;
} 

void DisplayAllReg(CONTEXT &lcContext, PROCESS_INFORMATION &pi, Disassembler &d){
	ColorChanger::SetColor(GREEN);
	printf("\n------------------------------------------\n\n");
	std::string list[] = {"EAX","EBX", "ECX", "EDX", "ESI", "EDI", "EBP", "ESP", "EIP", "EFLAGS"};
	for (int i = 0 ; i < 10; i++)
		DisplayReg(list[i], lcContext, pi, d); 
    ColorChanger::SetColor(GREEN);
    printf("\n------------------------------------------\n\n");
    ColorChanger::Reset();
}

list<DWORD> getvalueregister(DWORD registre, PROCESS_INFORMATION &pi){
  DWORD dataRead, dwReadBytes;
  list<DWORD> res;
  map<DWORD,bool> dejavu;

  res.push_back(registre);

  while( ReadProcessMemory(pi.hProcess, (void*)registre, &dataRead, sizeof(DWORD), &dwReadBytes)){
    if(dejavu.find(registre)!=dejavu.end())
        break;
    res.push_back(dataRead);
    registre=dataRead;
    dejavu[registre]=true;
        break;
  }
  return res;
}



/*-----------------------------CONSOLE OPTION-------------------------*/

HANDLE ColorChanger::hStdout = INVALID_HANDLE_VALUE;
WORD ColorChanger::wOldColor;
bool ColorChanger::valid = false;
CONSOLE_SCREEN_BUFFER_INFO ColorChanger::csbiInfo;

bool ColorChanger::Init(){
	valid = false;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE){
		printf("MSDN ERROR: GetStdHandle Fail !\n");
		return valid;
	}
	if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo)){
		printf("MSDN ERROR: GetConsoleScreenBufferInfo Fail !\n");
		return valid;
	}
	wOldColor = csbiInfo.wAttributes;
	valid = true;
	return valid;
}

void ColorChanger::SetColor(Color color, bool intens){
	if (valid){
		WORD col = color;
		if (intens)
			col |= FOREGROUND_INTENSITY;
		if (!SetConsoleTextAttribute(hStdout, col))
			printf("MSDN ERROR: SetConsoleTextAttribute Fail !\n");
		return;
	}
	printf("ColorChanger ERROR: Bad initialization !\n");
}

void ColorChanger::Reset(){
	SetConsoleTextAttribute(hStdout, wOldColor);
}

void ColorChanger::cls()
{
	if (!valid){
		printf("ColorChanger ERROR: Bad initialization !\n");
		return;
	}
    COORD coordScreen = { 0, 0 };    /* here's where we'll home the
                                        cursor */ 
    BOOL bSuccess;
    DWORD cCharsWritten;
    DWORD dwConSize;                 /* number of character cells in
                                        the current buffer */ 

    /* get the number of character cells in the current buffer */ 

    bSuccess = GetConsoleScreenBufferInfo( hStdout, &csbiInfo );
    PERR( bSuccess, "GetConsoleScreenBufferInfo" );
    dwConSize = csbiInfo.dwSize.X * csbiInfo.dwSize.Y;

    /* fill the entire screen with blanks */ 

    bSuccess = FillConsoleOutputCharacter( hStdout, (TCHAR) ' ',
       dwConSize, coordScreen, &cCharsWritten );
    PERR( bSuccess, "FillConsoleOutputCharacter" );

    /* now set the buffer's attributes accordingly */ 

    bSuccess = FillConsoleOutputAttribute( hStdout, csbiInfo.wAttributes,
       dwConSize, coordScreen, &cCharsWritten );
    PERR( bSuccess, "FillConsoleOutputAttribute" );

    /* put the cursor at (0, 0) */ 

    bSuccess = SetConsoleCursorPosition( hStdout, coordScreen );
    PERR( bSuccess, "SetConsoleCursorPosition" );
    return;
 }