
#ifndef __NGX_CONF_H__
#define __NGX_CONF_H__

#include <vector>

#include "ngx_global.h"  

class CConfig
{

private:
	CConfig();
public:
	~CConfig();
private:
	static CConfig *m_instance;

public:	
	static CConfig* GetInstance() 
	{	
		if(m_instance == NULL)
		{
			//锁
			if(m_instance == NULL)
			{					
				m_instance = new CConfig();
				static CGarhuishou cl; 
			}
			//放锁		
		}
		return m_instance;
	}	
	class CGarhuishou  //类中套类，用于释放对象
	{
	public:				
		~CGarhuishou()
		{
			if (CConfig::m_instance)
			{						
				delete CConfig::m_instance;
				CConfig::m_instance = NULL;				
			}
		}
	};
//---------------------------------------------------
public:
	//装载配置文件
    bool Load(const char *pConfName); 

	////根据ItemName获取字符串类型配置信息
	const char *GetString(const char *pItemName);

	//根据ItemName获取数字类型配置信息
	int  GetIntDefault(const char *pItemName, const int nDef);

private:
	std::vector<LPCConfItem> m_ConfigItemList; //存储配置信息的列表

};

#endif
