#include "CDynaTable.h"

BM2F_ENTERACE(gctp_formDelete)

BM2_FUNCTION_EXPORT
int f_gctp_formDelete(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CDbCommand cmd(conn);

	try
	{
		CTransactionManager::Commit(0);

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			sqlstr = "DELETE FROM tgctp04 WHERE form_no = @FORM_NO";
			cmd.Parameters.Set("FORM_NO", bcls_rec->Tables[0].Rows[i]["FORM_NO"].ToString());
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tgctp05 WHERE form_no = @FORM_NO";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tgctp06 WHERE form_no = @FORM_NO";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tgctp07 WHERE form_no = @FORM_NO";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tesbuttonresinfo_res@nnaum_dblink WHERE aclid IN"
				" (SELECT aclid FROM tesbuttonresinfo@nnaum_dblink WHERE fname = @FORM_NO)";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tesbuttonresinfo@nnaum_dblink WHERE fname = @FORM_NO";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tesformresinfo_res@nnaum_dblink WHERE aclid IN"
				" (SELECT aclid FROM tesformresinfo@nnaum_dblink WHERE name = @FORM_NO)";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tesformresinfo@nnaum_dblink WHERE name = @FORM_NO";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tesformpara@nnaum_dblink WHERE form_name = @FORM_NO";
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

	if (doFlag == 0)
	{
		CTransactionManager::Commit(0);
		CTransactionManager::Begin(0, 0);
	}
	else
	{
		CTransactionManager::Abort(0);
		CTransactionManager::Begin(0, 0);
	}

	return doFlag;
}