#include "CUtils.h"

BM2F_ENTERACE(gctp_configLoad)

BM2_FUNCTION_IMPORT
int f_gctp_getConfigData(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);

BM2_FUNCTION_IMPORT
int f_gctp_buttonConfig(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn, CString formNo);

BM2_FUNCTION_EXPORT
int f_gctp_configLoad(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
		formNo = GetColValueC(bcls_rec->Tables[0], 0, "FORM_NO").Trim();
		PrintLog("formNo", formNo);

		bcls_ret->Tables[0].set_TableName("FORM");
		sqlstr = "SELECT * FROM tgctp04 WHERE form_no = '" + formNo + "' ORDER BY page_id,tree_level,location";
		PrintLog("sqlstr", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["FORM"]);
		cmd_inq.Close();

		bcls_ret->Tables.Add("LOGIN");
		bcls_ret->Tables["LOGIN"].Columns.Add(DT_STRING, "USER_ID");
		bcls_ret->Tables["LOGIN"].Columns.Add(DT_STRING, "USER_NAME");
		bcls_ret->Tables["LOGIN"].Columns.Add(DT_STRING, "COMPUTER_NAME");
		bcls_ret->Tables["LOGIN"].Columns.Add(DT_STRING, "IP_ADDRESS");

		bcls_ret->Tables["LOGIN"].Rows.Add();
		bcls_ret->Tables["LOGIN"].Rows[0]["USER_ID"] = s.userid;
		bcls_ret->Tables["LOGIN"].Rows[0]["USER_NAME"] = s.username;
		bcls_ret->Tables["LOGIN"].Rows[0]["COMPUTER_NAME"] = s.fore_machine;
		bcls_ret->Tables["LOGIN"].Rows[0]["IP_ADDRESS"] = s.fore_ip;

		//查询配置数据
		doFlag = f_gctp_getConfigData(bcls_rec, bcls_ret, conn);
		if (doFlag < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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