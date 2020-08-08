
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ngx_c_memory.h"

CMemory* CMemory::m_instance = NULL;

void *CMemory::AllocMemory(int memCount,bool ifmemset)
{	    
	void *tmpData = (void *)new char[memCount]; //我并不会判断new是否成功，如果new失败，程序根本不应该继续运行，就让它崩溃以方便我们排错吧
    if(ifmemset) //要求内存清0
    {
	    memset(tmpData,0,memCount);
    }
	return tmpData;
}

void CMemory::FreeMemory(void *point)
{		
    delete [] ((char *)point); 
}

