# 内存管理

## Requirements

1. 验证不同进程的相同的地址可以保存不同的数据。
   (1) 在VS中，设置固定基地址，编写两个不同可执行文件。同时运行这两个文件。然后使用调试器附加到两个程序的进程，查看内存，看两个程序是否使用了相同的内存地址；
   (2) 在不同的进程中，尝试使用VirtualAlloc分配一块相同地址的内存，写入不同的数据。再读出。
2. (难度较高)配置一个Windbg双机内核调试环境，查阅Windbg的文档，了解
   (1) Windbg如何在内核调试情况下看物理内存，也就是通过物理地址访问内存
   (2) 如何查看进程的虚拟内存分页表，在分页表中找到物理内存和虚拟内存的对应关系。然后通过Windbg的物理内存查看方式和虚拟内存的查看方式，看同一块物理内存中的数据情况

## Experiment

### 虚拟内存

在一般计算机中 一个物理地址是唯一的，同一个物理地址下的数据是相同的

在虚拟内存管理当中，每个进程的地址是独立的，实际在物理地址中是两个不同的存储空间，但是虚拟的内存地址是相同的，不同进程，相同内存地址，在内核部分映射成为不同的物理地址，这就是为什么进程能在计算机上相互隔离运行的原因

### 实验过程

#### 查看内存地址

- 新建两个vs项目，写入不同的代码
  
  <img src="./img/twoprocess.png">

- 设置固定基址
  
  <img src="./img/fix-addr.png">

- 下断点调试，打开内存窗口 (vs导航栏：调试 > 窗口 > 内存)
  
  <img src="./img/memory.png">

  <img src="./img/memory2.png">

  - 两个程序使用了相同的内存地址
  - 同时运行却能使用相同的地址，是因为使用的是虚拟地址，映射于不同的物理地址

#### 使用VirtualAlloc分配一块相同地址的内存，写入不同的数据再读出

##### 代码

```c
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>             // For exit

int main() {
	printf("Demo A virtualalloc\n");

	LPVOID lpvBase;               // Base address of the test memory
	LPTSTR lpPtr;                 // Generic character pointer
	BOOL bSuccess;                // Flag
	DWORD i;                      // Generic counter
	SYSTEM_INFO sSysInfo;         // Useful information about the system

	GetSystemInfo(&sSysInfo);     // Initialize the structure.

	DWORD dwPageSize = sSysInfo.dwPageSize;
	// dwPageSize = 4096
	printf("%d\n", dwPageSize);

	// Reserve pages in the virtual address space of the process.
	int PAGELIMIT = 1;

	lpvBase = VirtualAlloc(
		(LPVOID)0x40000000,                 // System selects address
		PAGELIMIT*dwPageSize, // Size of allocation
		MEM_RESERVE | MEM_COMMIT,          // Allocate reserved pages
		PAGE_READWRITE);       // Protection = no access
	if (lpvBase == NULL)
	{
		_tprintf(TEXT("Error! %s with error code of %ld.\n"), TEXT("VirtualAlloc reserve failed."), GetLastError());
		exit(0);
	}

	lpPtr = (LPTSTR)lpvBase;

	// Write to memory.
	for (i = 0; i < PAGELIMIT*dwPageSize; i++) {
		lpPtr[i] = 'a';
	}

	// Read from memory
	for (i = 0; i < PAGELIMIT*dwPageSize; i++) {
		printf("%c", lpPtr[i]);
	}

	bSuccess = VirtualFree(
		lpvBase,       // Base address of block
		0,             // Bytes of committed pages
		MEM_RELEASE);  // Decommit the pages

	_tprintf(TEXT("\nRelease %s.\n"), bSuccess ? TEXT("succeeded") : TEXT("failed"));

	return 0;
}
```

##### 测试

- 调试A_demo，观察字符a写入过程
- `61h`是a的ascii值的16进制
- `00411929  mov         byte ptr [eax],61h  
` 代码执行完这句后，去查看eax寄存器的值，存的是a所在的地址

    <img src="./img/finda.png">

    <img src="./img/finda2.png">

    <img src="./img/memory3.png">

- 此时调试B_demo，b也可以写入

    <img src="./img/memoryb1.png">

- 相同内存地址存入了不同的数据，证明不同进程相同内存地址可以保存不同的数据

    <img src="./img/memoryab.png">

### Windbg双机内核调试

#### 思路

- 由于我们直接调试的操作系统内核，所以需要两台计算机安装两个Windows，然后连个计算机使用串口进行连接
- 所以我们需要再虚拟机中安装一个Windows（安装镜像自己找，XP就可以），然后通过虚拟串口和host pipe连接的方式，让被调试系统和windbg链接，windbg可以调试
- 使用Windbg  内核调试 VirtualBox 关键字搜索，能找到很多教程
- 如果决定Windows虚拟机太重量级了，可以用Linux虚拟机+gdb也能进行相关的实验，以gdb 远程内核调试 为关键字搜索，也能找到很多教程

