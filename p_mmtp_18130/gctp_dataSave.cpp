#include "CDynaTable2.h"

BM2F_ENTERACE(gctp_dataSave)

BM2_FUNCTION_EXPORT
int f_gctp_dataSave(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	try
	{
		if (bcls_rec->Tables.Contains("DATA_INS") && bcls_rec->Tables["DATA_INS"].Rows.get_Count() > 0)
		{
			doFlag = f_gctp_dataInsert(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		if (bcls_rec->Tables.Contains("DATA_UPD") && bcls_rec->Tables["DATA_UPD"].Rows.get_Count() > 0)
		{
			doFlag = f_gctp_dataUpdate(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		if (bcls_rec->Tables.Contains("DATA_DEL") && bcls_rec->Tables["DATA_DEL"].Rows.get_Count() > 0)
		{
			doFlag = f_gctp_dataDelete(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
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