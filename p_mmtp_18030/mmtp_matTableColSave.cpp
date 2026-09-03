#include "CDynaTable.h"

BM2F_ENTERACE(mmtp_matTableColSave)
int f_mmtp_matTableColSave(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//变量定义
	int doFlag = 0;
	CString sqlstr = " ";

	//程序变量
	CString matKind = "";
	CString tableEname = "";
	CDataTable dtColumn;

	//数据库操作类定义
	CDbCommand cmd(conn);

	try
	{
		CTracer log(__FUNCTION__);

		CDynaTable tmmtp06("TMMTP06", conn);
		CDynaTable tmmtp04("TMMTP04", conn);
		CDynaTable tmmtp04_old("TMMTP04", conn);

		tableEname = GetColValueC(bcls_rec->Tables[0], 0, "TABLE_NAME");
		if (tableEname.Trim() != "")
		{
			tmmtp06.SetFilterColVal("TABLE_ENAME", tableEname);
			if (tmmtp06.Delete() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			dtColumn = GetTableColName1(tableEname, conn);
			tmmtp06.CopyFrom(dtColumn);
			tmmtp06.Print();
			if (tmmtp06.Insert() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}
		else
		{
			matKind = GetColValueC(bcls_rec->Tables[0], 0, "MAT_KIND").Trim().ToUpper();
			if (matKind == "")
			{
				strcpy(s.msg, "请输入物料类型");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			
			tableEname = "TMM" + matKind + "01";
			Log::Info("", __FUNCTION__, "物料主档表结构生成[{0}]", tableEname);
			tmmtp06.SetFilterColVal("TABLE_ENAME", tableEname);
			if (tmmtp06.Delete() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			dtColumn = GetTableColName1(tableEname, conn);
			tmmtp06.CopyFrom(dtColumn);
			if (tmmtp06.Insert() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (matKind == "HP")
			{
				tableEname = "TMMHP02";
				tmmtp06.SetFilterColVal("TABLE_ENAME", tableEname);
				if (tmmtp06.Delete() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				dtColumn = GetTableColName1(tableEname, conn);
				tmmtp06.CopyFrom(dtColumn);
				if (tmmtp06.Insert() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}

			tableEname = "TMM" + matKind + "96";
			Log::Info("", __FUNCTION__, "物料履历表结构生成[{0}]", tableEname);
			tmmtp06.SetFilterColVal("TABLE_ENAME", tableEname);
			if (tmmtp06.Delete() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			dtColumn = GetTableColName1(tableEname, conn);
			tmmtp06.CopyFrom(dtColumn);
			if (tmmtp06.Insert() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			Log::Info("", __FUNCTION__, "条件结构生成[{0}]", tableEname);
			sqlstr = "SELECT DISTINCT cfgitm_name FROM tmmtp04 WHERE table_ename = '" + tableEname + "'";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteReader();
			while (cmd.Read())
			{
				PrintLog("CFGITM_NAME", cmd.GetString(1));

				tmmtp04_old.SetFilterColVal("CFGITM_NAME", cmd.GetString(1));
				tmmtp04_old.SetFilterColVal("TABLE_ENAME", tableEname);
				if (tmmtp04_old.Query() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				if (tmmtp04_old.Delete() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				tmmtp04.CopyFrom(dtColumn);
				for (int i = 0; i < tmmtp04.GetRowCount(); i++)
				{
					tmmtp04.CopyColVal("CFGITM_NAME", tmmtp04_old.GetDataRow(), i);
					tmmtp04.CopyColVal("CFGITM_DESC", tmmtp04_old.GetDataRow(), i);
					for (int j = 0; j < tmmtp04_old.GetRowCount(); j++)
					{
						if (tmmtp04.GetColValString("ITEM_ENAME", i) == tmmtp04_old.GetColValString("ITEM_ENAME", j))
						{
							tmmtp04.MergeFrom(tmmtp04_old.GetDataRow(j), i);
							tmmtp04.SetColVal("SEQ_NO", (CDecimal)i + 1, i);
						}
					}

					if (tmmtp04.GetColValString("ORIGIN_CODE", i).Trim() == "")
					{
						tmmtp04.SetColVal("ORIGIN_CODE", "0", i);
					}
				}

				if (tmmtp04.Insert() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}
			cmd.Close();
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