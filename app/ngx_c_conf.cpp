
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "ngx_func.h"   
#include "ngx_c_conf.h"  

CConfig *CConfig::m_instance = NULL;

CConfig::CConfig()
{		
}

CConfig::~CConfig()
{    
	for(std::vector<LPCConfItem>::iterator iter = m_ConfigItemList.begin(); 
    iter != m_ConfigItemList.end(); ++iter)
	{		
		delete (*iter);
	}
	m_ConfigItemList.clear(); 
}

bool CConfig::Load(const char *pConfName) 
{   
   //文本文件
   FILE *pf = fopen(pConfName, "r");
   if (NULL == pf)
        return false;

    char lineBuf[501];
    while(!feof(pf))
    {
        //按行处理
        if (NULL == fgets(lineBuf, 500, pf))
            continue;

        if (0 == lineBuf[0] || '[' == lineBuf[0])            
            continue;
        
        if (';' == *lineBuf ||
            ' ' == *lineBuf ||
            '#' == *lineBuf ||
            '\t' == *lineBuf ||
            '\n' == *lineBuf)
            continue;

    stringproc:
        //屁股后边若有换行，回车，空格等都截取掉
        if (strlen(lineBuf) > 0)
        {
            char cLast = lineBuf[strlen(lineBuf) - 1];
            if ('\r' == cLast || '\n' == cLast || ' ' == cLast)
                {
                    lineBuf[strlen(lineBuf) - 1] = 0;
                    goto stringproc;
                }
          
        }
        
        if (0 == lineBuf[0])
            continue;

        //strchr 查找子字符串,返回一个指向该字符串中第一次出现的字符的指针，
        //如果字符串中不包含该字符则返回NULL空指针     
        char *pTemp = strchr(lineBuf, '=');
        if (pTemp != NULL)
        {
            LPCConfItem pCConfItem = new CConfItem;
            memset(pCConfItem, 0, sizeof(CConfItem));

            //表示把src所指向的字符串中以src地址开始的前n个字节复制到dest所指的数组中
            //，并返回被复制后的dest
            //=左边
            strncpy(pCConfItem->ItemName, lineBuf, (int)(pTemp - lineBuf));

            //右边
            strcpy(pCConfItem->ItemContent, pTemp + 1);

            //(pCConfItem->ItemName);
            Rtrim(pCConfItem->ItemName);
            Ltrim(pCConfItem->ItemContent);
            //Rtrim(pCConfItem->ItemContent);

            m_ConfigItemList.push_back(pCConfItem);
        }
    }


    fclose(pf);
    return true;
}

const char *CConfig::GetString(const char *pItemName)
{
	for(std::vector<LPCConfItem>::iterator iter = m_ConfigItemList.begin();
        iter != m_ConfigItemList.end(); ++iter)
	{	
		if(strcasecmp((*iter)->ItemName, pItemName) == 0)
			return (*iter)->ItemContent;
	}

	return NULL;
}

int CConfig::GetIntDefault(const char *pItemName, const int nDef)
{
	for(std::vector<LPCConfItem>::iterator iter = m_ConfigItemList.begin(); 
        iter !=m_ConfigItemList.end(); ++iter)
	{	
		if(strcasecmp( (*iter)->ItemName, pItemName) == 0)
			return atoi((*iter)->ItemContent);
	}

	return nDef;
}



