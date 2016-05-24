#include "disassembler.h"
#include "breakpoint.hpp"
#include "getNameFromHandle.hpp"
#include "userInput.hpp"
#include "Display.hpp"



#include <iostream>
#include <windows.h>
#include <string>
#include <map>
#include <signal.h>
#include <stdio.h>


using std::cout;
using std::endl;
using std::cin;
using std::string;
using std::map;


int ctrl_c=0;
void intHandler(int sig) {
    if(ctrl_c>=1)
        ctrl_c++;
    Prompt();
    printf("Use \"q\" to quit.\n");
    signal(sig, intHandler);
}




void EnterDebugLoop(char * nameofprocess){

    DEBUG_EVENT debug_event;
    STARTUPINFO si; 
    PROCESS_INFORMATION pi;
    
    std::map < LPVOID, string > DllNameMap;//contient le nom de toute les dll charger//

    CONTEXT lcContext;
    lcContext.ContextFlags = CONTEXT_FULL;

    Disassembler d;
    Breakpoint breakpoint;
    DWORD addressLastbreakpoint=0;
    
    string s;
    bool firstBreakpoint=true;
    unsigned int step_count = 0;
    DWORD dwContinueStatus = DBG_CONTINUE;

    GetThreadContext(pi.hThread, &lcContext);
    ctrl_c=1;
    inputUser(nameofprocess,breakpoint,lcContext,si,pi,d);
    ctrl_c--;

    while(1){
        //Attend un evenement//
        WaitForDebugEvent(&debug_event, INFINITE);
        ColorChanger::Reset();
        switch (debug_event.dwDebugEventCode){
            case EXCEPTION_DEBUG_EVENT: 
                switch(debug_event.u.Exception.ExceptionRecord.ExceptionCode){ 

                    case DBG_CONTROL_C: 
                        GetThreadContext(pi.hThread, &lcContext);
                        if(ctrl_c==0){
                            Prompt();
                            printf("Signal SIGINT received.\n");
                            ctrl_c++;
                            step_count =  inputUser(nameofprocess,breakpoint,lcContext,si,pi,d);
                            ctrl_c--;
                        }
                        else ctrl_c--;
                    break;


                    case EXCEPTION_BREAKPOINT: 
                        GetThreadContext(pi.hThread, &lcContext);
                        if(!firstBreakpoint) //on ignore le premier breakpoint mis par le kernel//
                        {
                            --lcContext.Eip;
                            SetThreadContext(pi.hThread, &lcContext);
                            breakpoint.deleteInvisible(pi);
                            if(breakpoint.addrPresentUser(lcContext.Eip)){
                                addressLastbreakpoint = lcContext.Eip;
                                Prompt();
                                printf("Breakpoint reached at address 0x%lX \n",addressLastbreakpoint);
                                breakpoint.deleteUser(lcContext.Eip, pi);
                                lcContext.EFlags |= 0x100;
                                SetThreadContext(pi.hThread, &lcContext);
                                DisplayNInstruction(5,lcContext.Eip,pi,d, breakpoint.GetMap(), breakpoint.GetPair());
                                ctrl_c=1;
                                step_count = inputUser(nameofprocess,breakpoint,lcContext,si,pi,d);
                                 ctrl_c--;
                            }
                            else{
                                Prompt();
                                printf("Step done, adress 0x%lX reached\n",lcContext.Eip);
                                DisplayNInstruction(5,lcContext.Eip,pi,d, breakpoint.GetMap(), breakpoint.GetPair());
                                ctrl_c=1;
                                step_count = inputUser(nameofprocess,breakpoint,lcContext,si,pi,d);
                                 ctrl_c--;
                            }
                        }
                        else {
                            //breakpoint.setInvisible(lcContext.Eip,si,pi);
                            breakpoint.setAllUser(pi, d);
                            lcContext.EFlags |= 0x100;
                            SetThreadContext(pi.hThread, &lcContext);
                            step_count=1;
                        }
                        break;
                 
                    case EXCEPTION_SINGLE_STEP: 
                        GetThreadContext(pi.hThread, &lcContext);
                        if(firstBreakpoint){
                            firstBreakpoint=false;
                            //breakpoint.setInvisible(lcContext.Eip,si,pi);
                            lcContext.EFlags |= 0x100;
                            SetThreadContext(pi.hThread, &lcContext);
                            break;
                        }
                        if(addressLastbreakpoint != 0){
                            breakpoint.setUser(addressLastbreakpoint,pi, d); //on remet le breakpoint pour qu'il est une persistance//
                            addressLastbreakpoint = 0;
                        }
                        if(step_count){
                            step_count--;
                            if (!step_count){
                                Prompt();
                                printf("Step done, adress 0x%lX reached\n",lcContext.Eip);
                                DisplayNInstruction(5,lcContext.Eip,pi,d, breakpoint.GetMap(), breakpoint.GetPair());
                                ctrl_c=1;
                                step_count = inputUser(nameofprocess,breakpoint,lcContext,si,pi,d);
                                 ctrl_c--;
                                break;
                            }
                            lcContext.EFlags |= 0x100;
                            SetThreadContext(pi.hThread, &lcContext);
                            //breakpoint.setInvisible(lcContext.Eip,si,pi); //on met un breakpoint invisible à l'adresse de la prochaine instruction
                        }
                        break;
                    default:
                        if(debug_event.u.Exception.dwFirstChance == 1){
                            Prompt();
                            printf("First chance exception at 0x%lX, exception-code: 0x%lX\n",   (DWORD)debug_event.u.Exception.ExceptionRecord.ExceptionAddress,(DWORD)debug_event.u.Exception.ExceptionRecord.ExceptionCode);
                        }
                        dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;   
                }
                break;
 
            case CREATE_THREAD_DEBUG_EVENT: 
                Prompt();
                printf("Thread 0x%lX (Id: %lu) created at: 0x%lX\n", (DWORD)debug_event.u.CreateThread.hThread, debug_event.dwThreadId, (DWORD)debug_event.u.CreateThread.lpStartAddress);
                break;

            case CREATE_PROCESS_DEBUG_EVENT: 
                ColorChanger::cls();
                Prompt();
                ColorChanger::SetColor(WHITE, true);
                printf("%s is running \n",nameofprocess);
                ColorChanger::Reset();
                break;
 
            case EXIT_THREAD_DEBUG_EVENT: 
                Prompt();
                printf("Thread (Id: %lu) exited with code: 0x%lX\n", debug_event.dwThreadId,debug_event.u.ExitThread.dwExitCode);
                break;
 
            case EXIT_PROCESS_DEBUG_EVENT: 
                Prompt();
                ColorChanger::SetColor(WHITE, true);
                printf("%s exited with code 0x%lX\n",nameofprocess,debug_event.u.ExitProcess.dwExitCode);
                ColorChanger::Reset();
                firstBreakpoint=true;
                GetThreadContext(pi.hThread, &lcContext);
                ctrl_c=1;
                inputUser(nameofprocess,breakpoint,lcContext,si,pi,d, true); 
                ctrl_c--;
                break;
 
            case LOAD_DLL_DEBUG_EVENT:  
                s= GetFileNameFromHandle(debug_event.u.LoadDll.hFile); //recupere le chemin de la dll///
                DllNameMap.insert(std::make_pair( debug_event.u.LoadDll.lpBaseOfDll,s.c_str()));
                Prompt();
                ColorChanger::SetColor(BLACK, true);
                printf("%s - Loaded at 0x%lX\n",s.c_str(), (DWORD)debug_event.u.LoadDll.lpBaseOfDll);
                ColorChanger::Reset();
                break;
 
            case UNLOAD_DLL_DEBUG_EVENT:
                if(DllNameMap.find(debug_event.u.UnloadDll.lpBaseOfDll) != DllNameMap.end()){ //verifie si le string de la dll a ete mis dans la map//
                    Prompt();
                    cout<<"DLL "<< DllNameMap[debug_event.u.UnloadDll.lpBaseOfDll]<<" Unloaded"<<endl;
                }
                else{
                    Prompt();
                    ColorChanger::SetColor(BLACK, true);
                    printf("DLL at 0x%lX Unloaded\n", (DWORD)debug_event.u.UnloadDll.lpBaseOfDll);
                    ColorChanger::Reset();
                }
                break;
 
            case OUTPUT_DEBUG_STRING_EVENT: 
                break;

            case RIP_EVENT:
                 break;
        }
      
        //continue l'execution du debugguee//
        ColorChanger::SetColor(WHITE, true);
        ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, dwContinueStatus);
                      
         // Reset//
        dwContinueStatus = DBG_CONTINUE;
    }
}

int main(int argc,char ** argv){
    signal(SIGINT, intHandler);
    ColorChanger::Init();
    
    if(argc!=2){
        Prompt();
        cout<<"Wrong number of arguments to 'dbg'."<<endl;
        Prompt();
        cout<<"Please execute as: dbg.exe name"<<endl;
        exit(0);
    }
    
    EnterDebugLoop(argv[1]);

    return 0;
}