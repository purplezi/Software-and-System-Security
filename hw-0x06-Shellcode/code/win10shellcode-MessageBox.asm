00CFFD30 FC                   cld  
00CFFD31 33 D2                xor         edx,edx  
00CFFD33 B2 30                mov         dl,30h  
00CFFD35 64 FF 32             push        dword ptr fs:[edx]  
00CFFD38 5A                   pop         edx  
00CFFD39 8B 52 0C             mov         edx,dword ptr [edx+0Ch]  
00CFFD3C 8B 52 14             mov         edx,dword ptr [edx+14h]  
00CFFD3F 8B 72 28             mov         esi,dword ptr [edx+28h]  
00CFFD42 33 C9                xor         ecx,ecx  
00CFFD44 B1 18                mov         cl,18h  
00CFFD46 33 FF                xor         edi,edi  
00CFFD48 33 C0                xor         eax,eax  
00CFFD4A AC                   lods        byte ptr [esi]  
00CFFD4B 3C 61                cmp         al,61h  
00CFFD4D 7C 02                jl          00CFFD51  
00CFFD4F 2C 20                sub         al,20h  
00CFFD51 C1 CF 0D             ror         edi,0Dh  
00CFFD54 03 F8                add         edi,eax  
00CFFD56 E2 F0                loop        00CFFD48  
00CFFD58 81 FF 5B BC 4A 6A    cmp         edi,6A4ABC5Bh  
00CFFD5E 8B 5A 10             mov         ebx,dword ptr [edx+10h]  
00CFFD61 8B 12                mov         edx,dword ptr [edx]  
00CFFD63 75 DA                jne         00CFFD3F  
00CFFD65 8B 53 3C             mov         edx,dword ptr [ebx+3Ch]  
00CFFD68 03 D3                add         edx,ebx  
00CFFD6A FF 72 34             push        dword ptr [edx+34h]  
00CFFD6D 8B 52 78             mov         edx,dword ptr [edx+78h]  
00CFFD70 03 D3                add         edx,ebx  
00CFFD72 8B 72 20             mov         esi,dword ptr [edx+20h]  
00CFFD75 03 F3                add         esi,ebx  
00CFFD77 33 C9                xor         ecx,ecx  
00CFFD79 41                   inc         ecx  
00CFFD7A AD                   lods        dword ptr [esi]  
00CFFD7B 03 C3                add         eax,ebx  
00CFFD7D 81 38 47 65 74 50    cmp         dword ptr [eax],50746547h  
00CFFD83 75 F4                jne         00CFFD79  
00CFFD85 81 78 04 72 6F 63 41 cmp         dword ptr [eax+4],41636F72h  
00CFFD8C 75 EB                jne         00CFFD79  
00CFFD8E 81 78 08 64 64 72 65 cmp         dword ptr [eax+8],65726464h  
00CFFD95 75 E2                jne         00CFFD79  
00CFFD97 49                   dec         ecx  
00CFFD98 8B 72 24             mov         esi,dword ptr [edx+24h]  
00CFFD9B 03 F3                add         esi,ebx  
00CFFD9D 66 8B 0C 4E          mov         cx,word ptr [esi+ecx*2]  
00CFFDA1 8B 72 1C             mov         esi,dword ptr [edx+1Ch]  
00CFFDA4 03 F3                add         esi,ebx  
00CFFDA6 8B 14 8E             mov         edx,dword ptr [esi+ecx*4]  
00CFFDA9 03 D3                add         edx,ebx  
00CFFDAB 52                   push        edx  
00CFFDAC 33 FF                xor         edi,edi  
00CFFDAE 57                   push        edi  
00CFFDAF 68 61 72 79 41       push        41797261h  
00CFFDB4 68 4C 69 62 72       push        7262694Ch  
00CFFDB9 68 4C 6F 61 64       push        64616F4Ch  
00CFFDBE 54                   push        esp  
00CFFDBF 53                   push        ebx  
00CFFDC0 FF D2                call        edx  
00CFFDC2 68 33 32 01 01       push        1013233h  
00CFFDC7 66 89 7C 24 02       mov         word ptr [esp+2],di  
00CFFDCC 68 75 73 65 72       push        72657375h  
00CFFDD1 54                   push        esp  
00CFFDD2 FF D0                call        eax  
00CFFDD4 68 6F 78 41 01       push        141786Fh  
00CFFDD9 8B DF                mov         ebx,edi  
00CFFDDB 88 5C 24 03          mov         byte ptr [esp+3],bl  
00CFFDDF 68 61 67 65 42       push        42656761h  
00CFFDE4 68 4D 65 73 73       push        7373654Dh  
00CFFDE9 54                   push        esp  
00CFFDEA 50                   push        eax  
00CFFDEB FF 54 24 2C          call        dword ptr [esp+2Ch]  
00CFFDEF 57                   push        edi  
00CFFDF0 68 7A 69 7A 69       push        697A697Ah  
00CFFDF5 8B DC                mov         ebx,esp  
00CFFDF7 57                   push        edi  
00CFFDF8 53                   push        ebx  
00CFFDF9 53                   push        ebx  
00CFFDFA 57                   push        edi  
00CFFDFB FF D0                call        eax  
00CFFDFD 68 65 73 73 01       push        1737365h  
00CFFE02 8B DF                mov         ebx,edi  
00CFFE04 88 5C 24 03          mov         byte ptr [esp+3],bl  
00CFFE08 68 50 72 6F 63       push        636F7250h  
00CFFE0D 68 45 78 69 74       push        74697845h  
00CFFE12 54                   push        esp  
00CFFE13 FF 74 24 40          push        dword ptr [esp+40h]  
00CFFE17 FF 54 24 40          call        dword ptr [esp+40h]  
00CFFE1B 57                   push        edi  
00CFFE1C FF D0                call        eax