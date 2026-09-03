/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2018
Author:      admin
Version:     1.0
Date:        2018-06-06 15:41:35
Description: 数据删除函数
**************************************************/

#include "CDynaTable.h"

BM2_FUNCTION_EXPORT
int f_mmtp_data_delete(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CString config_name, CString cfggrp_name, CString operation_type)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString sTableName = "";

	try
	{
		if (!bcls_rec->Tables.Contains("DATA_DEL"))
		{
			strcpy(s.msg, "没有传入条件数据块");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		CDynaTable tmmtp03("TMMTP03", conn);
		if (bcls_rec->Tables.Contains("TMMTP03") && bcls_rec->Tables["TMMTP03"].Rows.get_Count() > 0)
		{
			tmmtp03.CopyFrom(bcls_rec->Tables["TMMTP03"]);
		}
		else if (config_name.Trim() != "")
		{
			tmmtp03.SetFilterColVal("CFGITM_NAME", config_name.Trim());
			tmmtp03.SetFilterColVal("CFGGRP_NAME", cfggrp_name.Trim());
			tmmtp03.Query();
		}

		if (tmmtp03.GetRowCount() == 0)
		{
			strcpy(s.msg, "没有配置数据");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sTableName = tmmtp03.GetColValString("TABLE_NAME");
		if (sTableName.Trim() == "")
		{
			strcpy(s.msg, "没有配置数据表名");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}
		PrintLog("sTableName", sTableName);

		CDynaTable tmmtp02("TMMTP02", conn);
		if (bcls_rec->Tables.Contains("TMMTP02") && bcls_rec->Tables["TMMTP02"].Rows.get_Count() > 0)
		{
			tmmtp02.CopyFrom(bcls_rec->Tables["TMMTP02"]);
		}
		else if (config_name.Trim() != "")
		{
			tmmtp02.SetFilterColVal("CFGITM_NAME", config_name.Trim());
			tmmtp02.SetFilterColVal("CFGGRP_NAME", cfggrp_name.Trim());
			tmmtp02.Query();
		}

		CDynaTable tmmtp05("TMMTP05", conn);
		if (bcls_rec->Tables.Contains("TMMTP05") && bcls_rec->Tables["TMMTP05"].Rows.get_Count() > 0)
		{
			tmmtp05.CopyFrom(bcls_rec->Tables["TMMTP05"]);
		}
		else if (config_name.Trim() != "")
		{
			if (operation_type.Trim() == "")
			{
				operation_type = "F5";
			}

			tmmtp05.SetFilterColVal("CFGITM_NAME", config_name.Trim());
			tmmtp05.SetFilterColVal("CFGGRP_NAME", cfggrp_name.Trim());
			tmmtp05.SetFilterColVal("OPERATION_TYPE", operation_type.Trim());
			tmmtp05.Query();
		}

		//定义删除数据表
		CDynaTable dynaTable(sTableName, conn);

		//获取前台传入修改数据
		dynaTable.CopyFrom(bcls_rec->Tables[0]);

		if (tmmtp03.GetColValString("TABLE_TYPE") == "1")
		{
			if (tmmtp03.GetColValString("TABLE_ENAME").Trim() == "")
			{
				strcpy(s.msg, "没有配置虚拟数据表名");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			dynaTable.AddFilterColName("TABLE_ENAME");
			dynaTable.AddFilterColName("NOW_ROW");
			dynaTable.SetColValAllRow("TABLE_ENAME", tmmtp03.GetColValString("TABLE_ENAME"));
		}
		else
		{
			//添加条件列和修改列
			for (int i = 0; i < tmmtp02.GetRowCount(); i++)
			{
				if (tmmtp02.GetColValString("KEYWORD_FLAG", i) == "1")
				{
					dynaTable.AddFilterColName(tmmtp02.GetColValString("COLUMN_NAME", i));
				}
			}
		}

		//删除数据
		if (dynaTable.DeleteAll() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		for (int i = 0; i < tmmtp05.GetRowCount(); i++)
		{
			if (tmmtp05.GetColValString("FUNC_NAME", i).Trim() != "")
			{
				//设置EDCALL参数
				CString funcName = tmmtp05.GetColValString("FUNC_NAME", i).Trim();
				CString pkName = tmmtp05.GetColValString("KEY_1", i).Trim();
				CString pkValue = tmmtp05.GetColValString("KEYVALUE_1", i).Trim();

				strcpy(e.func_name[0], funcName);
				strcpy(e.pk_name[0], pkName);
				strcpy(e.pk_val[0], pkValue);

				//将ED结构压入 EIClass func_rec 中
				bcls_rec->SetED(e);

				doFlag = f_epedcall(bcls_rec, bcls_ret);
				if (doFlag != 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
		}
	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}