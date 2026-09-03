#include "CUtils2.h"

BM2F_ENTERACE(gctp_configItemSave)

BM2_FUNCTION_EXPORT
int f_gctp_configItemSave(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString cfgitmGrpName = GetColValueC(bcls_rec->Tables[0], 0, "CFGITM_GRP_NAME");
	CDbCommand cmd(conn);

	try
	{
		if (cfgitmGrpName.Trim() != "")
		{
			sqlstr = "DELETE FROM tgctp00 WHERE cfgitm_grp_name = '" + cfgitmGrpName + "'";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
			{
				SetColValue(bcls_rec->Tables[0], i, "CFGITM_GRP_NAME", cfgitmGrpName);
				AddColValue(bcls_rec->Tables[0], i, "SEQ_NO", (CDecimal)i);

				if (GetColValueC(bcls_rec->Tables[0], i, "DATA_TYPE").Trim() == "")
				{
					AddColValue(bcls_rec->Tables[0], i, "DATA_TYPE", "C");
				}
			}

			if (!TableDataInsert("TGCTP00", bcls_rec->Tables[0], conn))
			{
				throw CApplicationException(-1, s.msg, log.Location);
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