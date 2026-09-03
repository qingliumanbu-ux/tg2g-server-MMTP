#include "CDynaTable.h"

BM2F_ENTERACE(gctp_configDelete)

BM2_FUNCTION_EXPORT
int f_gctp_configDelete(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CDbCommand cmd(conn);

	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			sqlstr = "DELETE FROM tgctp01 WHERE cfgitm_name = '" + bcls_rec->Tables[0].Rows[i]["CFGITM_NAME"].ToString() + "'";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tgctp02 WHERE cfgitm_name = '" + bcls_rec->Tables[0].Rows[i]["CFGITM_NAME"].ToString() + "'";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();
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