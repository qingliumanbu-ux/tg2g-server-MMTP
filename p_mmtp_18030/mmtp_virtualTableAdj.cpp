#include "CDynaTable.h"

BM2F_ENTERACE(mmtp_virtualTableAdj)
int f_mmtp_virtualTableAdj(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//变量定义
	int doFlag = 0;
	CString sqlstr = " ";

	//程序变量
	CString tableEname = "";
	CString tableCname = "";

	try
	{
		CTracer log(__FUNCTION__);

		tableEname = bcls_rec->Tables[0].Rows[0]["TABLE_NAME"].ToString().Trim().ToUpper();
		if (tableEname == "")
		{
			strcpy(s.msg, "请输入数据表名");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}
		
		tableCname = bcls_rec->Tables[0].Rows[0]["TABLE_CNAME"].ToString().Trim().ToUpper();
		if (tableCname == "")
		{
			strcpy(s.msg, "请输入数据表中文名");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		CDynaTable tmmtp06("TMMTP06", conn);
		tmmtp06.CopyFrom(bcls_rec->Tables[0]);

		tmmtp06.SetFilterColVal("TABLE_ENAME", tableEname);
		if (tmmtp06.Delete() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		for (int i = 0; i < tmmtp06.GetRowCount(); i++)
		{
			tmmtp06.SetColVal("TABLE_ENAME", tableEname, i);
			tmmtp06.SetColVal("TABLE_CNAME", tableCname, i);
			tmmtp06.SetColVal("SEQ_NO", (CDecimal)i, i);
			tmmtp06.SetColVal("ITEM_ENAME", bcls_rec->Tables[0].Rows[i]["ITEM_NAME"].ToString().ToUpper(), i);
		}

		//tmmtp06.Print();
		if (tmmtp06.Insert() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}
	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
		CMessageFormat::Format(s.msg, "Database Error，sqlcode=[{0},{1}]", arguments, 2);
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