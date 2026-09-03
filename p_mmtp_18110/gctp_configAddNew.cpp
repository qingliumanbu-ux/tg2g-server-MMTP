#include "CDynaTable.h"

BM2F_ENTERACE(gctp_configAddNew)

BM2_FUNCTION_EXPORT
int f_gctp_configAddNew(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString cfgitmName = "";
	int iRowNo = 0;

	//动态数据表定义
	CDynaTable tgctp01("TGCTP01", conn);
	CDynaTable tgctp02("TGCTP02", conn);

	//数据库操作类定义
	CDbCommand cmd(conn);

	try
	{
		cfgitmName = GetColValueC(bcls_rec->Tables[0], 0, "CFGITM_NAME");
		if (cfgitmName.Trim() == "")
		{
			strcpy(s.msg, "没有传入配置号");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		for (int i = 0; i < bcls_rec->Tables.get_Count(); i++)
		{
			CString blkName = bcls_rec->Tables[i].get_TableName();
			if (blkName == "MAIN_ITEM")
			{
				sqlstr = "DELETE FROM tgctp02 WHERE cfgitm_name = '" + cfgitmName + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				tgctp02.CopyFrom(bcls_rec->Tables[blkName]);

				CDecimal seqNo = 0;
				CDecimal seqNoCondition = 0;
				for (int j = 0; j < tgctp02.GetRowCount(); j++)
				{
					//配置名
					tgctp02.SetColVal("CFGITM_NAME", cfgitmName, j);

					//字段唯一流水号
					tgctp02.SetColVal("SEQ_ID", GetTrackSeqNo("MMTP_VTABLE_ROWID", 18, conn), j);

					//字段总序号
					seqNo = seqNo + 1;
					tgctp02.SetColVal("SEQ_NO", seqNo, j);

					//条件字段
					if (tgctp02.GetColValString("DEFAULT_SHOW_STATE", j).Substring(0, 1) == "1")
					{
						seqNoCondition = seqNoCondition + 1;
						tgctp02.SetColVal("SEQ_NO_01", seqNoCondition, j);
					}
				}

				CDecimal seqNoGrid = 0;
				for (int j = 0; j < tgctp02.GetRowCount(); j++)
				{
					//多记录字段
					if (tgctp02.GetColValString("DEFAULT_SHOW_STATE", j).Substring(1, 1) == "1")
					{
						seqNoGrid = seqNoGrid + 1;
						tgctp02.SetColVal("SEQ_NO_02", seqNoGrid, j);
					}
				}

				CDecimal seqNoCtrl = 0;
				for (int j = 0; j < tgctp02.GetRowCount(); j++)
				{
					//单记录字段
					if (tgctp02.GetColValString("DEFAULT_SHOW_STATE", j).Substring(2, 1) == "1")
					{
						seqNoCtrl = seqNoCtrl + 1;
						tgctp02.SetColVal("SEQ_NO_03", seqNoCtrl, j);
					}
				}

				tgctp02.Print();
				if (tgctp02.Insert() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}
			else
			{
				sqlstr = "DELETE FROM tgctp01 WHERE cfgitm_name = '" + cfgitmName + "' AND cfgitm_grp_name = '" + blkName + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				for (int j = 0; j < bcls_rec->Tables[i].Rows.get_Count(); j++)
				{
					CString sRowId = GetTrackSeqNo("MMTP_VTABLE_ROWID", 20, conn);
					for (int k = 0; k < bcls_rec->Tables[i].Columns.get_Count(); k++)
					{
						CString colName = bcls_rec->Tables[i].Columns[k].get_ColumnName();
						CString colValue = bcls_rec->Tables[i].Rows[j][colName].ToString();

						tgctp01.SetColVal("CFGITM_NAME", cfgitmName, iRowNo);
						tgctp01.SetColVal("CFGITM_GRP_NAME", blkName, iRowNo);
						tgctp01.SetColVal("ITEM_ENAME", colName, iRowNo);
						tgctp01.SetColVal("NOW_ROW", sRowId, iRowNo);
						tgctp01.SetColVal("ITEM_CVALUE", colValue, iRowNo);
						iRowNo++;
					}
				}
			}
		}

		//新增数据
		if (tgctp01.Insert() < 0)
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