# 内存管理

## 实验要求

- [x] 阅读[VirtualAlloc](https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc)、[VirtualFree](https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualfree)、[VirtualProtect](https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualprotect)等函数的官方文档
- [x] 编程使用malloc分配一段内存，测试是否这段内存所在的整个4KB都可以写入读取
- [x] 使用VirtualAlloc分配一段[可读可写的内存](https://docs.microsoft.com/zh-cn/windows/win32/memory/memory-protection-constants)，写入内存，然后将这段内存改为只读，再读数据和写数据，看是否会有异常情况。然后VirtualFree这段内存，再测试对这段内存的读写释放正常

## 实验过程

### malloc

- 编写代码
  
    ```
    #include<stdio.h>
    #include<malloc.h>
    int main()
    {
        // sizeof获取了数据在内存中所占用的存储空间，以字节为单位来计数
        // win10 64bit C语言 sizeof(int) = 4B
        // 虚拟内存基本管理单元大小为4KB
        // 4B x 1K = 4KB
        // 2B x 1K = 2KB
        printf("int所占的字节数为sizeof(int) = %d\n", sizeof(int));
        int len1 = 1e2;
        int len2 = 1e3;
        int* a = (int*)malloc(sizeof(int) * len1);
        // 4KB以内
        for (int i = 0; i < len2; i++) {
            a[i] = i;
        }
        for (int i = 0; i < len2; i++) {
            printf("a[%d]=%d ", i, a[i]);
        }
        // 超过4KB
        for (int i = len2; i < len2 + 100; i++) {
            a[i] = i;
        }
        for (int i = len2; i < len2 + 100; i++) {
            printf("a[%d]=%d ", i, a[i]);
        }
        return 0;
    }
    ```

- Visual Studio在直接运行的情况下，在4KB内能够跑出结果，不会报错；在超过4KB一部分后，能够跑出结果，但会报错

    <img src="./img/malloc1.png">

- Visual Studio在调试的情况下，在4KB内能够跑出结果，不会报错；在超过4KB一部分后，抛出内存访问冲突的异常和缓冲区溢出的警告，但同时也能够跑出结果

    <img src="./img/malloc3.png">

### 虚拟内存

#### 使用VirtualAlloc分配一段可读可写的内存

- Memory Protection Constants
  
  | constant / value | explain | description | 
  | ---------------- | ------- | ----------- |
  | PAGE_READWRITE 0x04 | 可读可写 | Enables read-only or read/write access to the committed region of pages. If Data Execution Prevention is enabled, attempting to execute code in the committed region results in an access violation. |
  | PAGE_READONLY 0x02 | 只读 | Enables read-only access to the committed region of pages. An attempt to write to the committed region results in an access violation. If Data Execution Prevention is enabled, an attempt to execute code in the committed region results in an access violation. |

- VirtualAlloc中的Allocate reserved pages和Allocate commit pages的区别

  | Value | Meaning |
  | ----- | ------- |
  | MEM_COMMIT 0x00001000 | Allocates memory charges (from the overall size of memory and the paging files on disk) for the specified reserved memory pages. The function also guarantees that when the caller later initially accesses the memory, the contents will be zero. Actual physical pages are not allocated unless/until the virtual addresses are actually accessed. |
  | MEM_RESEVER 0x00002000 | Reserves a range of the process's virtual address space without allocating any actual physical storage in memory or in the paging file on disk. |
  
- 将代码中flProtect字段的PAGE_NOACCESS修改为PAGE_READWRITE，并且将flAllocationType字段修改为MEM_RESERVE|MEM_COMMIT(若仅仅是MEM_RESERVE，则尽管设置了PAGE_READWRITE，还是无法访问内存)
  ```
  lpvBase = VirtualAlloc(
		NULL,                 // System selects address
		PAGELIMIT * dwPageSize, // Size of allocation
		MEM_RESERVE | MEM_COMMIT,          // Allocate reserved pages
		PAGE_READWRITE);       // Protection = no access
  ```
- 读写正常
  
  <img src="./img/readsuccess.png">

  <img src="./img/writesuccess.png">

#### 完整代码
  
    ```
    // A short program to demonstrate dynamic memory allocation
    // using a structured exception handler.

    #include <windows.h>
    #include <tchar.h>
    #include <stdio.h>
    #include <stdlib.h>             // For exit

    #define PAGELIMIT 2            // Number of pages to ask for

    LPTSTR lpNxtPage;               // Address of the next page to ask for
    DWORD dwPages = 0;              // Count of pages gotten so far
    DWORD dwPageSize;               // Page size on this computer

    INT PageFaultExceptionFilter(DWORD dwCode)
    {
        LPVOID lpvResult;

        // If the exception is not a page fault, exit.

        if (dwCode != EXCEPTION_ACCESS_VIOLATION)
        {
            _tprintf(TEXT("Exception code = %d.\n"), dwCode);
            return EXCEPTION_EXECUTE_HANDLER;
        }

        _tprintf(TEXT("Exception is a page fault.\n"));

        // If the reserved pages are used up, exit.

        if (dwPages >= PAGELIMIT)
        {
            _tprintf(TEXT("Exception: out of pages.\n"));
            return EXCEPTION_EXECUTE_HANDLER;
        }

        // Otherwise, commit another page.

        lpvResult = VirtualAlloc(
            (LPVOID)lpNxtPage, // Next page to commit
            dwPageSize,         // Page size, in bytes
            MEM_COMMIT,         // Allocate a committed page
            PAGE_READWRITE);    // Read/write access
        if (lpvResult == NULL)
        {
            _tprintf(TEXT("VirtualAlloc failed.\n"));
            return EXCEPTION_EXECUTE_HANDLER;
        }
        else
        {
            _tprintf(TEXT("Allocating another page.\n"));
        }

        // Increment the page count, and advance lpNxtPage to the next page.

        dwPages++;
        lpNxtPage = (LPTSTR)((PCHAR)lpNxtPage + dwPageSize);

        // Continue execution where the page fault occurred.

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    VOID ErrorExit(LPTSTR lpMsg)
    {
        _tprintf(TEXT("Error! %s with error code of %ld.\n"),
            lpMsg, GetLastError());
        exit(0);
    }

    VOID _tmain(VOID)
    {
        LPVOID lpvBase;               // Base address of the test memory
        LPTSTR lpPtr;                 // Generic character pointer
        BOOL bSuccess;                // Flag
        DWORD i;                      // Generic counter
        SYSTEM_INFO sSysInfo;         // Useful information about the system

        GetSystemInfo(&sSysInfo);     // Initialize the structure.

        _tprintf(TEXT("This computer has page size %d.\n"), sSysInfo.dwPageSize);

        //dwPageSize = sSysInfo.dwPageSize;
        dwPageSize = 10;

        // Reserve pages in the virtual address space of the process.

        lpvBase = VirtualAlloc(
            NULL,                 // System selects address
            PAGELIMIT * dwPageSize, // Size of allocation
            MEM_RESERVE | MEM_COMMIT,          // Allocate reserved pages
            PAGE_READWRITE);       // Protection = no access
        if (lpvBase == NULL)
            //ErrorExit(TEXT("VirtualAlloc reserve failed."));
            ErrorExit(LPTSTR("VirtualAlloc reserve failed."));

        lpPtr = lpNxtPage = (LPTSTR)lpvBase;

        // Use structured exception handling when accessing the pages.
        // If a page fault occurs, the exception filter is executed to
        // commit another page from the reserved block of pages.

        for (i = 0; i < PAGELIMIT * dwPageSize; i++)
        {
            __try
            {
                // Write to memory.

                lpPtr[i] = 'a';
            }

            // If there's a page fault, commit another page and try again.

            __except (PageFaultExceptionFilter(GetExceptionCode()))
            {

                // This code is executed only if the filter function
                // is unsuccessful in committing the next page.

                _tprintf(TEXT("Exiting process.\n"));

                ExitProcess(GetLastError());

            }

        }
        
        for (i = 0; i < PAGELIMIT * dwPageSize; i++)
        {
            // Read from the memory
            printf("%d : %c\n", i, lpPtr[i]);
        }

        // Release the block of pages when you are finished using them.

        bSuccess = VirtualFree(
            lpvBase,       // Base address of block
            0,             // Bytes of committed pages
            MEM_RELEASE);  // Decommit the pages

        _tprintf(TEXT("Release %s.\n"), bSuccess ? TEXT("succeeded") : TEXT("failed"));

    }
    ```

#### 将上面的内存改为只读

- 需要用VirtualProtect进行修改
- 可以进行读，但是无法写入
  
  <img src="./img/readonly.png">

- 在调试的情况下引发写入访问权限冲突的异常

  <img src="./img/writeerror.png">

#### 完整代码

    ```
    // A short program to demonstrate dynamic memory allocation
    // using a structured exception handler.

    #include <windows.h>
    #include <tchar.h>
    #include <stdio.h>
    #include <stdlib.h>             // For exit

    #define PAGELIMIT 2            // Number of pages to ask for

    LPTSTR lpNxtPage;               // Address of the next page to ask for
    DWORD dwPages = 0;              // Count of pages gotten so far
    DWORD dwPageSize;               // Page size on this computer

    INT PageFaultExceptionFilter(DWORD dwCode)
    {
        LPVOID lpvResult;

        // If the exception is not a page fault, exit.

        if (dwCode != EXCEPTION_ACCESS_VIOLATION)
        {
            _tprintf(TEXT("Exception code = %d.\n"), dwCode);
            return EXCEPTION_EXECUTE_HANDLER;
        }

        _tprintf(TEXT("Exception is a page fault.\n"));

        // If the reserved pages are used up, exit.

        if (dwPages >= PAGELIMIT)
        {
            _tprintf(TEXT("Exception: out of pages.\n"));
            return EXCEPTION_EXECUTE_HANDLER;
        }

        // Otherwise, commit another page.

        lpvResult = VirtualAlloc(
            (LPVOID)lpNxtPage, // Next page to commit
            dwPageSize,         // Page size, in bytes
            MEM_COMMIT,         // Allocate a committed page
            PAGE_READWRITE);    // Read/write access
        if (lpvResult == NULL)
        {
            _tprintf(TEXT("VirtualAlloc failed.\n"));
            return EXCEPTION_EXECUTE_HANDLER;
        }
        else
        {
            _tprintf(TEXT("Allocating another page.\n"));
        }

        // Increment the page count, and advance lpNxtPage to the next page.

        dwPages++;
        lpNxtPage = (LPTSTR)((PCHAR)lpNxtPage + dwPageSize);

        // Continue execution where the page fault occurred.

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    VOID ErrorExit(LPTSTR lpMsg)
    {
        _tprintf(TEXT("Error! %s with error code of %ld.\n"),
            lpMsg, GetLastError());
        exit(0);
    }

    VOID _tmain(VOID)
    {
        LPVOID lpvBase;               // Base address of the test memory
        LPTSTR lpPtr;                 // Generic character pointer
        BOOL bSuccess;                // Flag
        DWORD i;                      // Generic counter
        SYSTEM_INFO sSysInfo;         // Useful information about the system

        GetSystemInfo(&sSysInfo);     // Initialize the structure.

        _tprintf(TEXT("This computer has page size %d.\n"), sSysInfo.dwPageSize);

        //dwPageSize = sSysInfo.dwPageSize;
        dwPageSize = 10;

        // Reserve pages in the virtual address space of the process.

        lpvBase = VirtualAlloc(
            NULL,                 // System selects address
            PAGELIMIT * dwPageSize, // Size of allocation
            MEM_RESERVE | MEM_COMMIT,          // Allocate reserved pages
            PAGE_READWRITE);       // Protection = no access
        if (lpvBase == NULL)
            //ErrorExit(TEXT("VirtualAlloc reserve failed."));
            ErrorExit(LPTSTR("VirtualAlloc reserve failed."));

        lpPtr = lpNxtPage = (LPTSTR)lpvBase;

        // Use structured exception handling when accessing the pages.
        // If a page fault occurs, the exception filter is executed to
        // commit another page from the reserved block of pages.

        // 具有可读可写的权限

        for (i = 0; i < PAGELIMIT * dwPageSize; i++)
        {
            __try
            {
                // Write to memory.

                lpPtr[i] = 'a';
            }

            // If there's a page fault, commit another page and try again.

            __except (PageFaultExceptionFilter(GetExceptionCode()))
            {

                // This code is executed only if the filter function
                // is unsuccessful in committing the next page.

                _tprintf(TEXT("Exiting process.\n"));

                ExitProcess(GetLastError());

            }

        }
        
        for (i = 0; i < PAGELIMIT * dwPageSize; i++)
        {
            // Read from the memory
            printf("%d : %c\n", i, lpPtr[i]);
        }

        // 将这段内存改为只读
        puts("change to read only");
        DWORD dwOldProtect = PAGE_READWRITE;
        BOOL change_to_readonly = VirtualProtect(
            lpvBase,                 
            PAGELIMIT * dwPageSize, 
            PAGE_READONLY,          
            &dwOldProtect);
        if (change_to_readonly == FALSE)
            //ErrorExit(TEXT("VirtualAlloc reserve failed."));
            ErrorExit(LPTSTR("Page change to read only failed."));

        puts("Read-Only mode: checking read...");
        for (i = 0; i < PAGELIMIT * dwPageSize; i++)
        {
            // Read from the memory
            printf("%d : %c\n", i, lpPtr[i]);
        }
        puts("Read-Only mode: checking write...");
        for (i = 0; i < PAGELIMIT * dwPageSize; i++)
        {
            __try
            {
                // Write to memory.

                lpPtr[i] = 'a';
            }

            // If there's a page fault, commit another page and try again.

            __except (PageFaultExceptionFilter(GetExceptionCode()))
            {

                // This code is executed only if the filter function
                // is unsuccessful in committing the next page.

                _tprintf(TEXT("Exiting process.\n"));

                ExitProcess(GetLastError());

            }
        }

        // Release the block of pages when you are finished using them.

        bSuccess = VirtualFree(
            lpvBase,       // Base address of block
            0,             // Bytes of committed pages
            MEM_RELEASE);  // Decommit the pages

        _tprintf(TEXT("Release %s.\n"), bSuccess ? TEXT("succeeded") : TEXT("failed"));

    }
    ```

#### VirtualFree该段内存

> If a page is released, its state changes to free, and it is available for subsequent allocation operations. After memory is released or decommited, you can never refer to the memory again. Any information that may have been in that memory is gone forever. Attempting to read from or write to a free page results in an access violation exception.

- 在代码后追加读写操作，都会引发异常

  <img src="./img/readerror.png">

  <img src="./img/writeafterfree.png">

    ```
    puts("After free: checking write...");
    for (i = 0; i < PAGELIMIT * dwPageSize; i++)
    {
        // Write to memory.

        lpPtr[i] = 'a';
    }

    puts("After free: checking read...");
    for (i = 0; i < PAGELIMIT * dwPageSize; i++)
    {
        // Read from the memory
        printf("%d : %c\n", i, lpPtr[i]);
    }
    ```

## 参考资料

- [VirtualAlloc MEM_COMMIT and MEM_RESERVE](https://stackoverflow.com/questions/26029374/virtualalloc-mem-commit-and-mem-reserve)
- [代码参见](./code/virtualalloc.cpp)