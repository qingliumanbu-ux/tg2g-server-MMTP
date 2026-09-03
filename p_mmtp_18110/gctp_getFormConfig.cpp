/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2022
Author:      admin
Version:     1.0
Date:        2022-07-20 16:38:18
Description: 画面配置获取
**************************************************/

#include "CUtils.h"

BM2_FUNCTION_IMPORT
int f_gctp_buttonConfig(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn, CString formNo);

BM2_FUNCTION_IMPORT
int f_gctp_getConfigData(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);

BM2F_ENTERACE(gctp_getFormConfig)
int f_gctp_getFormConfig(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString formNo = "";
	CString queryFlag = "";

	//数据库操作类定义
	CDbCommand cmd_inq(conn);

	try
	{
		formNo = GetColValueC(bcls_rec->Tables[0], 0, "FORM_NO").Trim();
		PrintLog("formNo", formNo);

		//1: 导出时查询
		queryFlag = GetColValueC(bcls_rec->Tables[0], 0, "QUERY_FLAG").Trim();
		PrintLog("queryFlag", queryFlag);

		sqlstr = "SELECT * FROM tgctp04 WHERE form_no = '" + formNo + "' AND cfgitm_name > ' ' ORDER BY page_id,tree_level,location";
		PrintLog("sqlstr", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		if (queryFlag == "1")
		{
			bcls_ret->Tables[0].set_TableName("FORM");

			//查询画面配置数据
			doFlag = f_gctp_getConfigData(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		else
		{
			bcls_ret->Tables[0].set_TableName("FORM_CFGITM");
		}

		//查询按钮配置数据
		doFlag = f_gctp_buttonConfig(bcls_rec, bcls_ret, conn, formNo);
		if (doFlag < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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


