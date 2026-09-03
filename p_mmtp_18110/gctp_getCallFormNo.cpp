/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2022
Author:      admin
Version:     1.0
Date:        2022-07-20 16:54:33
Description: 调用画面名获取
**************************************************/

#include "CUtils2.h"

BM2F_ENTERACE(gctp_getCallFormNo)
int f_gctp_getCallFormNo(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString formNo = "";

	//数据库操作类定义
	CDbCommand cmd_inq(conn);

	try
	{
		formNo = GetColValueC(bcls_rec->Tables[0], 0, "NAME").Trim();
		PrintLog("formNo", formNo);

		bcls_ret->Tables[0].set_TableName("CALL_FORM_NO");
		sqlstr = "SELECT item_cvalue AS call_form_no FROM TGCTP05 WHERE form_no = '" + formNo +
			"' AND item_ename = 'CALL_FORM_NO' AND SUBSTR(item_cvalue,1,3) <> 'EPR' ORDER BY now_row";
		PrintLog("sqlstr", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["CALL_FORM_NO"]);
		cmd_inq.Close();
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


