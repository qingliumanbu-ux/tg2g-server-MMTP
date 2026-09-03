#include "CDynaTable2.h"

BM2F_ENTERACE(gctp_copyFromConfig)

BM2_FUNCTION_IMPORT
int f_gctp_getConfigData(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);

BM2_FUNCTION_IMPORT
int f_gctp_buttonConfig(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn, CString formNo);

BM2_FUNCTION_IMPORT
int f_gctp_setFromConfig(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);

BM2_FUNCTION_IMPORT
int f_gctp_configDataSave(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);

BM2_FUNCTION_IMPORT
int f_gctp_configDataNew(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);

BM2_FUNCTION_EXPORT
int f_gctp_copyFromConfig(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString wholeBacklogCodeSrc = "";
	CString wholeBacklogCode = "";
	CString wholeBacklogName = "";
	CString matLineType = "";
	CString formNo = "";
	CString formName = "";
	CString formNoSrc = "";
	CString formNoPop = "";
	CString copyFlag = "";
	CString cfgitmName = "";
	CString keyOld = "";
	CString keyNew = "";
	CString prodTableName = "";
	CString planTableName = "TPSCRA2";

	EIClass bcls_ret_bt;

	//动态数据表定义
	CDynaTable2 tgctp06("TGCTP06", conn);
	CDynaTable2 tgctp07("TGCTP07", conn);

	//数据库操作类定义
	CDbCommand cmd(conn);

	try
	{
		//0:普通复制; 1:事件复制; 2:实绩画面复制; 3:导入
		copyFlag = bcls_rec->Tables[0].Rows[0]["COPY_FLAG"].ToString().Trim();
		PrintLog("copyFlag", copyFlag);

		if (copyFlag == "3")
		{
			formNo = bcls_rec->Tables[0].Rows[0]["FORM_NO"].ToString().Trim();
			PrintLog("formNo", formNo);
		}
		else
		{
			formNo = bcls_rec->Tables[0].Rows[0]["FORM_NO"].ToString().Trim();
			formName = GetColValueC(bcls_rec->Tables[0], 0, "FORM_NAME").Trim();
			formNoSrc = GetColValueC(bcls_rec->Tables[0], 0, "FORM_NO_SRC").Trim();
			formNoPop = GetColValueC(bcls_rec->Tables[0], 0, "FORM_NO_POP").Trim();
			//wholeBacklogCodeSrc = GetColValueC(bcls_rec->Tables[0], 0, "WHOLE_BACKLOG_CODE_SRC").Trim();
			//wholeBacklogCode = GetColValueC(bcls_rec->Tables[0], 0, "WHOLE_BACKLOG_CODE").Trim();
			//wholeBacklogName = GetColValueC(bcls_rec->Tables[0], 0, "WHOLE_BACKLOG_NAME").Trim();
			//prodTableName = GetColValueC(bcls_rec->Tables[0], 0, "PROD_TABLE_NAME").Trim();

			PrintLog("formNo", formNo);
			PrintLog("formName", formName);
			PrintLog("formNoSrc", formNoSrc);
		}

		if (copyFlag == "1")
		{
			sqlstr = "SELECT cfgitm_name FROM tgctp04 WHERE form_no = '" + formNo + "'";
			PrintLog("sqlstr", sqlstr);
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteReader();
			if (cmd.Read())
			{
				cfgitmName = cmd.GetString(1);
			}
			cmd.Close();
		}
		else
		{
			sqlstr = "DELETE FROM tgctp04 WHERE form_no = '" + formNo + "'";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tgctp05 WHERE form_no = '" + formNo + "'";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tgctp06 WHERE form_no = '" + formNo + "'";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tgctp07 WHERE form_no = '" + formNo + "'";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			if (copyFlag != "3")
			{
				bcls_rec->Tables.Add("FORM");
				sqlstr = "SELECT * FROM tgctp04 WHERE form_no = '" + formNoSrc + "'";
				PrintLog("sqlstr", sqlstr);
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteQuery(bcls_rec->Tables["FORM"]);
				cmd.Close();
			}

			if (copyFlag == "2")
			{
				//校验画面名是否正确
				if (formNo.GetLength() < 6 && formNo.Substring(0, 2) != "MM")
				{
					strcpy(s.msg, "复制的实绩画面名不正确");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (formNo.Find("B1") > 0)
				{
					CString strForm = "T" + formNo.Replace("B1", "");
					sqlstr = "SELECT whole_backlog_code,prod_table_name FROM tmm00si16 WHERE prod_table_name = '" + strForm + "' ORDER BY unit_code";
				}
				else
				{
					sqlstr = "SELECT whole_backlog_code,prod_table_name FROM tmm00si16 WHERE form_code =  '" + formNo + "'";
				}

				PrintLog("sqlstr", sqlstr);
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					wholeBacklogCode = cmd.GetString(1);
					prodTableName = cmd.GetString(2);
					keyNew = prodTableName.Substring(1);
				}
				else
				{
					keyNew = formNo.Substring(0, 6);
				}
				cmd.Close();

				PrintLog("keyNew", keyNew);

				keyOld = formNoSrc.Substring(0, 6);
				PrintLog("keyOld", keyOld);

				doFlag = f_gctp_getConfigData(bcls_rec, bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				bcls_rec->Tables.Add("DS_MAIN");
				bcls_rec->Tables["DS_MAIN"].Clone(bcls_ret->Tables["DS_MAIN"]);

				bcls_rec->Tables.Add("DS_ITEM");
				bcls_rec->Tables["DS_ITEM"].Clone(bcls_ret->Tables["DS_ITEM"]);

				if (bcls_ret->Tables.Contains("CONDITION_MAIN"))
				{
					bcls_rec->Tables.Add("CONDITION_MAIN");
					bcls_rec->Tables["CONDITION_MAIN"].Clone(bcls_ret->Tables["CONDITION_MAIN"]);
				}

				if (bcls_ret->Tables.Contains("CONDITION_ITEM"))
				{
					bcls_rec->Tables.Add("CONDITION_ITEM");
					bcls_rec->Tables["CONDITION_ITEM"].Clone(bcls_ret->Tables["CONDITION_ITEM"]);
				}

				if (bcls_ret->Tables.Contains("GRID_MAIN"))
				{
					bcls_rec->Tables.Add("GRID_MAIN");
					bcls_rec->Tables["GRID_MAIN"].Clone(bcls_ret->Tables["GRID_MAIN"]);
				}

				if (bcls_ret->Tables.Contains("GRID_ITEM"))
				{
					bcls_rec->Tables.Add("GRID_ITEM");
					bcls_rec->Tables["GRID_ITEM"].Clone(bcls_ret->Tables["GRID_ITEM"]);
				}

				if (bcls_ret->Tables.Contains("CONTROL_MAIN"))
				{
					bcls_rec->Tables.Add("CONTROL_MAIN");
					bcls_rec->Tables["CONTROL_MAIN"].Clone(bcls_ret->Tables["CONTROL_MAIN"]);
				}

				if (bcls_ret->Tables.Contains("CONTROL_ITEM"))
				{
					bcls_rec->Tables.Add("CONTROL_ITEM");
					bcls_rec->Tables["CONTROL_ITEM"].Clone(bcls_ret->Tables["CONTROL_ITEM"]);
				}
			}

			if (copyFlag != "3")
			{
				for (int i = 0; i < bcls_rec->Tables["FORM"].Rows.get_Count(); i++)
				{
					bcls_rec->Tables["FORM"].Rows[i]["FORM_NO"] = formNo;
					bcls_rec->Tables["FORM"].Rows[i]["FORM_NAME"] = formName;
					bcls_rec->Tables["FORM"].Rows[i]["FUNCTION_ID"] = "";

					if (copyFlag == "2")
					{
						CString cfgitmOld = bcls_rec->Tables["FORM"].Rows[i]["CFGITM_NAME"].ToString();
						PrintLog("cfgitmOld", cfgitmOld);

						if (cfgitmOld.Find("B1") > 0)
						{
							cfgitmName = keyNew + "B1";
						}
						else
						{
							CString strRelace = StringSplit(cfgitmOld, "_", 1);
							if (strRelace.Trim() != "")
							{
								cfgitmName = cfgitmOld.Replace(strRelace, keyNew);
							}
							else
							{
								cfgitmName = keyNew + "_" + "NEW";
							}
						}

						PrintLog("cfgitmName", cfgitmName);
						bcls_rec->Tables["FORM"].Rows[i]["CFGITM_NAME"] = cfgitmName;

						for (int j = 0; j < bcls_ret->Tables["DS_MAIN"].Rows.get_Count(); j++)
						{
							if (bcls_ret->Tables["DS_MAIN"].Rows[j]["CFGITM_NAME"].ToString() == cfgitmOld)
							{
								bcls_rec->Tables["DS_MAIN"].Rows.Add();
								bcls_rec->Tables["DS_MAIN"].Rows[bcls_rec->Tables["DS_MAIN"].Rows.get_Count() - 1].Merge(bcls_ret->Tables["DS_MAIN"].Rows[j]);
								bcls_rec->Tables["DS_MAIN"].Rows[bcls_rec->Tables["DS_MAIN"].Rows.get_Count() - 1]["CFGITM_NAME"] = cfgitmName;

								if (cfgitmName.Find("_PLAN") > 0)
								{
									bcls_rec->Tables["DS_MAIN"].Rows[bcls_rec->Tables["DS_MAIN"].Rows.get_Count() - 1]["TABLE_NAME"] = planTableName;
								}
								else if (cfgitmName.Find("_PROD") > 0 || cfgitmName.Find("B1") > 0)
								{
									bcls_rec->Tables["DS_MAIN"].Rows[bcls_rec->Tables["DS_MAIN"].Rows.get_Count() - 1]["TABLE_NAME"] = prodTableName;
								}
							}
						}

						for (int j = 0; j < bcls_ret->Tables["DS_ITEM"].Rows.get_Count(); j++)
						{
							if (bcls_ret->Tables["DS_ITEM"].Rows[j]["CFGITM_NAME"].ToString() == cfgitmOld)
							{
								bcls_rec->Tables["DS_ITEM"].Rows.Add();
								bcls_rec->Tables["DS_ITEM"].Rows[bcls_rec->Tables["DS_ITEM"].Rows.get_Count() - 1].Merge(bcls_ret->Tables["DS_ITEM"].Rows[j]);
								bcls_rec->Tables["DS_ITEM"].Rows[bcls_rec->Tables["DS_ITEM"].Rows.get_Count() - 1]["CFGITM_NAME"] = cfgitmName;

								if (cfgitmName.Find("_PLAN") > 0)
								{
									bcls_rec->Tables["DS_MAIN"].Rows[bcls_rec->Tables["DS_MAIN"].Rows.get_Count() - 1]["TABLE_NAME"] = planTableName;
								}
								else if (cfgitmName.Find("_PROD") > 0 || cfgitmName.Find("B1") > 0)
								{
									bcls_rec->Tables["DS_ITEM"].Rows[bcls_rec->Tables["DS_ITEM"].Rows.get_Count() - 1]["TABLE_NAME"] = prodTableName;
								}
							}
						}

						if (bcls_ret->Tables.Contains("CONDITION_MAIN"))
						{
							for (int j = 0; j < bcls_ret->Tables["CONDITION_MAIN"].Rows.get_Count(); j++)
							{
								if (bcls_ret->Tables["CONDITION_MAIN"].Rows[j]["CFGITM_NAME"].ToString() == cfgitmOld)
								{
									bcls_rec->Tables["CONDITION_MAIN"].Rows.Add();
									bcls_rec->Tables["CONDITION_MAIN"].Rows[bcls_rec->Tables["CONDITION_MAIN"].Rows.get_Count() - 1].Merge(bcls_ret->Tables["CONDITION_MAIN"].Rows[j]);
									bcls_rec->Tables["CONDITION_MAIN"].Rows[bcls_rec->Tables["CONDITION_MAIN"].Rows.get_Count() - 1]["CFGITM_NAME"] = cfgitmName;
								}
							}
						}

						if (bcls_ret->Tables.Contains("CONDITION_ITEM"))
						{
							for (int j = 0; j < bcls_ret->Tables["CONDITION_ITEM"].Rows.get_Count(); j++)
							{
								if (bcls_ret->Tables["CONDITION_ITEM"].Rows[j]["CFGITM_NAME"].ToString() == cfgitmOld)
								{
									bcls_rec->Tables["CONDITION_ITEM"].Rows.Add();
									bcls_rec->Tables["CONDITION_ITEM"].Rows[bcls_rec->Tables["CONDITION_ITEM"].Rows.get_Count() - 1].Merge(bcls_ret->Tables["CONDITION_ITEM"].Rows[j]);
									bcls_rec->Tables["CONDITION_ITEM"].Rows[bcls_rec->Tables["CONDITION_ITEM"].Rows.get_Count() - 1]["CFGITM_NAME"] = cfgitmName;

									if (cfgitmName.Find("_PLAN") > 0)
									{
										bcls_rec->Tables["DS_MAIN"].Rows[bcls_rec->Tables["DS_MAIN"].Rows.get_Count() - 1]["TABLE_NAME"] = planTableName;
									}
									else if (cfgitmName.Find("_PROD") > 0 || cfgitmName.Find("B1") > 0)
									{
										bcls_rec->Tables["CONDITION_ITEM"].Rows[bcls_rec->Tables["CONDITION_ITEM"].Rows.get_Count() - 1]["TABLE_NAME"] = prodTableName;
									}
								}
							}
						}

						if (bcls_ret->Tables.Contains("GRID_MAIN"))
						{
							for (int j = 0; j < bcls_ret->Tables["GRID_MAIN"].Rows.get_Count(); j++)
							{
								if (bcls_ret->Tables["GRID_MAIN"].Rows[j]["CFGITM_NAME"].ToString() == cfgitmOld)
								{
									bcls_rec->Tables["GRID_MAIN"].Rows.Add();
									bcls_rec->Tables["GRID_MAIN"].Rows[bcls_rec->Tables["GRID_MAIN"].Rows.get_Count() - 1].Merge(bcls_ret->Tables["GRID_MAIN"].Rows[j]);
									bcls_rec->Tables["GRID_MAIN"].Rows[bcls_rec->Tables["GRID_MAIN"].Rows.get_Count() - 1]["CFGITM_NAME"] = cfgitmName;
								}
							}
						}

						if (bcls_ret->Tables.Contains("GRID_ITEM"))
						{
							for (int j = 0; j < bcls_ret->Tables["GRID_ITEM"].Rows.get_Count(); j++)
							{
								if (bcls_ret->Tables["GRID_ITEM"].Rows[j]["CFGITM_NAME"].ToString() == cfgitmOld)
								{
									bcls_rec->Tables["GRID_ITEM"].Rows.Add();
									bcls_rec->Tables["GRID_ITEM"].Rows[bcls_rec->Tables["GRID_ITEM"].Rows.get_Count() - 1].Merge(bcls_ret->Tables["GRID_ITEM"].Rows[j]);
									bcls_rec->Tables["GRID_ITEM"].Rows[bcls_rec->Tables["GRID_ITEM"].Rows.get_Count() - 1]["CFGITM_NAME"] = cfgitmName;

									if (cfgitmName.Find("_PLAN") > 0)
									{
										bcls_rec->Tables["DS_MAIN"].Rows[bcls_rec->Tables["DS_MAIN"].Rows.get_Count() - 1]["TABLE_NAME"] = planTableName;
									}
									else if (cfgitmName.Find("_PROD") > 0 || cfgitmName.Find("B1") > 0)
									{
										bcls_rec->Tables["GRID_ITEM"].Rows[bcls_rec->Tables["GRID_ITEM"].Rows.get_Count() - 1]["TABLE_NAME"] = prodTableName;
									}
								}
							}
						}

						if (bcls_ret->Tables.Contains("CONTROL_MAIN"))
						{
							for (int j = 0; j < bcls_ret->Tables["CONTROL_MAIN"].Rows.get_Count(); j++)
							{
								if (bcls_ret->Tables["CONTROL_MAIN"].Rows[j]["CFGITM_NAME"].ToString() == cfgitmOld)
								{
									bcls_rec->Tables["CONTROL_MAIN"].Rows.Add();
									bcls_rec->Tables["CONTROL_MAIN"].Rows[bcls_rec->Tables["CONTROL_MAIN"].Rows.get_Count() - 1].Merge(bcls_ret->Tables["CONTROL_MAIN"].Rows[j]);
									bcls_rec->Tables["CONTROL_MAIN"].Rows[bcls_rec->Tables["CONTROL_MAIN"].Rows.get_Count() - 1]["CFGITM_NAME"] = cfgitmName;
								}
							}
						}

						if (bcls_ret->Tables.Contains("CONTROL_ITEM"))
						{
							for (int j = 0; j < bcls_ret->Tables["CONTROL_ITEM"].Rows.get_Count(); j++)
							{
								if (bcls_ret->Tables["CONTROL_ITEM"].Rows[j]["CFGITM_NAME"].ToString() == cfgitmOld)
								{
									bcls_rec->Tables["CONTROL_ITEM"].Rows.Add();
									bcls_rec->Tables["CONTROL_ITEM"].Rows[bcls_rec->Tables["CONTROL_ITEM"].Rows.get_Count() - 1].Merge(bcls_ret->Tables["CONTROL_ITEM"].Rows[j]);
									bcls_rec->Tables["CONTROL_ITEM"].Rows[bcls_rec->Tables["CONTROL_ITEM"].Rows.get_Count() - 1]["CFGITM_NAME"] = cfgitmName;

									if (cfgitmName.Find("_PLAN") > 0)
									{
										bcls_rec->Tables["DS_MAIN"].Rows[bcls_rec->Tables["DS_MAIN"].Rows.get_Count() - 1]["TABLE_NAME"] = planTableName;
									}
									else if (cfgitmName.Find("_PROD") > 0 || cfgitmName.Find("B1") > 0)
									{
										bcls_rec->Tables["CONTROL_ITEM"].Rows[bcls_rec->Tables["CONTROL_ITEM"].Rows.get_Count() - 1]["TABLE_NAME"] = prodTableName;
									}
								}
							}
						}
					}
				}
			}
		}

		if (copyFlag == "2" || copyFlag == "3")
		{
			//PrintDataTable(bcls_rec->Tables["DS_MAIN"]);
			//PrintDataTable(bcls_rec->Tables["DS_ITEM"]);
			//PrintDataTable(bcls_rec->Tables["GRID_ITEM"]);

			doFlag = f_gctp_configDataNew(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		if (copyFlag == "3")
		{
			if (bcls_rec->Tables.Contains("CUSTOM_EVENT"))
			{
				bcls_ret_bt.Tables.Add(bcls_rec->Tables["CUSTOM_EVENT"]);
			}

			if (bcls_rec->Tables.Contains("OPERATE_DO"))
			{
				CDataTable dtDo;
				dtDo.Copy(bcls_rec->Tables["OPERATE_DO"]);

				bcls_ret_bt.Tables.Add(dtDo);
				bcls_rec->Tables["OPERATE_DO"].Rows.Clear();
			}

			if (bcls_rec->Tables.Contains("OPERATE"))
			{
				CDataTable dtIo;
				dtIo.Copy(bcls_rec->Tables["OPERATE"]);

				bcls_ret_bt.Tables.Add(dtIo);
				bcls_rec->Tables["OPERATE"].Rows.Clear();
			}
		}
		else
		{
			//查询按钮配置数据
			doFlag = f_gctp_buttonConfig(bcls_rec, &bcls_ret_bt, conn, formNoSrc);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			bcls_rec->Tables.Add("CUSTOM_EVENT");
			bcls_rec->Tables.Add("OPERATE_DO");

			if (bcls_ret_bt.Tables.Contains("OPERATE_DO"))
			{
				bcls_rec->Tables["OPERATE_DO"].Clone(bcls_ret_bt.Tables["OPERATE_DO"]);
				bcls_rec->Tables.Add("OPERATE");
			}
		}

		if (bcls_ret_bt.Tables.Contains("CUSTOM_EVENT") && bcls_ret_bt.Tables["CUSTOM_EVENT"].Rows.get_Count() > 0)
		{
			PrintLog("原画面有事件");
			for (int i = 0; i < bcls_ret_bt.Tables["CUSTOM_EVENT"].Rows.get_Count(); i++)
			{
				tgctp07.MergeFrom(bcls_ret_bt.Tables["CUSTOM_EVENT"].Rows[i], i);
				tgctp07.SetColVal("FORM_NO", formNo, i);
				tgctp07.SetColVal("ID", 2000000000 + GetSeqence("GCTP_EVENT_SEQ_NO", conn), i);

				if (bcls_ret_bt.Tables.Contains("OPERATE_DO") && bcls_ret_bt.Tables["OPERATE_DO"].Rows.get_Count() > 0)
				{
					for (int j = 0; j < bcls_ret_bt.Tables["OPERATE_DO"].Rows.get_Count(); j++)
					{
						if (bcls_ret_bt.Tables["OPERATE_DO"].Rows[j]["ID"].ToDecimal() == bcls_ret_bt.Tables["CUSTOM_EVENT"].Rows[i]["ID"].ToDecimal())
						{
							PrintLog("j", j);
							PrintLog("复制原事件ID对应的操作", bcls_ret_bt.Tables["OPERATE_DO"].Rows[j]["ID"].ToDecimal());
							PrintLog("NOW_ROW", bcls_ret_bt.Tables["OPERATE_DO"].Rows[j]["NOW_ROW"].ToString());

							bcls_rec->Tables["OPERATE_DO"].Rows.Add();
							CDataRow& drDo = bcls_rec->Tables["OPERATE_DO"].Rows[bcls_rec->Tables["OPERATE_DO"].Rows.get_Count() - 1];
							drDo.Merge(bcls_ret_bt.Tables["OPERATE_DO"].Rows[j]);
							drDo["FORM_NO"] = formNo;
							drDo["ID"] = tgctp07.GetColValDecimal("ID", i);
							drDo["NOW_ROW"] = GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn);
							drDo["OPERATE_PARTITION"] = GetColValueC(bcls_rec->Tables[0], 0, "ABBREV");

							CString dsNameNew = "";

							if (copyFlag == "1")
							{
								drDo["DATASET_NAME"] = cfgitmName;
							}
							else if (copyFlag == "2")
							{
								//keyOld = drDo["DATASET_NAME"].ToString().Substring(0, 6);
								//PrintLog("keyOld", keyOld);

								dsNameNew = drDo["DATASET_NAME"].ToString().Replace(keyOld, keyNew);
								PrintLog("dsNameNew", dsNameNew);

								drDo["DATASET_NAME"] = dsNameNew;
							}

							if (bcls_ret_bt.Tables.Contains("OPERATE") && bcls_ret_bt.Tables["OPERATE"].Rows.get_Count() > 0)
							{
								for (int k = 0; k < bcls_ret_bt.Tables["OPERATE"].Rows.get_Count(); k++)
								{
									if (bcls_ret_bt.Tables["OPERATE"].Rows[k]["NOW_ROW"].ToString() == bcls_ret_bt.Tables["OPERATE_DO"].Rows[j]["NOW_ROW"].ToString())
									{
										PrintLog("k", k);
										PrintLog("复制原事件ID对应的输入输出", bcls_ret_bt.Tables["OPERATE"].Rows[k]["ID"].ToDecimal());
										tgctp06.MergeFrom(bcls_ret_bt.Tables["OPERATE"].Rows[k]);
										tgctp06.SetColVal("FORM_NO", formNo);
										tgctp06.SetColVal("ID", tgctp07.GetColValDecimal("ID", i));
										tgctp06.SetColVal("NOW_ROW", drDo["NOW_ROW"].ToString());

										if (copyFlag == "1")
										{
											tgctp06.SetColVal("CFGITM_NAME", cfgitmName);
										}
										else if (copyFlag == "2")
										{
											tgctp06.SetColVal("CFGITM_NAME", dsNameNew);
										}

										tgctp06.MergeTo(bcls_rec->Tables["OPERATE"], 0, true, true);
									}
								}
							}
						}
					}
				}
			}
		}

		tgctp07.CopyTo(bcls_rec->Tables["CUSTOM_EVENT"]);

		if (bcls_ret_bt.Tables.Contains("OPERATE_DO") && bcls_ret_bt.Tables["OPERATE_DO"].Rows.get_Count() > 0)
		{
			for (int i = 0; i < bcls_ret_bt.Tables["OPERATE_DO"].Rows.get_Count(); i++)
			{
				if (bcls_ret_bt.Tables["OPERATE_DO"].Rows[i]["NAME"].ToString().Trim() != "")
				{
					PrintLog("i", i);
					PrintLog("复制原按钮对应的操作", bcls_ret_bt.Tables["OPERATE_DO"].Rows[i]["NAME"].ToString());
					PrintLog("NOW_ROW", bcls_ret_bt.Tables["OPERATE_DO"].Rows[i]["NOW_ROW"].ToString());

					bcls_rec->Tables["OPERATE_DO"].Rows.Add();
					CDataRow& drDo = bcls_rec->Tables["OPERATE_DO"].Rows[bcls_rec->Tables["OPERATE_DO"].Rows.get_Count() - 1];
					drDo.Merge(bcls_ret_bt.Tables["OPERATE_DO"].Rows[i]);
					drDo["FORM_NO"] = formNo;
					drDo["ID"] = 1000000000 + GetSeqence("GCTP_BUTTON_SEQ_NO", conn);
					drDo["NOW_ROW"] = GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn);
					drDo["OPERATE_PARTITION"] = GetColValueC(bcls_rec->Tables[0], 0, "ABBREV");

					CString dsNameNew = "";

					if (copyFlag == "1")
					{
						drDo["DATASET_NAME"] = cfgitmName;
					}
					else if (copyFlag == "2")
					{
						//keyOld = drDo["DATASET_NAME"].ToString().Substring(0, 6);
						//PrintLog("keyOld", keyOld);

						dsNameNew = drDo["DATASET_NAME"].ToString().Replace(keyOld, keyNew);
						PrintLog("dsNameNew", dsNameNew);

						drDo["DATASET_NAME"] = dsNameNew;
						drDo["CALL_FORM_NO"] = drDo["CALL_FORM_NO"].ToString().Replace(keyOld, keyNew);

						PrintLog("1.CALL_SERVICE", drDo["CALL_SERVICE"].ToString());
						drDo["CALL_SERVICE"] = drDo["CALL_SERVICE"].ToString().Replace(keyOld.ToLower(), keyNew.ToLower());
						PrintLog("2.CALL_SERVICE", drDo["CALL_SERVICE"].ToString());
					}

					if (bcls_ret_bt.Tables.Contains("OPERATE") && bcls_ret_bt.Tables["OPERATE"].Rows.get_Count() > 0)
					{
						for (int j = 0; j < bcls_ret_bt.Tables["OPERATE"].Rows.get_Count(); j++)
						{
							if (bcls_ret_bt.Tables["OPERATE"].Rows[j]["NOW_ROW"].ToString() == bcls_ret_bt.Tables["OPERATE_DO"].Rows[i]["NOW_ROW"].ToString())
							{
								PrintLog("j", j);
								PrintLog("复制原按钮对应的输入输出", bcls_ret_bt.Tables["OPERATE"].Rows[j]["NOW_ROW"].ToString());
								tgctp06.MergeFrom(bcls_ret_bt.Tables["OPERATE"].Rows[j]);
								tgctp06.SetColVal("FORM_NO", formNo);
								tgctp06.Print("FORM_NO");

								tgctp06.SetColVal("ID", drDo["ID"].ToString());
								tgctp06.SetColVal("NOW_ROW", drDo["NOW_ROW"].ToString());

								if (copyFlag == "1")
								{
									tgctp06.SetColVal("CFGITM_NAME", cfgitmName);
								}
								else if (copyFlag == "2")
								{
									tgctp06.SetColVal("CFGITM_NAME", dsNameNew);
								}

								tgctp06.MergeTo(bcls_rec->Tables["OPERATE"], 0, true, true);
							}
						}
					}
				}
			}
		}

		//PrintDataTable(bcls_rec->Tables["OPERATE_DO"]);
		//PrintDataTable(bcls_rec->Tables["OPERATE"]);

		AddColValue(bcls_rec->Tables[0], 0, "NEW_FLAG", "1");
		doFlag = f_gctp_setFromConfig(bcls_rec, bcls_ret, conn);
		if (doFlag < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
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