#include "CDynaTable.h"

BM2F_ENTERACE(gctp_configDataClear)

BM2_FUNCTION_EXPORT
int f_gctp_configDataClear(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString formNo = "";
	CString cfgitmName = "";

	//数据库操作类定义
	CDbCommand cmd(conn);

	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			formNo = GetColValueC(bcls_rec->Tables[0], i, "FORM_NO").Trim();
			cfgitmName = GetColValueC(bcls_rec->Tables[0], i, "CFGITM_NAME").Trim();

			if (formNo != "")
			{
				sqlstr = "DELETE FROM tgctp04 WHERE form_no = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp05 WHERE form_no = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp06 WHERE form_no = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp07 WHERE form_no = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);
			}
			else if (cfgitmName != "")
			{
				sqlstr = "DELETE FROM tgctp01 WHERE cfgitm_name = '" + cfgitmName + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp02 WHERE cfgitm_name = '" + cfgitmName + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp03 WHERE cfgitm_name = '" + cfgitmName + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);
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