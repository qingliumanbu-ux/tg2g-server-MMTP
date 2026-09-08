/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2018
Author:      178773
Version:     1.0
Date:        2018-06-05 10:16:50
Description: 数据查询函数
**************************************************/

#include "CDynaTable.h"

BM2_FUNCTION_EXPORT
int f_mmtp_data_query(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	return f_mmtp_data_query(bcls_rec, bcls_ret, conn, "", "");
}

BM2_FUNCTION_EXPORT
int f_mmtp_data_query(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CString config_name, CString cfggrp_name)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString sTableName = "";
	CString sJoinTableName1 = "";
	CString sJoinTableName2 = "";
	CString sJoinType = "";
	CString sQryTabWhere = " ";
	CString sTemplateType = "";
	CString sQueryFlag = "";
	CString sqlstrSubTotal = "";
	CString sqlstrTotal = "";
	CString sqlFrom = "";
	CString sqlWhere = "";
	CString sqlWhereAdd = "";
	CString sqlGroupBy = "";
	CString sqlOrderBy = "";
	CString sqlWhereSubTotal = "";
	CString sqlWhereAddSubTotal = "";
	CString sqlGroupBySubTotal = "";
	CString sqlOrderBySubTotal = "";
	CString sqlCondition = "";
	CString sqlUnionSelect = "";

	int iRecordFrom = 0;
	int iPageSize = 0;
	int iTotalRecord = 0;
	int addNum = 0;
	int addNumSubTotal = 0;

	CDataTable dtQuery;
	CDataTable dtSubTotal;
	CDataTable dtTotal;

	list<CString> listColNameGroup;
	list<CString> listColNameSubTotal;

	try
	{
		if (!bcls_rec->Tables.Contains("CONDITION"))
		{
			strcpy(s.msg, "没有传入条件数据块");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		CDynaTable tmmtp01("TMMTP01", conn);
		if (bcls_rec->Tables.Contains("TMMTP01") && bcls_rec->Tables["TMMTP01"].Rows.get_Count() > 0)
		{
			tmmtp01.CopyFrom(bcls_rec->Tables["TMMTP01"]);
		}
		else if (config_name.Trim() != "")
		{
			tmmtp01.SetFilterColVal("CFGITM_NAME", config_name.Trim());
			tmmtp01.SetFilterColVal("CFGGRP_NAME", cfggrp_name.Trim());
			tmmtp01.Query();
		}

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

		sJoinTableName1 = tmmtp03.GetColValString("TABLE_NAME_1");
		PrintLog("sJoinTableName1", sJoinTableName1);

		sJoinTableName2 = tmmtp03.GetColValString("TABLE_NAME_2");
		PrintLog("sJoinTableName2", sJoinTableName2);

		sJoinType = tmmtp03.GetColValString("JOINT_TYPE_CODE");
		PrintLog("sJoinType", sJoinType);

		sQryTabWhere = GetColValueC(bcls_rec->Tables["CONDITION"], 0, "QRY_TAB_WHERE");
		PrintLog("sQryTabWhere", sQryTabWhere);

		if (bcls_rec->Tables.Contains("PAGEINFO"))
		{
			iRecordFrom = GetColValueD(bcls_rec->Tables["PAGEINFO"], 0, "NOW_RECORD").ToInt32();
			PrintLog("iRecordFrom", iRecordFrom);

			iPageSize = GetColValueD(bcls_rec->Tables["PAGEINFO"], 0, "EVERY_PAGE").ToInt32();
			PrintLog("iPageSize", iPageSize);
		}

		if (tmmtp03.GetColValString("TEMPLATE_TYPE") == "07")
		{
			PrintLog("统计报表");

			sqlstr = "SELECT ";
			sqlFrom = " FROM " + sTableName;
			sqlstrSubTotal = "SELECT ";
			sqlstrTotal = "SELECT ";
			sqlGroupBy = " GROUP BY ";
			sqlOrderBy = " ORDER BY ";
			sqlGroupBySubTotal = " GROUP BY ";
			sqlOrderBySubTotal = " ORDER BY ";

			for (int i = 0; i < tmmtp01.GetRowCount(); i++)
			{
				CString colName = tmmtp01.GetColValString("COLUMN_NAME", i);
				if (tmmtp01.GetColValString("DEFAULT_VALUE", i).Trim() != "" &&
					tmmtp01.GetColValString("DISP_COL_FLAG", i) == "0")
				{
					tmmtp01.Print("COLUMN_NAME", i);
					if (tmmtp01.GetColValString("CONTROL_CLASS", i).Substring(0, 1) != "S")
					{
						if (!bcls_rec->Tables["CONDITION"].Columns.Contains(colName))
						{
							bcls_rec->Tables["CONDITION"].Columns.Add(DT_STRING, colName);
						}
					}
					else
					{
						if (!bcls_rec->Tables["CONDITION"].Columns.Contains(colName))
						{
							bcls_rec->Tables["CONDITION"].Columns.Add(DT_DECIMAL, colName);
						}
					}

					//tmmtp01.Print("DEFAULT_VALUE", i);
					if (bcls_rec->Tables["CONDITION"].Rows.get_Count() == 0)
					{
						bcls_rec->Tables["CONDITION"].Rows.Add();
					}

					bcls_rec->Tables["CONDITION"].Rows[0][colName] = tmmtp01.GetColValString("DEFAULT_VALUE", i);
				}

				if (tmmtp01.GetColValString("QRY_TAB_WHERE", i).Trim() != "")
				{
					sQryTabWhere += tmmtp01.GetColValString("QRY_TAB_WHERE", i).Trim();
				}
			}

			PrintLog("111");
			if (bcls_rec->Tables["CONDITION"].Rows.get_Count() > 0)
			{
				sqlCondition = SetWhereSQL(sTableName, bcls_rec->Tables["CONDITION"].Rows[0], conn);
			}

			PrintLog("sqlCondition", sqlCondition);

			if (sQryTabWhere.Trim() != "")
			{
				if (sqlCondition.Trim() == "")
				{
					sqlCondition = sQryTabWhere;
				}
				else
				{
					sqlCondition += " AND " + sQryTabWhere;
				}
			}

			bcls_ret->Tables.Add("SQL_WHERE");
			bcls_ret->Tables["SQL_WHERE"].Columns.Add(DT_STRING, "SQL_WHERE");
			bcls_ret->Tables["SQL_WHERE"].Rows.Add();
			bcls_ret->Tables["SQL_WHERE"].Rows[0]["SQL_WHERE"] = sqlCondition;

			for (int i = 0; i < tmmtp02.GetRowCount(); i++)
			{
				if (tmmtp02.GetColValString("GROUP_FLAG", i) == "1" || tmmtp02.GetColValString("GROUP_FLAG", i) == "2" ||
					tmmtp02.GetColValString("QUERY_SQL", i).Trim() == "")
				{
					CString colName = tmmtp02.GetColValString("COLUMN_NAME", i);
					listColNameGroup.push_back(colName);

					if (tmmtp02.GetColValString("GROUP_FLAG", i) == "1" ||
						tmmtp02.GetColValString("GROUP_FLAG", i) == "2")
					{
						sqlWhere += " AND " + colName + " = t." + colName;
						sqlWhereAdd += " AND " + colName + " > ' '";

						if (addNum > 0)
						{
							sqlstr += ",";
							sqlGroupBy += ",";
							sqlOrderBy += ",";
						}

						sqlstr += colName;
						sqlGroupBy += colName;
						sqlOrderBy += colName;

						if (tmmtp02.GetColValString("GROUP_FLAG", i) == "2")
						{
							listColNameSubTotal.push_back(colName);

							sqlWhereSubTotal += " AND " + colName + " = t." + colName;
							sqlWhereAddSubTotal += " AND " + colName + " > ' '";

							if (addNumSubTotal > 0)
							{
								sqlstrSubTotal += ",";
								sqlGroupBySubTotal += ",";
								sqlOrderBySubTotal += ",";
							}

							sqlstrSubTotal += colName;
							sqlGroupBySubTotal += colName;
							sqlOrderBySubTotal += colName;

							addNumSubTotal++;
						}
					}

					//if (addNum > 0)
					//{	
					//	if (sJoinTableName1.Trim() != "" && sJoinType == "U")
					//	{
					//		sqlUnionSelect += ",";
					//	}
					//}

					//if (sJoinTableName1.Trim() != "" && sJoinType == "U")
					//{
					//	sqlUnionSelect += colName;
					//}

					addNum++;
				}
			}

			if (sJoinTableName1.Trim() != "" && sJoinType == "U")
			{
				CDataTable dtTable;
				CDataTable dtTable1;
				CDataTable dtTable2;
				SetDataTableColName(sTableName, dtTable, conn);
				SetDataTableColName(sJoinTableName1, dtTable1, conn);
				MergeDataTableColumn(dtTable, dtTable1, conn);

				if (sJoinTableName2.Trim() != "")
				{
					SetDataTableColName(sJoinTableName2, dtTable2, conn);
					MergeDataTableColumn(dtTable, dtTable2, conn);
				}

				sqlFrom = " FROM (" + SetInqSQL(sTableName, dtTable, conn) + " UNION ALL " + SetInqSQL(sJoinTableName1, dtTable, conn);
				if (sJoinTableName2.Trim() != "")
				{
					sqlFrom += " UNION ALL " + SetInqSQL(sJoinTableName2, dtTable, conn);
				}
				sqlFrom += ")";
				PrintLog("sqlFrom", sqlFrom);
			}

			for (int i = 0; i < tmmtp02.GetRowCount(); i++)
			{
				if (tmmtp02.GetColValString("GROUP_FLAG", i) == "0" &&
					tmmtp02.GetColValString("QUERY_SQL", i).Trim() != "")
				{
					CString colName = tmmtp02.GetColValString("COLUMN_NAME", i);
					CString sqlSelect = tmmtp02.GetColValString("QUERY_SQL", i).Trim();
					CString sqlConditionSub = tmmtp02.GetColValString("CND_RELATION", i).Trim();

					if (addNum > 0)
					{
						sqlstr += ",";
					}

					if (addNumSubTotal > 0)
					{
						sqlstrSubTotal += ",";
					}

					if (sqlstrTotal != "SELECT ")
					{
						sqlstrTotal += ",";
					}

					sqlstr += "(SELECT " + sqlSelect + sqlFrom + " WHERE 1 = 1 ";
					sqlstrSubTotal += "(SELECT " + sqlSelect + sqlFrom + " WHERE 1 = 1 ";
					sqlstrTotal += "(SELECT " + sqlSelect + sqlFrom + " WHERE 1 = 1 ";

					if (sqlCondition.Trim() != "")
					{
						sqlstr += " AND " + sqlCondition;
						sqlstrSubTotal += " AND " + sqlCondition;
						sqlstrTotal += " AND " + sqlCondition;
					}

					if (sqlConditionSub.Trim() != "")
					{
						sqlstr += " AND " + sqlConditionSub;
						sqlstrSubTotal += " AND " + sqlConditionSub;
						sqlstrTotal += " AND " + sqlConditionSub;
					}

					if (sqlWhere.Trim() != "")
					{
						sqlstr += sqlWhere;
					}

					if (sqlWhereSubTotal.Trim() != "")
					{
						sqlstrSubTotal += sqlWhereSubTotal;
					}

					sqlstr += ") AS " + colName;
					sqlstrSubTotal += ") AS " + colName;
					sqlstrTotal += ") AS " + colName;

					addNum++;
				}
			}

			sqlstr += sqlFrom + " t WHERE 1 = 1";
			sqlstrSubTotal += sqlFrom + " t WHERE 1 = 1";

			if (conn->DatabaseKind == DB_KIND_ORACLE)
			{
				sqlstrTotal += " FROM dual WHERE 1 = 1";
			}
			else if (conn->DatabaseKind == DB_KIND_DB2 || conn->DatabaseKind == DB_KIND_DB2_ORACLE)
			{
// DM8 适配 CHANGE-271:查询。SYSIBM 辅助表改为 DUAL。
// 改写原因：SYSIBM 辅助表改为 DUAL；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
				// sqlstrTotal += " FROM sysibm.sysdummy1";
// DM8 SQL：
				sqlstrTotal += " FROM DUAL";
			}

			if (sqlCondition.Trim() != "")
			{
				sqlstr += " AND " + sqlCondition;
				sqlstrSubTotal += " AND " + sqlCondition;
			}

			if (sqlWhereAdd.Trim() != "")
			{
				sqlstr += sqlWhereAdd;
			}

			if (sqlWhereAddSubTotal.Trim() != "")
			{
				sqlstrSubTotal += sqlWhereAddSubTotal;
			}

			sqlstr += sqlGroupBy + sqlOrderBy;

			PrintLog("sqlstr", sqlstr);
			PrintLog("sqlstrTotal", sqlstrTotal);

			QueryData(sqlstr, dtQuery, conn);

			if (tmmtp03.GetColValString("EXT_ITEM1") == "1")
			{
				QueryData(sqlstrTotal, dtTotal, conn);
			}

			if (addNumSubTotal > 0)
			{
				sqlstrSubTotal += sqlGroupBySubTotal + sqlOrderBySubTotal;
				PrintLog("sqlstrSubTotal", sqlstrSubTotal);
				QueryData(sqlstrSubTotal, dtSubTotal, conn);
			}

			bcls_ret->Tables[0].Clone(dtQuery);

			tmmtp03.Print("EXT_ITEM1");
			if (tmmtp03.GetColValString("EXT_ITEM1") == "1")
			{
				bcls_ret->Tables[0].Columns.Add(DT_STRING, "TOTAL");
			}
			
			for (int i = 0; i < dtQuery.Rows.get_Count(); i++)
			{
				if (i == 0 || addNumSubTotal == 0)
				{
					bcls_ret->Tables[0].Rows.Add();
					bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1].Merge(dtQuery.Rows[i]);
				}
				else
				{
					for (list<CString>::const_iterator iter = listColNameSubTotal.begin(); iter != listColNameSubTotal.end(); iter++)
					{
						if (dtQuery.Rows[i][*iter].ToString() == dtQuery.Rows[i - 1][*iter].ToString())
						{
							bcls_ret->Tables[0].Rows.Add();
							bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1].Merge(dtQuery.Rows[i]);
							//bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1][*iter] = " ";
						}
						else
						{
							for (int j = 0; j < dtSubTotal.Rows.get_Count(); j++)
							{
								if (dtSubTotal.Rows[j][*iter].ToString() == dtQuery.Rows[i - 1][*iter].ToString())
								{
									bcls_ret->Tables[0].Rows.Add();
									bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1].Merge(dtSubTotal.Rows[j]);
									if (tmmtp03.GetColValString("EXT_ITEM1") == "1")
									{
										bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1]["TOTAL"] = "小计";
									}
									break;
								}
							}

							bcls_ret->Tables[0].Rows.Add();
							bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1].Merge(dtQuery.Rows[i]);
						}
					}

					if (i == dtQuery.Rows.get_Count() - 1)
					{
						for (list<CString>::const_iterator iter = listColNameSubTotal.begin(); iter != listColNameSubTotal.end(); iter++)
						{
							for (int j = 0; j < dtSubTotal.Rows.get_Count(); j++)
							{
								if (dtSubTotal.Rows[j][*iter].ToString() == dtQuery.Rows[i][*iter].ToString())
								{
									bcls_ret->Tables[0].Rows.Add();
									bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1].Merge(dtSubTotal.Rows[j]);
									if (tmmtp03.GetColValString("EXT_ITEM1") == "1")
									{
										bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1]["TOTAL"] = "小计";
									}
									break;
								}
							}
						}
					}
				}
			}

			if (tmmtp03.GetColValString("EXT_ITEM1") == "1")
			{
				bcls_ret->Tables[0].Rows.Add();
				bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1].Merge(dtTotal.Rows[0]);
				bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1]["TOTAL"] = "总计";
			}
			
			for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
			{
				for (int j = 0; j < bcls_ret->Tables[0].Columns.get_Count(); j++)
				{
					if (bcls_ret->Tables[0].Columns[j].get_ColumnName() != "TOTAL" &&
						bcls_ret->Tables[0].Rows[i][j].ToString() == "")
					{
						int groupFlag = 0;
						for (list<CString>::const_iterator iter = listColNameGroup.begin(); iter != listColNameGroup.end(); iter++)
						{
							if (bcls_ret->Tables[0].Columns[j].get_ColumnName() == *iter)
							{
								groupFlag = 1;
								break;
							}
						}

						if (groupFlag == 0)
						{
							bcls_ret->Tables[0].Rows[i][j] = "0";
						}
					}
				}
			}

			//PrintDataTable(bcls_ret->Tables[0]);

			//if (iPageSize > 0)
			//{
			//	QueryData(sqlstr, iRecordFrom, iPageSize, iTotalRecord, bcls_rec->Tables["CONDITION"], conn);
			//}
			//else
			//{
			//	QueryData(sqlstr, bcls_rec->Tables["CONDITION"], conn);
			//}

			bcls_ret->Tables.Add("SQL_FROM");
			bcls_ret->Tables["SQL_FROM"].Columns.Add(DT_STRING, "SQL_FROM");
			bcls_ret->Tables["SQL_FROM"].Rows.Add();
			bcls_ret->Tables["SQL_FROM"].Rows[0]["SQL_FROM"] = sqlFrom;
		}
		else
		{
			for (int i = 0; i < tmmtp01.GetRowCount(); i++)
			{
				CString colName = tmmtp01.GetColValString("COLUMN_NAME", i);
				if (tmmtp01.GetColValString("DEFAULT_VALUE", i).Trim() != "" &&
					tmmtp01.GetColValString("DISP_COL_FLAG", i) == "0")
				{
					tmmtp01.Print("COLUMN_NAME", i);
					if (tmmtp01.GetColValString("CONTROL_CLASS", i).Substring(0, 1) != "S")
					{
						if (!bcls_rec->Tables["CONDITION"].Columns.Contains(colName))
						{
							bcls_rec->Tables["CONDITION"].Columns.Add(DT_STRING, colName);
						}
					}
					else
					{
						if (!bcls_rec->Tables["CONDITION"].Columns.Contains(colName))
						{
							bcls_rec->Tables["CONDITION"].Columns.Add(DT_DECIMAL, colName);
						}
					}

					tmmtp01.Print("DEFAULT_VALUE", i);

					if (bcls_rec->Tables["CONDITION"].Rows.get_Count() == 0)
					{
						bcls_rec->Tables["CONDITION"].Rows.Add();
					}
					bcls_rec->Tables["CONDITION"].Rows[0][colName] = tmmtp01.GetColValString("DEFAULT_VALUE", i);
				}

				if (tmmtp01.GetColValString("QRY_TAB_WHERE", i).Trim() != "")
				{
					sQryTabWhere += tmmtp01.GetColValString("QRY_TAB_WHERE", i).Trim();
				}
			}

			CDynaTable dynaTable(sTableName, conn);
			dynaTable.AddOrderByAscColName("NOW_ROW");

			if (tmmtp03.GetColValString("TABLE_TYPE") == "1")
			{
				if (tmmtp03.GetColValString("TABLE_ENAME").Trim() == "")
				{
					strcpy(s.msg, "没有配置虚拟数据表名");
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				CString colName = "";
				CString colNameRec = "";
				CString colVal = "";
				CString strOperator = "";
				CString strCondition = "table_ename = '" + tmmtp03.GetColValString("TABLE_ENAME") + "'";

				CDynaTable tmmtp06("TMMTP06", conn);
				if (tmmtp06.Query(strCondition) <= 0)
				{
					strcpy(s.msg, "虚拟表未维护");
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				for (int i = 0; i < bcls_rec->Tables["CONDITION"].Columns.get_Count(); i++)
				{
					strCondition += " AND now_row IN (SELECT now_row FROM " + sTableName + " WHERE table_ename = '" +
						tmmtp03.GetColValString("TABLE_ENAME") + "'";

					colNameRec = bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().ToUpper();
					colVal = bcls_rec->Tables["CONDITION"].Rows[0][colNameRec].ToString();

					if (bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(0, 1) == ">")
					{
						if (bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(0, 2) == ">=")
						{
							colName = bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(2).ToUpper();
							strOperator = " >= ";
						}
						else
						{
							colName = bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(1).ToUpper();
							strOperator = " > ";
						}
					}
					else if (bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(0, 1) == "<")
					{
						if (bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(0, 2) == "<=")
						{
							colName = bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(2).ToUpper();
							strOperator = " <= ";
						}
						else
						{
							colName = bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(1).ToUpper();
							strOperator = " < ";
						}
					}
					else if (bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(0, 1) == "%")
					{
						colName = bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(1).ToUpper();
						strOperator = " LIKE ";
					}
					else if (bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(0, 1) == "^")
					{
						colName = bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(1).ToUpper();
						strOperator = " IN ";
					}
					else if (bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(0, 1) == "=")
					{
						colName = bcls_rec->Tables["CONDITION"].Columns[i].get_ColumnName().Substring(1).ToUpper();
						strOperator = " = ";
					}
					else
					{
						colName = colNameRec;
						strOperator = " = ";
					}

					//PrintLog("colName", colName);
					//PrintLog("strOperator", strOperator);
					//PrintLog("colVal", colVal);

					int existsFlag = 0;
					for (int j = 0; j < tmmtp06.GetRowCount(); j++)
					{
						if (tmmtp06.GetColValString("ITEM_ENAME", j) == colName)
						{
							if (colVal.Trim() != "")
							{
								strCondition += " AND item_ename = '" + colName + "' AND item_cvalue ";
								if (strOperator == " LIKE ")
								{
									strCondition += "LIKE '" + colVal + "%'";
								}
								else if (strOperator == " IN ")
								{
									strCondition += "IN (" + colVal + ")";
								}
								else
								{
									strCondition += strOperator + "'" + colVal + "'";
								}
							}

							existsFlag = 1;
							break;
						}
					}

					if (existsFlag == 0 && dynaTable.Contains(colName) && colVal.Trim() != "")
					{
						if (strOperator == " LIKE ")
						{
							strCondition += " AND " + colName + "LIKE '" + colVal + "%'";
						}
						else if (strOperator == " IN ")
						{
							strCondition += " AND " + colName + "IN (" + colVal + ")";
						}
						else
						{
							strCondition += " AND " + colName + strOperator + "'" + colVal + "'";
						}
					}

					strCondition += ")";
				}


				PrintLog("strCondition", strCondition);

				dynaTable.Query(strCondition);
				//dynaTable.Print();

				for (int i = 0; i < tmmtp02.GetRowCount(); i++)
				{
					CString colName = tmmtp02.GetColValString("COLUMN_NAME", i);
					//PrintLog("colName", colName);

					if (!bcls_ret->Tables[0].Columns.Contains(colName))
					{
						if (tmmtp02.GetColValString("DATA_TYPE", i) == "C")
						{
							bcls_ret->Tables[0].Columns.Add(DT_STRING, colName);
						}
						else
						{
							bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, colName);
						}
					}
				}

				if (!bcls_ret->Tables[0].Columns.Contains("NOW_ROW"))
				{
					bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "NOW_ROW");
				}

				for (int i = 0; i < dynaTable.GetRowCount(); i++)
				{
					if (i == 0 || dynaTable.GetColValString("NOW_ROW", i) > dynaTable.GetColValString("NOW_ROW", i - 1))
					{
						bcls_ret->Tables[0].Rows.Add();
					}

					//PrintLog("i", i);
					//PrintLog("RowsCount", bcls_ret->Tables[0].Rows.get_Count());

					CDataRow& drRet = bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1];
					for (int j = 0; j < tmmtp02.GetRowCount(); j++)
					{
						CString colName = tmmtp02.GetColValString("COLUMN_NAME", j);

						if (dynaTable.GetColValString("ITEM_ENAME", i) == colName)
						{
							//PrintLog("竖表字段", colName);
							if (tmmtp02.GetColValString("DATA_TYPE", j) == "C")
							{
								drRet[colName] = dynaTable.GetColValString("ITEM_CVALUE", i);
							}
							else
							{
								int pointNum = 0;
								for (int k = 0; k < tmmtp06.GetRowCount(); k++)
								{
									if (tmmtp06.GetColValString("ITEM_ENAME", k) == colName)
									{
										int pointPos = tmmtp06.GetColValString("ITEM_LEN", k).Find(",");
										if (pointPos > 0)
										{
											pointNum = atoi(tmmtp06.GetColValString("ITEM_LEN", k).Substring(pointPos + 1));
										}
										break;
									}
								}

								drRet[colName] = dynaTable.GetColValDecimal("ITEM_VALUE_N", i).Round(pointNum);
							}

							break;
						}
					}

					//dynaTable.Print("NOW_ROW", i);
					MergDataRow(dynaTable.GetDataRow(i), drRet, false, false);
				}

				//PrintDataTable(bcls_ret->Tables[0]);

				iTotalRecord = bcls_ret->Tables[0].Rows.get_Count();
			}
			else
			{
				//设置连接表名
				sJoinType = GetColValueC(bcls_rec->Tables["TMMTP03"], 0, "JOINT_TYPE_CODE");
				PrintLog("sJoinType", sJoinType);

				sJoinTableName1 = GetColValueC(bcls_rec->Tables["TMMTP03"], 0, "TABLE_NAME_1");
				PrintLog("sJoinTableName1", sJoinTableName1);

				sJoinTableName2 = GetColValueC(bcls_rec->Tables["TMMTP03"], 0, "TABLE_NAME_2");
				PrintLog("sJoinTableName2", sJoinTableName2);

				dynaTable.SetTableName(sTableName);

				//设置连接表名
				if (sJoinType == "U")
				{
					if (sJoinTableName1.Trim() != "")
					{
						dynaTable.AddUnionTable(sJoinTableName1.Trim());
					}

					if (sJoinTableName2.Trim() != "")
					{
						dynaTable.AddUnionTable(sJoinTableName2.Trim());
					}

					dynaTable.SetUnionTable();
				}
				else if (sJoinTableName1.Trim() != "")
				{
					dynaTable.SetJoinTable(sJoinTableName1, sJoinTableName2, sJoinType);
				}

				for (int i = 0; i < tmmtp02.GetRowCount(); i++)
				{
					CString colName = tmmtp02.GetColValString("COLUMN_NAME", i);

					if (sJoinType == "I" || sJoinType == "L")
					{
						PrintLog("i", i);
						tmmtp02.Print("TABLE_NAME", i);
						PrintLog("colName", colName);

						if (tmmtp02.GetColValString("TABLE_NAME", i).Substring(1) == sJoinTableName1.Substring(1))
						{
							dynaTable.AddJoinSelectColName1(colName);

							PrintLog("colName", colName);
							tmmtp02.Print("FOREIGN_KEY_SEQ", i);
							if (tmmtp02.GetColValDecimal("FOREIGN_KEY_SEQ", i) > 0)
							{
								PrintLog("FOREIGN_KEY_SEQ colName", colName);

								CString strJoin = "t1." + colName + " = ";
								PrintLog("1.strJoin", strJoin);

								int joinFlag = 0;
								for (int j = 0; j < tmmtp02.GetRowCount(); j++)
								{
									if (tmmtp02.GetColValString("TABLE_NAME", j).Substring(1) == sTableName.Substring(1) &&
										tmmtp02.GetColValDecimal("FOREIGN_KEY_SEQ", j) == tmmtp02.GetColValDecimal("FOREIGN_KEY_SEQ", i))
									{
										joinFlag = 1;
										strJoin += "t." + tmmtp02.GetColValString("COLUMN_NAME", j);
										PrintLog("2.strJoin", strJoin);

										break;
									}
								}

								if (joinFlag == 0)
								{
									strJoin += "t." + colName;
								}

								dynaTable.AddJoinColName1(strJoin);
							}
						}
						else if (tmmtp02.GetColValString("TABLE_NAME", i).Substring(1) == sJoinTableName2.Substring(1))
						{
							dynaTable.AddJoinSelectColName2(colName);

							if (tmmtp02.GetColValDecimal("FOREIGN_KEY_SEQ", i) > 0)
							{
								CString strJoin = "t2." + colName + " = ";
								int joinFlag = 0;
								for (int j = 0; j < tmmtp02.GetRowCount(); j++)
								{
									if (tmmtp02.GetColValString("TABLE_NAME", j).Substring(1) == sTableName.Substring(1) &&
										tmmtp02.GetColValDecimal("FOREIGN_KEY_SEQ", j) == tmmtp02.GetColValDecimal("FOREIGN_KEY_SEQ", i))
									{
										joinFlag = 1;
										strJoin += "t." + tmmtp02.GetColValString("COLUMN_NAME", j);
										break;
									}
								}

								if (joinFlag == 0)
								{
									strJoin += "t." + colName;
								}

								dynaTable.AddJoinColName2(strJoin);
							}
						}
					}

					if (tmmtp02.GetColValString("ORDER_MARK", i) == "A")
					{
						dynaTable.AddOrderByAscColName(colName);
					}
					else if (tmmtp02.GetColValString("ORDER_MARK", i) == "D")
					{
						dynaTable.AddOrderByDescColName(colName);
					}
				}

				//PrintDataTable(bcls_rec->Tables["CONDITION"]);

				CString condition = "";
				if (bcls_rec->Tables["CONDITION"].Rows.get_Count() > 0)
				{
					condition = dynaTable.BuildWhereString(bcls_rec->Tables["CONDITION"].Rows[0]);
					if (sQryTabWhere.Trim() != "")
					{
						if (condition.Trim() == "")
						{
							condition = sQryTabWhere;
						}
						else
						{
							condition += " AND " + sQryTabWhere;
						}
					}
				}
				
				if (condition.Trim() == "")
				{
					condition = "1 = 1";
				}
				
				PrintLog("condition", condition);
				iTotalRecord = dynaTable.Query(condition, 0, iRecordFrom, iPageSize);
				PrintLog("iTotalRecord", iTotalRecord);
				PrintLog("返回行数", dynaTable.GetRowCount());
				if (iTotalRecord < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				//for (int i = 0; i < tmmtp02.GetRowCount(); i++)
				//{
				//CString colName = tmmtp02.GetColValString("COLUMN_NAME", i);
				//if (tmmtp02.GetColValDecimal("TRANS_FACTOR", i) != 0)
				//{
				//	double transFactor = tmmtp02.GetColValDecimal("TRANS_FACTOR", i).ToDouble();

				//	for (int j = 0; j < dynaTable.GetRowCount(); j++)
				//	{
				//		dynaTable.SetColVal(colName, dynaTable.GetColValDecimal(colName, j) * pow(10, transFactor), j);
				//		dynaTable.Print(colName, j);
				//	}
				//}

				//if (tmmtp02.GetColValString("CONTROL_CLASS", i).Substring(0, 1) == "D")
				//{
				//	for (int j = 0; j < dynaTable.GetRowCount(); j++)
				//	{
				//		CString colValue = dynaTable.GetColValString(colName, j);
				//		if (tmmtp02.GetColValString("CONTROL_CLASS", i).Substring(0, 2) == "D1" && colValue.GetLength() >= 8)
				//		{
				//			colValue = colValue.Substring(0, 4) + "-" + colValue.Substring(4, 2) + "-" + colValue.Substring(6, 2);
				//		}
				//		else if (tmmtp02.GetColValString("CONTROL_CLASS", i).Substring(0, 2) == "D2" && colValue.GetLength() >= 12)
				//		{
				//			colValue = colValue.Substring(0, 4) + "-" + colValue.Substring(4, 2) + "-" + colValue.Substring(6, 2) + " " +
				//				colValue.Substring(8, 2) + ":" + colValue.Substring(10, 2);
				//		}
				//		else if (tmmtp02.GetColValString("CONTROL_CLASS", i).Substring(0, 2) == "D3" && colValue.GetLength() >= 14)
				//		{
				//			colValue = colValue.Substring(0, 4) + "-" + colValue.Substring(4, 2) + "-" + colValue.Substring(6, 2) + " " +
				//				colValue.Substring(8, 2) + ":" + colValue.Substring(10, 2) + ":" + colValue.Substring(12, 2);
				//		}

				//		dynaTable.SetColVal(colName, colValue, j);
				//	}
				//}
				//}

				//dynaTable.Print();
				dynaTable.CopyTo(bcls_ret->Tables[0]);

				if (!bcls_ret->Tables[0].Columns.Contains("NOW_ROW"))
				{
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "NOW_ROW");
				}

				for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
				{
					bcls_ret->Tables[0].Rows[i]["NOW_ROW"] = CDecimal(i).ToString();
				}
			}
		}

		if (iPageSize > 0)
		{
			//返回分页信息
			bcls_ret->Tables.Add("PAGEINFO");	//增加块
			bcls_ret->Tables["PAGEINFO"].Columns.Add(DT_DECIMAL, "TOTAL_RECORD");	//总记录数
			bcls_ret->Tables["PAGEINFO"].Rows.Add();
			bcls_ret->Tables["PAGEINFO"].Rows[0][0] = iTotalRecord;
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

	doFlag = iTotalRecord;
	return doFlag;
}