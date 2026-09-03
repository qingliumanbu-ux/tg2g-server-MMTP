#include "CDynaTable.h"

BM2F_ENTERACE(gctp_configTemplate)

BM2_FUNCTION_EXPORT
int f_gctp_configTemplate(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CDynaTable tgctp02("TGCTP02", conn);
	CDbCommand cmd(conn);

	try
	{
		tgctp02.CopyFrom(bcls_rec->Tables[0]);
		tgctp02.AddFilterColName("CFGITM_NAME");
		tgctp02.AddFilterColName("PAGE_ID");
		if (tgctp02.Delete() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		tgctp02.SetColVal("AREA_NO", "0", 0);
		if (tgctp02.GetColValString("TEMPLATE_TYPE") == "0")
		{
			tgctp02.SetColVal("AREA_NAME", "主区域", 0);
		}
		else if (tgctp02.GetColValString("TEMPLATE_TYPE") == "1" || tgctp02.GetColValString("TEMPLATE_TYPE") == "2")
		{
			tgctp02.SetColVal("AREA_NAME", "上区域", 0);
		}
		else if (tgctp02.GetColValString("TEMPLATE_TYPE") == "4" || tgctp02.GetColValString("TEMPLATE_TYPE") == "6")
		{
			tgctp02.SetColVal("AREA_NAME", "左区域", 0);
		}
		else if (tgctp02.GetColValString("TEMPLATE_TYPE") == "3" || tgctp02.GetColValString("TEMPLATE_TYPE") == "5")
		{
			tgctp02.SetColVal("AREA_NAME", "左上区域", 0);
		}

		if (tgctp02.GetColValString("TEMPLATE_TYPE") != "0")
		{
			tgctp02.SetColVal("AREA_NO", "1", 1);
		}

		if (tgctp02.GetColValString("TEMPLATE_TYPE") == "1")
		{
			tgctp02.SetColVal("AREA_NAME", "下区域", 1);
		}
		else if (tgctp02.GetColValString("TEMPLATE_TYPE") == "2" || tgctp02.GetColValString("TEMPLATE_TYPE") == "5")
		{
			tgctp02.SetColVal("AREA_NAME", "左下区域", 1);
		}
		else if (tgctp02.GetColValString("TEMPLATE_TYPE") == "4")
		{
			tgctp02.SetColVal("AREA_NAME", "右区域", 1);
		}
		else if (tgctp02.GetColValString("TEMPLATE_TYPE") == "6" || tgctp02.GetColValString("TEMPLATE_TYPE") == "3")
		{
			tgctp02.SetColVal("AREA_NAME", "右上区域", 1);
		}

		if (tgctp02.GetColValString("TEMPLATE_TYPE") != "0" && tgctp02.GetColValString("TEMPLATE_TYPE") != "1" && tgctp02.GetColValString("TEMPLATE_TYPE") != "4")
		{
			tgctp02.SetColVal("AREA_NO", "2", 2);
		}

		if (tgctp02.GetColValString("TEMPLATE_TYPE") == "2" || tgctp02.GetColValString("TEMPLATE_TYPE") == "6")
		{
			tgctp02.SetColVal("AREA_NAME", "右下区域", 2);
		}
		else if (tgctp02.GetColValString("TEMPLATE_TYPE") == "3")
		{
			tgctp02.SetColVal("AREA_NAME", "下区域", 2);
		}
		else if (tgctp02.GetColValString("TEMPLATE_TYPE") == "5")
		{
			tgctp02.SetColVal("AREA_NAME", "右区域", 2);
		}

		tgctp02.SetColValAllRow("TEMPLATE_TYPE", tgctp02.GetColValString("TEMPLATE_TYPE"));
		tgctp02.SetColValAllRow("CFGITM_NAME", tgctp02.GetColValString("CFGITM_NAME"));
		tgctp02.SetColValAllRow("PAGE_ID", tgctp02.GetColValString("PAGE_ID"));

		//新增数据
		if (tgctp02.Insert() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
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