/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2022
Author:      admin
Version:     1.0
Date:        2022-07-12 16:22:38
Description: 保存数据源配置
**************************************************/

#include "CDynaTable2.h"

BM2_FUNCTION_IMPORT
int f_gctp_dsItemNew(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn, CString cfgitmName);

BM2F_ENTERACE(gctp_setDsConfig)
int f_gctp_setDsConfig(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	//应用变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString dsName = "";

	//动态数据表定义
	CDynaTable2 tgctp02("TGCTP02", conn);
	CDynaTable2 tgctp03("TGCTP03", conn);

	//数据库操作类定义
	CDbCommand cmd(conn);

	try
	{
		dsName = bcls_rec->Tables[0].Rows[0]["CFGITM_NAME"].ToString().Trim();
		PrintLog("dsName", dsName);

		if (bcls_rec->Tables.Contains("INSERT_BLOCK") && bcls_rec->Tables["INSERT_BLOCK"].Rows.get_Count() > 0)
		{
			bcls_rec->Tables.Add("DS_ITEM");
			bcls_rec->Tables["DS_ITEM"].Clone(bcls_rec->Tables["INSERT_BLOCK"]);
			for (int i = 0; i < bcls_rec->Tables["INSERT_BLOCK"].Rows.get_Count(); i++)
			{
				bcls_rec->Tables["DS_ITEM"].Rows.Add();
				bcls_rec->Tables["DS_ITEM"].Rows[i].Merge(bcls_rec->Tables["INSERT_BLOCK"].Rows[i]);
			}

			doFlag = f_gctp_dsItemNew(bcls_rec, bcls_ret, conn, dsName);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}
		
		if (bcls_rec->Tables.Contains("UPDATE_BLOCK") && bcls_rec->Tables["UPDATE_BLOCK"].Rows.get_Count() > 0)
		{
			for (int i = 0; i < bcls_rec->Tables["UPDATE_BLOCK"].Rows.get_Count(); i++)
			{
				tgctp03.MergeFrom(bcls_rec->Tables["UPDATE_BLOCK"].Rows[i]);
				//tgctp03.Print();

				for (int j = 0; j < bcls_rec->Tables["DS_ITEM_CONFIG"].Rows.get_Count(); j++)
				{
					CString colName = bcls_rec->Tables["DS_ITEM_CONFIG"].Rows[j]["ITEM_ENAME"].ToString();				
					if (bcls_rec->Tables["DS_ITEM_CONFIG"].Rows[j]["GROUP_RULE_NO"].ToString().Trim() != "")
					{
						colName = bcls_rec->Tables["DS_ITEM_CONFIG"].Rows[j]["GROUP_RULE_NO"].ToString();
					}

					tgctp03.AddUpdateColName(colName);
					
					if (colName == "COLUMN_CNAME")
					{
						//同步更新画面配置字段列名
						tgctp02.AddUpdateColName(colName);
						tgctp02.CopyColVal(colName, bcls_rec->Tables["UPDATE_BLOCK"].Rows[i]);
					}
				}

				if (tgctp03.Update() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				if (tgctp02.GetUpdateColString().Trim() != "")
				{
					tgctp02.SetFilterColVal("TIMESTAMP", tgctp03.GetColValString("TIMESTAMP"));
					if (tgctp02.Update() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}
				}

				//条件字段判断
				int existsFlag = 0;
				CDecimal seqNo = 0;

				sqlstr = "SELECT timestamp,seq_no_01 FROM tgctp02 WHERE cfgitm_name = '" + dsName + "' AND cfgitm_grp_name = 'CONDITION_ITEM' ORDER BY seq_no_01";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				while (cmd.Read())
				{
					if (cmd.GetString(1) == tgctp03.GetColValString("TIMESTAMP"))
					{
						existsFlag = 1;
					}

					seqNo = cmd.GetDecimal(2);
				}
				cmd.Close();

				PrintLog("1.existsFlag", existsFlag);
				PrintLog("1.seqNo", seqNo);

				if (tgctp03.GetColValString("DEFAULT_SHOW_STATE").GetLength() > 0 && tgctp03.GetColValString("DEFAULT_SHOW_STATE").Substring(0, 1) == "1")
				{
					PrintLog("有条件字段", tgctp03.GetColValString("COLUMN_NAME"));
					if (existsFlag == 0)
					{
						PrintLog("补充条件字段");

						tgctp02.MergeFrom(tgctp03.GetDataRow());

						for (int k = 0; k < bcls_rec->Tables["CONDITION_ITEM_CONFIG"].Rows.get_Count(); k++)
						{
							CString colName = bcls_rec->Tables["CONDITION_ITEM_CONFIG"].Rows[k]["ITEM_ENAME"].ToString();
							PrintLog("Grid配置列", colName);

							if (bcls_rec->Tables["CONDITION_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim() != "" && tgctp02.GetColValString(colName).Trim() == "")
							{
								PrintLog("DEFAULT_VALUE", bcls_rec->Tables["CONDITION_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim());
								tgctp02.SetColVal(colName, bcls_rec->Tables["CONDITION_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim());
							}
						}

						tgctp02.SetColVal("CFGITM_GRP_NAME", "CONDITION_ITEM");
						tgctp02.SetColVal("SEQ_NO_01", seqNo + 1);
						tgctp02.SetColVal("MAIN_FLAG", "0");

						tgctp02.CopyColVal("LABEL_TEXT", "COLUMN_CNAME");

						if (tgctp02.GetColValString("DATA_TYPE") == "N")
						{
							tgctp02.SetColVal("CONTROL_CLASS", "05");
							tgctp02.SetColVal("OPERATOR", "4");
						}
						else
						{
							if (tgctp02.GetColValString("DATA_TYPE") == "O" || tgctp02.GetColValString("DATA_TYPE") == "H" ||
								tgctp02.GetColValString("DATA_TYPE") == "Z")
							{
								tgctp02.SetColVal("CONTROL_CLASS", "02");
								tgctp02.SetColVal("OPERATOR", "0");
							}
							else if (tgctp02.GetColValString("DATA_TYPE") == "B")
							{
								tgctp02.SetColVal("CONTROL_CLASS", "09");
								tgctp02.SetColVal("OPERATOR", "0");
							}
							else if (tgctp02.GetColValString("DATA_TYPE") == "D")
							{
								tgctp02.SetColVal("CONTROL_CLASS", "06");
								tgctp02.SetColVal("OPERATOR", "4");
							}
							else
							{
								tgctp02.SetColVal("CONTROL_CLASS", "01");
								tgctp02.SetColVal("OPERATOR", "0");
							}
						}

						//tgctp02.Print();
						if (tgctp02.Insert() < 0)
						{
							throw CApplicationException(-1, s.msg, s.svc_name);
						}
					}
				}
				else if (existsFlag == 1)
				{
					PrintLog("删除原条件字段", tgctp03.GetColValString("COLUMN_NAME"));
					
					sqlstr = "DELETE FROM tgctp02 WHERE cfgitm_name = '" + dsName + "' AND cfgitm_grp_name = 'CONDITION_ITEM' AND timestamp = '" +
						tgctp03.GetColValString("TIMESTAMP") + "'";
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}

				//Grid字段判断
				existsFlag = 0;
				seqNo = 0;

				sqlstr = "SELECT timestamp,seq_no_02 FROM tgctp02 WHERE cfgitm_name = '" + dsName + "' AND cfgitm_grp_name = 'GRID_ITEM' ORDER BY seq_no_02";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				while (cmd.Read())
				{
					if (cmd.GetString(1) == tgctp03.GetColValString("TIMESTAMP"))
					{
						existsFlag = 1;
					}

					seqNo = cmd.GetDecimal(2);
				}
				cmd.Close();

				PrintLog("2.existsFlag", existsFlag);
				PrintLog("2.seqNo", seqNo);

				if (tgctp03.GetColValString("DEFAULT_SHOW_STATE").GetLength() > 1 && tgctp03.GetColValString("DEFAULT_SHOW_STATE").Substring(1, 1) == "1")
				{
					PrintLog("有Grid字段", tgctp03.GetColValString("COLUMN_NAME"));
					if (existsFlag == 0)
					{
						PrintLog("补充Grid字段");
						tgctp02.MergeFrom(tgctp03.GetDataRow());

						for (int k = 0; k < bcls_rec->Tables["GRID_ITEM_CONFIG"].Rows.get_Count(); k++)
						{
							CString colName = bcls_rec->Tables["GRID_ITEM_CONFIG"].Rows[k]["ITEM_ENAME"].ToString();
							PrintLog("Grid配置列", colName);

							if (bcls_rec->Tables["GRID_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim() != "" && tgctp02.GetColValString(colName).Trim() == "")
							{
								PrintLog("DEFAULT_VALUE", bcls_rec->Tables["GRID_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim());
								tgctp02.SetColVal(colName, bcls_rec->Tables["GRID_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim());
							}
						}

						tgctp02.SetColVal("CFGITM_GRP_NAME", "GRID_ITEM");
						tgctp02.SetColVal("MAIN_FLAG", "0");

						//tgctp02.Print();
						if (tgctp02.Insert() < 0)
						{
							throw CApplicationException(-1, s.msg, s.svc_name);
						}
					}
				}
				else if (existsFlag == 1)
				{
					PrintLog("删除原GRID字段", tgctp03.GetColValString("COLUMN_NAME"));
					
					sqlstr = "DELETE FROM tgctp02 WHERE cfgitm_name = '" + dsName + "' AND cfgitm_grp_name = 'GRID_ITEM' AND timestamp = '" + 
						tgctp03.GetColValString("TIMESTAMP") + "'";
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}

				//控件字段判断
				existsFlag = 0;
				seqNo = 0;

				sqlstr = "SELECT timestamp,seq_no_03 FROM tgctp02 WHERE cfgitm_name = '" + dsName + "' AND cfgitm_grp_name = 'CONTROL_ITEM' ORDER BY seq_no_03";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				while (cmd.Read())
				{
					if (cmd.GetString(1) == tgctp03.GetColValString("TIMESTAMP"))
					{
						existsFlag = 1;
					}

					seqNo = cmd.GetDecimal(2);
				}
				cmd.Close();

				PrintLog("3.existsFlag", existsFlag);
				PrintLog("3.seqNo", seqNo);

				if (tgctp03.GetColValString("DEFAULT_SHOW_STATE").GetLength() > 2 && tgctp03.GetColValString("DEFAULT_SHOW_STATE").Substring(2, 1) == "1")
				{
					PrintLog("有控件字段", tgctp03.GetColValString("COLUMN_NAME"));

					if (existsFlag == 0)
					{
						PrintLog("补充控件字段");
						tgctp02.MergeFrom(tgctp03.GetDataRow());

						for (int k = 0; k < bcls_rec->Tables["CONTROL_ITEM_CONFIG"].Rows.get_Count(); k++)
						{
							CString colName = bcls_rec->Tables["CONTROL_ITEM_CONFIG"].Rows[k]["ITEM_ENAME"].ToString();
							PrintLog("Grid配置列", colName);

							if (bcls_rec->Tables["CONTROL_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim() != "" && tgctp02.GetColValString(colName).Trim() == "")
							{
								PrintLog("DEFAULT_VALUE", bcls_rec->Tables["CONTROL_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim());
								tgctp02.SetColVal(colName, bcls_rec->Tables["CONTROL_ITEM_CONFIG"].Rows[k]["DEFAULT_VALUE"].ToString().Trim());
							}
						}

						tgctp02.SetColVal("CFGITM_GRP_NAME", "CONTROL_ITEM");
						tgctp02.SetColVal("MAIN_FLAG", "0");

						tgctp02.CopyColVal("LABEL_TEXT", "COLUMN_CNAME");

						if (tgctp02.GetColValString("DATA_TYPE") == "N")
						{
							tgctp02.SetColVal("CONTROL_CLASS", "05");
						}
						else
						{
							if (tgctp02.GetColValString("DATA_TYPE") == "O" || tgctp02.GetColValString("DATA_TYPE") == "H" ||
								tgctp02.GetColValString("DATA_TYPE") == "Z")
							{
								tgctp02.SetColVal("CONTROL_CLASS", "02");
							}
							else if (tgctp02.GetColValString("DATA_TYPE") == "B")
							{
								tgctp02.SetColVal("CONTROL_CLASS", "09");
							}
							else if (tgctp02.GetColValString("DATA_TYPE") == "D")
							{
								tgctp02.SetColVal("CONTROL_CLASS", "06");
							}
							else
							{
								tgctp02.SetColVal("CONTROL_CLASS", "01");
							}
						}

						//tgctp02.Print();
						if (tgctp02.Insert() < 0)
						{
							throw CApplicationException(-1, s.msg, s.svc_name);
						}
					}
				}
				else if (existsFlag == 1)
				{
					PrintLog("删除原控件字段", tgctp03.GetColValString("COLUMN_NAME"));
					
					sqlstr = "DELETE FROM tgctp02 WHERE cfgitm_name = '" + dsName + "' AND cfgitm_grp_name = 'CONTROL_ITEM' AND timestamp = '" +
						tgctp03.GetColValString("TIMESTAMP") + "'";
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
			}
		}

		if (bcls_rec->Tables.Contains("DELETE_BLOCK") && bcls_rec->Tables["DELETE_BLOCK"].Rows.get_Count() > 0)
		{
			for (int i = 0; i < bcls_rec->Tables["DELETE_BLOCK"].Rows.get_Count(); i++)
			{
				tgctp03.MergeFrom(bcls_rec->Tables["DELETE_BLOCK"].Rows[i]);
				if (tgctp03.Delete() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				tgctp02.SetFilterColVal("TIMESTAMP", tgctp03.GetColValString("TIMESTAMP"));
				if (tgctp02.Delete() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
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


