#include <windows.h>
#include <Psapi.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <string>
#include "Display.hpp"
#include "userInput.hpp"



using std::string;
using std::cin;
using std::cout;
using std::endl;



DWORD hexatodword(const char * hex){
  	DWORD x;   

  	x = strtol( hex, 0, 16 );

  	return x;
}

void createprocess(char * nameofprocess, STARTUPINFO &si, PROCESS_INFORMATION &pi,string arguments){  
  	ZeroMemory( &si, sizeof(si) ); 
  	si.cb = sizeof(si); 
  	ZeroMemory( &pi, sizeof(pi) );
  
  	if(!CreateProcess ( nameofprocess,(char *) arguments.c_str(), NULL, NULL, FALSE, 
                DEBUG_ONLY_THIS_PROCESS, NULL,NULL, &si, &pi )){
    cout<<endl;
    Prompt();
    cout<<"CreateProcess"<<" failed with error " << GetLastErrorAsString() << endl;
    Prompt();
    cout<<"Please check if "<<nameofprocess<<" is in your directory"<<endl;

    exit(1);                  
  }
                
}

unsigned int inputUser(char * nameofprocess, Breakpoint &breakpoint, CONTEXT &lcContext,STARTUPINFO &si, PROCESS_INFORMATION &pi, Disassembler &disassembler, bool debuggee_stopped){
    string buff;
    static bool existDebuggee=0;
    static string argument("");

    if (!argument.empty()){
    	createprocess(nameofprocess,si,pi,argument);
        argument = "";
        existDebuggee=1;
        return 0;
    }
    if (debuggee_stopped)
    	existDebuggee=0;
    
    for(;;){
    	Prompt();
        cout<<"Enter a command:"<<endl;
        std::getline(cin,buff);
        if (cin.fail() || cin.eof()){
        	cin.clear();
        	continue;
        }
        std::vector<string> v;
        std::istringstream s(buff);
        string command;
        
        if(s >> command){
       		if(!command.compare("q")){
           		if(existDebuggee){
                	TerminateProcess(pi.hProcess, 0);
                	existDebuggee=0;  
           		}
           		exit(0);
       		}

       		else if(!command.compare("g")){
            	if(existDebuggee)
            	    break;
            	Prompt();
            	cout<<"No debuggee started yet. Please use the command \"s\" . "<<endl;
       		}
               
       		else if(!command.compare("s")){
           		if(existDebuggee){
           			Prompt();
           			cout << "Debuggee has already started. Do you really want to restart it ? [y or n]" << endl;
           			for(;;){
        				std::getline(cin,buff);
        				if (cin.fail() || cin.eof()){
        					cin.clear();
        					continue;
        				}
        				if(buff.compare("y") && buff.compare("n")){
        					Prompt();
        					cout << "Invalid answer, please use \"y\" or \"n\"." << endl;
        				}
        				else
        					break;
        			}
        			if (buff[0] == 'y'){
            	    	TerminateProcess(pi.hProcess, 0);
            	    	argument = nameofprocess;
           				while(s >> command)
           					argument += " "+command;
            	    	return 0;
        			}
        			else
        				continue;
           		}
           		argument = nameofprocess;
           		while(s >> command)
           			argument += " "+command; 
           		createprocess(nameofprocess,si,pi,argument);
           		argument = "";
           		existDebuggee=1;
           		break;
       		}     
       
       		else if(!command.compare("r")){
       			if (!existDebuggee){
       				Prompt();
       				cout<<"No debuggee started yet. Please use the command \"s\" . "<<endl;
       				continue;
       			}
               	if (!(s >> command))
               		DisplayAllReg(lcContext, pi, disassembler);
               	else{
               		printf("\n");
               		if (!DisplayReg(command, lcContext, pi, disassembler)){
               			Prompt();
               			printf("Invalid register \"%s\", use EAX,EBX, ECX, EDX, ESI, EDI, EBP, ESP, EIP or EFLAGS\n",command.c_str());
               		}
               		else
               			printf("\n");
               	}
               	continue;
       		}
                
       		else if(!command.compare("h")){
            	DisplayHelp();
            	continue;
       		}
      
       		else if(!command.compare("bl")){
       			breakpoint.listAllUser();
               	continue;
       		}
       
       		else if(!command.compare("bc")){
               	breakpoint.DeleteAllUserbp(pi,existDebuggee);
               	continue;
       		}
       
       		else if(!command.compare("bp")){
       			if(!(s >> command)){
       				Prompt();
       				printf("Error command: \"%s\". Try \"bp 0xAddress\"\n", buff.c_str());
       			}
       			else{
       				DWORD address = hexatodword(command.c_str());
       				if (address){
       					if(!existDebuggee){
            				breakpoint.setUser(address);
            				Prompt();
            				printf("Breakpoint at 0x%lX saved for future load.\n",address);
            				continue;
            			}
       					if (breakpoint.setUser(address, pi, disassembler)){
       						Prompt();
       						printf("Breakpoint at 0x%lX was set.\n",address);
       					}
       					else{
       						Prompt();
       						printf("Invalid address %s\n",command.c_str());
       					}
       				}
       				else{
       					Prompt();
       					printf("Invalid address %s\n",command.c_str());
       				}
       			}
       			continue;
       		}
       
       		else if(!command.compare("t")){
            	if(!existDebuggee){
            		Prompt();
            		cout<<"No debuggee started yet. Please use the command \"s\" . "<<endl;
            		continue;
            	}
                lcContext.EFlags |= 0x100;
                SetThreadContext(pi.hThread, &lcContext);
                unsigned int count;
                if(!(s >> count))
                	count=1;
                return count;
       		}
          
      		else if(!command.compare("p")){
            	if(!existDebuggee){
            		Prompt();
            		cout<<"No debuggee started yet. Please use the command \"s\" . "<<endl;
            		continue;
            	}
  				DWORD dwReadBytes;
  				BYTE cInstruction[15];
  				ReadProcessMemory(pi.hProcess, (void*)lcContext.Eip, cInstruction, 15, &dwReadBytes);
  				uint64_t adress;
      			if((adress = disassembler.GetNextLineCall(cInstruction,lcContext.Eip)))
                    breakpoint.setInvisible(adress,pi, disassembler);
                else{
                	lcContext.EFlags |= 0x100;
                	SetThreadContext(pi.hThread, &lcContext);
                	return 1;
                }
                break;
      		}
    
       		else if(command[0]=='d'){
           		size_t size = 0;
           		string com = command;
           		if(!command.compare("db"))
             		size = sizeof(BYTE);
           		else if(!command.compare("dw"))
             		size = sizeof(WORD);
           		else if(!command.compare("dd"))
             		size = sizeof(DWORD);
           		else{
           			Prompt();
           			printf("Error command: \"%s\". Try \"h\"\n", buff.c_str());
           			continue;
           		}
             	if(!(s >> command)){
             		Prompt();
              		printf("Error command: \"%s\". Try \"%s 0xAddress\"\n", buff.c_str(), com.c_str());
             	}
            	else{
       				if (!existDebuggee){
       					Prompt();
       					cout<<"No debuggee started yet. Please use the command \"s\" . "<<endl;
       					continue;
       				}
                  	DisplayData(com,command,size,pi);
            	}
             	continue;
            }
       		else{
       			Prompt();
        	   	cout<<"Undefined command: \""<<buff<<"\". Try \"h\"."<<endl;
       		}
   		}
       	else{
       		Prompt();
           	cout<<"Undefined command: \""<<buff<<"\". Try \"h\"."<<endl;
       	}
    }
    return 0;
}