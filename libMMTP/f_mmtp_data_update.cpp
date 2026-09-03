/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2018
Author:      admin
Version:     1.0
Date:        2018-06-06 15:40:56
Description: 数据更新函数
**************************************************/

#include "CDynaTable.h"

BM2_FUNCTION_EXPORT
int f_mmtp_data_update(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CString config_name, CString cfggrp_name, CString operation_type)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	int archiveFlag = 0;
	CString sTableName = "";
	CString sSqlWhereAdd = "";

	try
	{
		if (!bcls_rec->Tables.Contains("DATA_UPD"))
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
				operation_type = "F4";
			}

			tmmtp05.SetFilterColVal("CFGITM_NAME", config_name.Trim());
			tmmtp05.SetFilterColVal("CFGGRP_NAME", cfggrp_name.Trim());
			tmmtp05.SetFilterColVal("OPERATION_TYPE", operation_type.Trim());
			tmmtp05.Query();
		}

		//定义修改数据表
		CDynaTable dynaTable(sTableName, conn);

		if (tmmtp03.GetColValString("TABLE_TYPE") == "1")
		{
			if (tmmtp03.GetColValString("TABLE_ENAME").Trim() == "")
			{
				strcpy(s.msg, "没有配置虚拟数据表名");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			dynaTable.AddUpdateColName("ITEM_CVALUE");
			dynaTable.AddUpdateColName("ITEM_VALUE_N");
			dynaTable.SetColVal("TABLE_ENAME", tmmtp03.GetColValString("TABLE_ENAME"));

			//定义修改前数据表
			CDynaTable dynaTablePre(sTableName, conn);
			CString strCondition = "table_ename = '" + tmmtp03.GetColValString("TABLE_ENAME") + "'";

			if (bcls_rec->Tables["DATA_UPD"].Columns.Contains("NOW_ROW"))
			{
				strCondition += " AND now_row IN (";
				for (int i = 0; i < bcls_rec->Tables["DATA_UPD"].Rows.get_Count(); i++)
				{
					if (i > 0)
					{
						strCondition += ",";
					}

					strCondition += "'" + bcls_rec->Tables["DATA_UPD"].Rows[i]["NOW_ROW"].ToString() + "'";
				}
			}
			else
			{
				strcpy(s.msg, "没有传入修改行号");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			strCondition += ")";
			PrintLog("strCondition", strCondition);
			dynaTablePre.AddOrderByAscColName("NOW_ROW");
			if (dynaTablePre.Query(strCondition) > 0)
			{
				if (tmmtp05.GetColValString("ITEM_UPD_MODE") == "2" || dynaTablePre.GetColValString("AFFIRM_FLAG") == "1")
				{
					CDynaTable dynaTableHistory("H" + sTableName.Substring(1), conn);
					dynaTableHistory.CopyFrom(dynaTablePre.GetDataTable());
					if (dynaTableHistory.Insert(1) < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					archiveFlag = 1;
				}

				if (!bcls_rec->Tables.Contains("DATA_DEL"))
				{
					bcls_rec->Tables.Add("DATA_DEL");
				}

				if (!bcls_rec->Tables.Contains("DATA_INS"))
				{
					bcls_rec->Tables.Add("DATA_INS");
				}

				for (int i = 0; i < bcls_rec->Tables["DATA_UPD"].Rows.get_Count(); i++)
				{
					bcls_rec->Tables["DATA_DEL"].Rows.Add();
					MergDataRow(bcls_rec->Tables["DATA_UPD"].Rows[i], bcls_rec->Tables["DATA_DEL"].Rows[i], true, true);

					bcls_rec->Tables["DATA_INS"].Rows.Add();
					for (int j = 0; j < dynaTablePre.GetRowCount(); j++)
					{
						if (dynaTablePre.GetColValString("NOW_ROW", j) == bcls_rec->Tables["DATA_UPD"].Rows[i]["NOW_ROW"].ToString())
						{
							if (archiveFlag == 0)
							{
								AddColValue(bcls_rec->Tables["DATA_INS"], i, "NOW_ROW", dynaTablePre.GetColValString("NOW_ROW", j));
							}
							else
							{
								AddColValue(bcls_rec->Tables["DATA_INS"], i, "NOW_ROW", " ");
							}

							AddColValue(bcls_rec->Tables["DATA_INS"], i, "OLD_ROWNO", dynaTablePre.GetColValString("OLD_ROWNO", j));
							AddColValue(bcls_rec->Tables["DATA_INS"], i, "AFFIRM_FLAG", dynaTablePre.GetColValString("AFFIRM_FLAG", j));
							AddColValue(bcls_rec->Tables["DATA_INS"], i, "AFFIRM_BY", dynaTablePre.GetColValString("AFFIRM_BY", j));
							AddColValue(bcls_rec->Tables["DATA_INS"], i, "AFFIRM_TIME", dynaTablePre.GetColValString("AFFIRM_TIME", j));
							AddColValue(bcls_rec->Tables["DATA_INS"], i, dynaTablePre.GetColValString("ITEM_ENAME", j), dynaTablePre.GetColValString("ITEM_CVALUE", j));
						}
					}

					for (int j = 0; j < tmmtp02.GetRowCount(); j++)
					{
						CString colName = tmmtp02.GetColValString("COLUMN_NAME", j);
						if (bcls_rec->Tables["DATA_UPD"].Columns.Contains(colName))
						{
							if (tmmtp05.GetColValString("ITEM_UPD_MODE") == "1" && tmmtp02.GetColValString("ITEM_UPD_MODE", j) == "1")
							{
								if (tmmtp02.GetColValString("DATA_TYPE", j) == "C")
								{
									AddColValue(bcls_rec->Tables["DATA_INS"], i, colName, bcls_rec->Tables["DATA_UPD"].Rows[0][colName].ToString());
								}
								else
								{
									AddColValue(bcls_rec->Tables["DATA_INS"], i, colName, bcls_rec->Tables["DATA_UPD"].Rows[0][colName].ToDecimal());
								}
							}
							else if (tmmtp02.GetColValString("REVISE_FLAG", j) == "1")
							{
								//PrintLog("colName", colName);
								if (tmmtp02.GetColValString("DATA_TYPE", j) == "C")
								{
									AddColValue(bcls_rec->Tables["DATA_INS"], i, colName, bcls_rec->Tables["DATA_UPD"].Rows[i][colName].ToString());
									//PrintLog("colValue", bcls_rec->Tables["DATA_UPD"].Rows[i][colName].ToString());
								}
								else
								{
									AddColValue(bcls_rec->Tables["DATA_INS"], i, colName, bcls_rec->Tables["DATA_UPD"].Rows[i][colName].ToDecimal());
									//PrintLog("colValue", bcls_rec->Tables["DATA_UPD"].Rows[i][colName].ToDecimal());
								}
							}
						}
					}
				}

				AddColValue(bcls_rec->Tables["DATA_INS"], "REC_CREATOR", (CString)s.userid);
				AddColValue(bcls_rec->Tables["DATA_INS"], "REC_CREATE_TIME", CDateTime::Now().ToString("yyyyMMddHHmmss"));

				PrintDataTable(bcls_rec->Tables["DATA_INS"]);
				doFlag = f_mmtp_data_delete(bcls_rec, bcls_ret, conn, "", "", "");
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				doFlag = f_mmtp_data_insert(bcls_rec, bcls_ret, conn, "", "", "");
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}
		}
		else
		{
			//获取前台传入修改数据
			dynaTable.CopyFrom(bcls_rec->Tables["DATA_UPD"]);
			dynaTable.Print();

			//添加条件列和修改列
			for (int i = 0; i < tmmtp02.GetRowCount(); i++)
			{
				CString colName = tmmtp02.GetColValString("COLUMN_NAME", i);
				if (tmmtp05.GetColValString("ITEM_UPD_MODE") == "1" && tmmtp02.GetColValString("REVISE_FLAG", i) == "1")
				{
					dynaTable.AddUpdateColName(colName);
					PrintLog("AddUpdateColName ", colName);

					for (int j = 1; j < bcls_rec->Tables["DATA_UPD"].Rows.get_Count(); j++)
					{
						dynaTable.CopyColVal(colName, j);
					}
				}
				else if (tmmtp02.GetColValString("REVISE_FLAG", i) == "1")
				{
					dynaTable.AddUpdateColName(colName);
					PrintLog("AddUpdateColName ", colName);
				}

				if (tmmtp02.GetColValString("KEYWORD_FLAG", i) == "1")
				{
					if (tmmtp02.GetColValString("DEFAULT_VALUE", i).Trim() != "")
					{
						sSqlWhereAdd += " AND " + colName + " = '" + tmmtp02.GetColValString("DEFAULT_VALUE", i).Trim() + "'";
					}
					else
					{
						dynaTable.AddFilterColName(colName);
						PrintLog("AddFilterColName ", colName);
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
							for (int k = 0; k < dynaTable.GetRowCount(); k++)
							{
								dynaTable.CopyColVal(retColName, bcls_ret->Tables[0].Rows[k], k);
							}
						}
					}
				}
			}

			//修改数据
			PrintLog("修改数据");
			if (sSqlWhereAdd.Trim() != "")
			{
				for (int i = 0; i < dynaTable.GetRowCount(); i++)
				{
					CString condition = dynaTable.BuildWhereString(i);
					if (condition.Trim() == "")
					{
						condition = " 1 = 1";
					}
					condition += sSqlWhereAdd;
					if (dynaTable.Update(condition, i) < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}
				}
			}
			else
			{
				if (dynaTable.Update() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}

			PrintLog("修改数据成功");
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