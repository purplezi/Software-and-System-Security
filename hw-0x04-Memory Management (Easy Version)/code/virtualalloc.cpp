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

	// 检查读操作
	puts("Read-Only mode: checking read...");
	for (i = 0; i < PAGELIMIT * dwPageSize; i++)
	{
		// Read from the memory
		printf("%d : %c\n", i, lpPtr[i]);
	}
	// 检查写操作
	//puts("Read-Only mode: checking write...");
	//for (i = 0; i < PAGELIMIT * dwPageSize; i++)
	//{
	//	__try
	//	{
	//		// Write to memory.

	//		lpPtr[i] = 'a';
	//	}

	//	// If there's a page fault, commit another page and try again.

	//	__except (PageFaultExceptionFilter(GetExceptionCode()))
	//	{

	//		// This code is executed only if the filter function
	//		// is unsuccessful in committing the next page.

	//		_tprintf(TEXT("Exiting process.\n"));

	//		ExitProcess(GetLastError());

	//	}
	//}

	// Release the block of pages when you are finished using them.

	bSuccess = VirtualFree(
		lpvBase,       // Base address of block
		0,             // Bytes of committed pages
		MEM_RELEASE);  // Decommit the pages

	_tprintf(TEXT("Release %s.\n"), bSuccess ? TEXT("succeeded") : TEXT("failed"));

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
}