#### 环境

- 主机：windows 10 企业版 1909 64位
- 虚拟机：windows 7 专业版 sp1_vl_build_x64
- Windows Software Development Kit - Windows 10.0.19041

#### 环境搭建

将主机称为Host端，windbg运行在主机上，通过虚拟串口连接虚拟机。将虚拟机称为Guest端，运行着待调试的系统

##### Guest端配置

- 为了让Windos支持内核调试，需要配置启动参数。首先需要对虚拟机配置虚拟串口，目的是为了建立host到guest的调试通信连接。如下图所示，选择com1并且映射成为\\.pip\com_1
  
  <img src="./img/com-config.png">

  - 注意不要选择 连接至现有通道或套接字 因为目前还没有建立管道。否则启动虚拟机时会报错误`不能为虚拟电脑打开一个新任务`
- 启动虚拟机，进入Window内部进行配置。以管理员身份启动CMD，输入以下命令
  
  <img src="./img/bcdedit.png">

  ```cmd
  # 设置端口1
  bcdedit /dbgsettings serial baudrate:115200 debugport:1
  
  # 复制一个开机选项，命名为“DebugEntry”，可任意命名
  
  # 增加一个开机引导项
  bcdedit /copy {current} /d DebugEntry

  # 激活debug
  bcdedit /displayorder {current} {替换第二个命令显示的UUID}
  bcdedit /debug {替换第二个命令显示的UUID} on
  ```

  [BCDEdit](https://docs.microsoft.com/en-us/windows-hardware/manufacture/desktop/bcdedit-command-line-options)用于管理启动配置数据（BCD）的命令行工具
- 重新启动系统，在开机的时候选择DebugEntry[启用调试程序]，能够正常启动系统
  
  <img src="./img/debugentry.png">

##### Host端配置

- [安装windbg](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)
  - [下载链接](https://developer.microsoft.com/zh-cn/windows/downloads/windows-10-sdk/)
  - Debugging Tools for Windows (WinDbg, KD, CDB, NTSD)
    
    <img src="./img/debugtools.png">

  - 找到windbg的路径为：`xxx\Windows Kits\10\Debuggers\x64`

- 在host端首先需要安装windbg，使用以下命令启动windbg
  - **注意启动时机是有要求的：先启动guest上的win7系统，后启动windbg**

    ```cmd
    # 以管理员的身份打开cmd命令行
    windbg.exe -k com:port=\\.\pipe\com_1,baud=115200,pipe
    ```

    <img src="./img/connect.png">

    - 时间过久，可以断住此时待调试的windows：在windbg的上方状态栏Debug > Break (此时虚拟机是不可以使用的)

      <img src="./img/connectsuccess.png">  

    - 提示如果要让虚拟机继续，则输入`g`，虚拟机又可以使用了
    - 上面的报错是因为符号文件还没有加载

- 配置windbg的符号下载地址：File > Symbol File Path 
  - 在`E:\WinSym`目录下存储信息，勾选reload
    
    ```
    srv*E:\WinSym*https://msdl.microsoft.com/download/symbols
    ```
    
    <img src="./img/symbolsearchpath.png">

  - 在`.reload`之后，一直出现如下报错

    <img src="./img/symbolreloaderror.png">

    - 解决方法：
        ```
        # 查看调试符号路径
        .sympath
        # 开启符号加载噪音模
        !sym noisy
        # 看看具体是怎么解析加载符号
        .reload /f ntkrnlmp
        ```

#### Windbg如何在内核调试情况下看物理内存，也就是通过物理地址访问内存

##### 查看物理内存

```
!address： 显示内存信息，比如内存的地址范围
```

##### 通过物理地址访问内存

#### 如何查看进程的虚拟内存分页表，在分页表中找到物理内存和虚拟内存的对应关系。然后通过Windbg的物理内存查看方式和虚拟内存的查看方式，看同一块物理内存中的数据情况

##### 查看进程的虚拟内存分页表

##### 物理内存和虚拟内存的对应关系

##### 然后通过Windbg的物理内存查看方式和虚拟内存的查看方式，看同一块物理内存中的数据情况

#### 内核调试案例



## Conclusion

### 虚拟内存

## References

- https://www.cnblogs.com/ck1020/p/6148399.html
- https://blog.csdn.net/lixiangminghate/article/details/54667694
- https://blog.csdn.net/weixin_42486644/article/details/80747462
- https://www.jianshu.com/p/09fab7c07533
- https://zhuanlan.zhihu.com/p/47771088
- https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/-pte
- https://reverseengineering.stackexchange.com/questions/21031/windbg-what-is-the-relation-between-the-vad-vad-the-ptes-pte-and-loade
- https://stackoverflow.com/questions/16749764/when-kernel-debugging-find-the-page-protection-of-a-user-mode-address