/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2018
Author:      178773
Version:     1.0
Date:        2018-06-05 10:16:50
Description: 数据新增函数
**************************************************/

#include "CDynaTable.h"

BM2_FUNCTION_EXPORT
int f_mmtp_data_insert(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CString config_name, CString cfggrp_name, CString operation_type)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	int iInsFlag = 0;
	CString sTableName = "";

	try
	{
		if (!bcls_rec->Tables.Contains("DATA_INS"))
		{
			strcpy(s.msg, "没有传入条件数据块");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		CDynaTable tmmtp03("TMMTP03", conn);
		if (bcls_rec->Tables.Contains("TMMTP03") && bcls_rec->Tables["TMMTP03"].Rows.get_Count() > 0)
		{
			tmmtp03.CopyFrom(bcls_rec->Tables["TMMTP03"]);
		}
		else if (config_name.Trim() != "")
		{
			tmmtp03.SetFilterColVal("CFGITM_NAME", config_name.Trim());
			tmmtp03.SetFilterColVal("CFGGRP_NAME", cfggrp_name.Trim());
			tmmtp03.Query();
			tmmtp03.CopyTo(bcls_rec->Tables["TMMTP03"]);
		}

		if (tmmtp03.GetRowCount() == 0)
		{
			strcpy(s.msg, "没有配置数据");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sTableName = tmmtp03.GetColValString("TABLE_NAME");
		if (sTableName.Trim() == "")
		{
			strcpy(s.msg, "没有配置数据表名");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}
		PrintLog("sTableName", sTableName);

		CDynaTable tmmtp02("TMMTP02", conn);
		if (bcls_rec->Tables.Contains("TMMTP02") && bcls_rec->Tables["TMMTP02"].Rows.get_Count() > 0)
		{
			tmmtp02.CopyFrom(bcls_rec->Tables["TMMTP02"]);
		}
		else if (config_name.Trim() != "")
		{
			tmmtp02.SetFilterColVal("CFGITM_NAME", config_name.Trim());
			tmmtp02.SetFilterColVal("CFGGRP_NAME", cfggrp_name.Trim());
			tmmtp02.Query();
			tmmtp02.CopyTo(bcls_rec->Tables["TMMTP02"]);
		}

		CDynaTable tmmtp05("TMMTP05", conn);
		if (bcls_rec->Tables.Contains("TMMTP05") && bcls_rec->Tables["TMMTP05"].Rows.get_Count() > 0)
		{
			tmmtp05.CopyFrom(bcls_rec->Tables["TMMTP05"]);
		}
		else if (config_name.Trim() != "")
		{
			if (operation_type.Trim() == "")
			{
				operation_type = "F3";
			}

			tmmtp05.SetFilterColVal("CFGITM_NAME", config_name.Trim());
			tmmtp05.SetFilterColVal("CFGGRP_NAME", cfggrp_name.Trim());
			tmmtp05.SetFilterColVal("OPERATION_TYPE", operation_type.Trim());
			tmmtp05.Query();
			tmmtp05.CopyTo(bcls_rec->Tables["TMMTP05"]);
		}

		//定义新增数据表
		CDynaTable dynaTable(sTableName, conn);

		if (tmmtp03.GetColValString("TABLE_TYPE") == "1")
		{
			if (tmmtp03.GetColValString("TABLE_ENAME").Trim() == "")
			{
				strcpy(s.msg, "没有配置虚拟数据表名");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			CString strCondition = "table_ename = '" + tmmtp03.GetColValString("TABLE_ENAME") + "'";
			PrintLog("strCondition", strCondition);

			CDynaTable tmmtp06("TMMTP06", conn);
			if (tmmtp06.Query(strCondition) <= 0)
			{
				strcpy(s.msg, "虚拟表未维护");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			int iRowNo = 0;
			int pkFlag = 0;
			for (int i = 0; i < bcls_rec->Tables["DATA_INS"].Rows.get_Count(); i++)
			{
				if (GetColValueC(bcls_rec->Tables["DATA_INS"], i, "REC_CREATOR").Trim() != "" &&
					GetColValueC(bcls_rec->Tables["DATA_INS"], i, "REC_CREATE_TIME").Trim() != "")
				{
					iInsFlag = 3;
				}

				CString sRowId = GetColValueC(bcls_rec->Tables["DATA_INS"], i, "NOW_ROW").Trim();
				if (sRowId == "")
				{
					sRowId = GetTrackSeqNo("MMTP_VTABLE_ROWID", 20, conn);
				}

				strCondition = "table_ename = '" + tmmtp03.GetColValString("TABLE_ENAME") + "'";

				for (int j = 0; j < bcls_rec->Tables["DATA_INS"].Columns.get_Count(); j++)
				{
					CString colName = bcls_rec->Tables["DATA_INS"].Columns[j].get_ColumnName();
					//PrintLog("colName", colName);

					for (int k = 0; k < tmmtp02.GetRowCount(); k++)
					{
						if (tmmtp02.GetColValString("COLUMN_NAME", k) == colName)
						{
							if (tmmtp02.GetColValString("KEYWORD_FLAG", k) == "1")
							{
								pkFlag++;
								strCondition += " AND item_ename = '" + colName + "' AND item_cvalue = '" + bcls_rec->Tables["DATA_INS"].Rows[i][colName].ToString() + "'";
							}

							break;
						}
					}

					for (int k = 0; k < tmmtp06.GetRowCount(); k++)
					{
						if (tmmtp06.GetColValString("ITEM_ENAME", k) == colName)
						{
							//PrintLog("iRowNo", iRowNo);
							dynaTable.MergeFrom(tmmtp06.GetDataRow(k), iRowNo);
							dynaTable.MergeFrom(bcls_rec->Tables["DATA_INS"].Rows[i], iRowNo);
							dynaTable.SetColVal("NOW_ROW", sRowId, iRowNo);

							if (GetColValueC(bcls_rec->Tables["DATA_INS"], i, "OLD_ROWNO").Trim() != "")
							{
								dynaTable.CopyColVal("OLD_ROWNO", bcls_rec->Tables["DATA_INS"].Rows[i], "OLD_ROWNO", iRowNo);
							}
							else if (GetColValueC(bcls_rec->Tables["DATA_INS"], i, "NOW_ROW").Trim() != "")
							{
								dynaTable.CopyColVal("OLD_ROWNO", bcls_rec->Tables["DATA_INS"].Rows[i], "NOW_ROW", iRowNo);
							}
							else
							{
								dynaTable.SetColVal("OLD_ROWNO", sRowId, iRowNo);
							}

							dynaTable.SetColVal("ITEM_CVALUE", bcls_rec->Tables["DATA_INS"].Rows[i][colName].ToString(), iRowNo);

							if (tmmtp06.GetColValString("ITEM_KIND", k) == "N")
							{
								dynaTable.SetColVal("ITEM_VALUE_N", bcls_rec->Tables["DATA_INS"].Rows[i][colName].ToDecimal(), iRowNo);
							}

							if (GetColValueC(bcls_rec->Tables["DATA_INS"], i, "AFFIRM_FLAG").Trim() == "")
							{
								dynaTable.SetColVal("AFFIRM_FLAG", "0", iRowNo);
								dynaTable.SetColVal("AFFIRM_BY", " ", iRowNo);
								dynaTable.SetColVal("AFFIRM_TIME", " ", iRowNo);
							}

							iRowNo++;
							break;
						}
					}
				}

				if (pkFlag > 0 && dynaTable.QueryCount(strCondition) > 0)
				{
					strcpy(s.msg, "主键冲突，无法新增");
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}
		}
		else
		{
			//获取前台传入新增数据
			dynaTable.CopyFrom(bcls_rec->Tables["DATA_INS"]);
		}

		for (int i = 0; i < tmmtp02.GetRowCount(); i++)
		{
			CString colName = tmmtp02.GetColValString("COLUMN_NAME", i);
			if (tmmtp02.GetColValString("DEFAULT_VALUE", i).Trim() != "")
			{
				if (tmmtp02.GetColValString("DATA_TYPE", i) == "C")
				{
					for (int j = 0; j < dynaTable.GetRowCount(); j++)
					{
						if (dynaTable.GetColValString(colName, j).Trim() == "")
						{
							dynaTable.SetColVal(colName, tmmtp02.GetColValString("DEFAULT_VALUE", i).Trim(), j);
						}
					}
				}
				else if (tmmtp02.GetColValString("DATA_TYPE", i) == "I")
				{
					for (int j = 0; j < dynaTable.GetRowCount(); j++)
					{
						if (dynaTable.GetColValString(colName, j).Trim() == "")
						{
							dynaTable.SetColVal(colName, (CDecimal)atoi(tmmtp02.GetColValString("DEFAULT_VALUE", i).Trim()), j);
						}
					}
				}
				else if (tmmtp02.GetColValString("DATA_TYPE", i) == "D")
				{
					for (int j = 0; j < dynaTable.GetRowCount(); j++)
					{
						if (dynaTable.GetColValString(colName, j).Trim() == "")
						{
							dynaTable.SetColVal(colName, (CDecimal)atof(tmmtp02.GetColValString("DEFAULT_VALUE", i).Trim()), j);
						}
					}
				}
			}
		}

		for (int i = 0; i < tmmtp05.GetRowCount(); i++)
		{
			if (tmmtp05.GetColValString("FUNC_NAME", i).Trim() != "")
			{
				//设置EDCALL参数
				CString funcName = tmmtp05.GetColValString("FUNC_NAME", i).Trim();
				CString pkName = tmmtp05.GetColValString("KEY_1", i).Trim();
				CString pkValue = tmmtp05.GetColValString("KEYVALUE_1", i).Trim();

				strcpy(e.func_name[0], funcName);
				strcpy(e.pk_name[0], pkName);
				strcpy(e.pk_val[0], pkValue);

				//将ED结构压入 EIClass func_rec 中
				bcls_rec->SetED(e);

				doFlag = f_epedcall(bcls_rec, bcls_ret);
				if (doFlag != 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (bcls_ret->Tables[0].Rows.get_Count() > 0)
				{
					for (int j = 0; j < bcls_ret->Tables[0].Columns.get_Count(); j++)
					{
						CString retColName = bcls_ret->Tables[0].Columns[j].get_ColumnName();
						dynaTable.AddUpdateColName(retColName);
						for (int k = 0; k < bcls_ret->Tables[0].Rows.get_Count(); k++)
						{
							dynaTable.CopyColVal(retColName, bcls_ret->Tables[0].Rows[k], k);
						}
					}
				}
			}
		}

		//新增数据
		if (dynaTable.Insert(iInsFlag) < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		dynaTable.CopyTo(bcls_ret->Tables[0]);
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