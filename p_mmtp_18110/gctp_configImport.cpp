#include "CDynaTable.h"

BM2F_ENTERACE(gctp_configImport)

BM2_FUNCTION_IMPORT
int f_gctp_formInfoInsert(CString formNo, CString formName, CString formPartition, CString appName, CDbConnection * conn);

BM2_FUNCTION_IMPORT
int f_gctp_getConfigItem(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CString cfgitmGrpName, CString custFlag);

BM2_FUNCTION_EXPORT
int f_gctp_configImport(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CString formPartition = "";
	CString appName = "";
	CDecimal dAclid = 0;
	int iRowNo = 0;

	CDynaTable tgctp01("TGCTP01", conn);
	CDynaTable tgctp02("TGCTP02", conn);
	CDynaTable tgctp04("TGCTP04", conn);
	CDynaTable tgctp05("TGCTP05", conn);
	CDynaTable tgctp06("TGCTP06", conn);
	CDynaTable tgctp07("TGCTP07", conn);

	CDbCommand cmd(conn);
	CDbCommand cmd_ins(conn);
	CDbCommand cmd_upd(conn);

	try
	{
		PrintDataTable(bcls_rec->Tables[0]);

		if (bcls_rec->Tables.Contains("CFGITM_NAME"))
		{
			CString sqlstr1 = "DELETE FROM tgctp01 WHERE cfgitm_name IN (";
			CString sqlstr2 = "DELETE FROM tgctp02 WHERE cfgitm_name IN (";

			for (int i = 0; i < bcls_rec->Tables["CFGITM_NAME"].Rows.get_Count(); i++)
			{
				if (i > 0)
				{
					sqlstr1 += ",";
					sqlstr2 += ",";
				}

				sqlstr1 += "'" + bcls_rec->Tables["CFGITM_NAME"].Rows[i]["CFGITM_NAME"].ToString() + "'";
				sqlstr2 += "'" + bcls_rec->Tables["CFGITM_NAME"].Rows[i]["CFGITM_NAME"].ToString() + "'";
			}
			
			sqlstr1 += ")";
			PrintLog("sqlstr1", sqlstr1);
			cmd.SetCommandText(sqlstr1);
			cmd.ExecuteNonQuery();
			cmd.Close();
			
			sqlstr2 += ")";
			PrintLog("sqlstr2", sqlstr2);
			cmd.SetCommandText(sqlstr2);
			cmd.ExecuteNonQuery();
			cmd.Close();
		}

		if (bcls_rec->Tables.Contains("FORM_NO"))
		{
			formPartition = GetColValueC(bcls_rec->Tables[0], 0, "FORM_PARTITION");
			PrintLog("formPartition", formPartition);

			appName = GetColValueC(bcls_rec->Tables[0], 0, "APP_NAME");
			PrintLog("appName", appName);

			for (int i = 0; i < bcls_rec->Tables["FORM_NO"].Rows.get_Count(); i++)
			{
				CString formNo = bcls_rec->Tables["FORM_NO"].Rows[i]["FORM_NO"].ToString();
				CString formName = bcls_rec->Tables["FORM_NO"].Rows[i]["FORM_NAME"].ToString();

				sqlstr = "DELETE FROM tgctp04 WHERE form_no = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp05 WHERE form_no = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp06 WHERE form_no = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp07 WHERE form_no = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "SELECT count(1) FROM tesformresinfo WHERE name = '" + formNo + "'";
				cmd.SetCommandText(sqlstr);
				PrintLog("sqlstr", sqlstr);
				if (cmd.ExecuteScalar() == 0)
				{
					PrintLog("f_gctp_formInfoInsert");
					doFlag = f_gctp_formInfoInsert(formNo, formName, formPartition, appName, conn);
					if (doFlag < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					PrintLog("f_gctp_formInfoInsert成功");
				}
				cmd.Close();
			}

			for (int i = 0; i < bcls_rec->Tables["TGCTP07"].Rows.get_Count(); i++)
			{
				CString buttonType = bcls_rec->Tables["TGCTP07"].Rows[i]["OPERATE_TYPE"].ToString();
				if (buttonType == "01")
				{
					buttonType = "A";
				}
				else if (buttonType == "02")
				{
					buttonType = "B";
				}
				else
				{
					continue;
				}

				CString idImport = bcls_rec->Tables["TGCTP07"].Rows[i]["ID"].ToString();
				CString formNo = bcls_rec->Tables["TGCTP07"].Rows[i]["FORM_NO"].ToString();
				CString buttonName = bcls_rec->Tables["TGCTP07"].Rows[i]["NAME"].ToString();
				CString buttonDesc = bcls_rec->Tables["TGCTP07"].Rows[i]["DESCRIPTION"].ToString();

				sqlstr = "SELECT aclid,optype,description FROM tesbuttonresinfo WHERE fname = '" + formNo + "' AND name = '" + buttonName + "'";
				cmd.SetCommandText(sqlstr);
				PrintLog("sqlstr", sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					dAclid = cmd.GetDecimal(1);
					PrintLog("dAclid", dAclid);

					if (cmd.GetString(2) != buttonType || cmd.GetString(3) != buttonDesc)
					{
						cmd_upd.Parameters.Set("buttonType", buttonType);
						cmd_upd.Parameters.Set("buttonDesc", buttonDesc);
						cmd_upd.Parameters.Set("dAclid", dAclid);

						if (cmd.GetString(3) == buttonDesc)
						{
							PrintLog("更新按钮类型");
							sqlstr = "UPDATE tesbuttonresinfo SET optype = @buttonType WHERE aclid = @dAclid";
							cmd_upd.SetCommandText(sqlstr);
							cmd_upd.ExecuteNonQuery();
							cmd_upd.Close();
						}
						else
						{
							PrintLog("更新按钮类型和描述");
							sqlstr = "UPDATE tesbuttonresinfo SET optype = @buttonType,description = @buttonDesc WHERE aclid = @dAclid";
							cmd_upd.SetCommandText(sqlstr);
							cmd_upd.ExecuteNonQuery();
							cmd_upd.Close();

							sqlstr = "UPDATE tesbuttonresinfo_res SET description = @buttonDesc WHERE aclid = @dAclid AND culture = 'zh_Hans'";
							cmd_upd.SetCommandText(sqlstr);
							cmd_upd.ExecuteNonQuery();
							cmd_upd.Close();
						}
					}
				}
				else
				{
					dAclid = 1000000000 + GetSeqence("MMTP_BUTTON_SEQ_NO", conn);
					PrintLog("新增按钮dAclid", dAclid);

					sqlstr = "INSERT INTO tesbuttonresinfo VALUES('" + buttonName + "','" + formNo + "'," + dAclid.ToString() + ",'" +
						buttonDesc + "','" + buttonType + "','" + appName + "','" + (CString)s.userid + "',' ')";
					PrintLog("sqlstr", sqlstr);
					cmd_ins.SetCommandText(sqlstr);
					cmd_ins.ExecuteNonQuery();
					cmd_ins.Close();

					sqlstr = "INSERT INTO tesbuttonresinfo_res VALUES('zh_Hans'," + dAclid.ToString() + ", '" + buttonDesc + "')";
					PrintLog("sqlstr", sqlstr);
					cmd_ins.SetCommandText(sqlstr);
					cmd_ins.ExecuteNonQuery();
					cmd_ins.Close();
				}
				cmd.Close();

				for (int j = 0; j < bcls_rec->Tables["TGCTP05"].Rows.get_Count(); j++)
				{
					if (bcls_rec->Tables["TGCTP05"].Rows[j]["ID"].ToString() == idImport)
					{
						bcls_rec->Tables["TGCTP05"].Rows[j]["ID"] = dAclid.ToString();
					}
				}

				for (int j = 0; j < bcls_rec->Tables["TGCTP06"].Rows.get_Count(); j++)
				{
					if (bcls_rec->Tables["TGCTP06"].Rows[j]["ID"].ToString() == idImport)
					{
						bcls_rec->Tables["TGCTP06"].Rows[j]["ID"] = dAclid.ToString();
					}
				}

				bcls_rec->Tables["TGCTP07"].Rows[i]["ID"] = dAclid.ToString();
			}
		}
		else if (bcls_rec->Tables.Contains("CFGITM_NAME"))
		{
			for (int i = 0; i < bcls_rec->Tables["CFGITM_NAME"].Rows.get_Count(); i++)
			{
				sqlstr = "DELETE FROM tgctp01 WHERE cfgitm_name = '" + bcls_rec->Tables["CFGITM_NAME"].Rows[i]["CFGITM_NAME"].ToString() + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);

				sqlstr = "DELETE FROM tgctp02 WHERE cfgitm_name = '" + bcls_rec->Tables["CFGITM_NAME"].Rows[i]["CFGITM_NAME"].ToString() + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("sqlstr", sqlstr);
			}
		}
		else
		{
			strcpy(s.msg, "导入数据不正确");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		for (int i = 0; i < bcls_rec->Tables["TGCTP01"].Rows.get_Count(); i++)
		{
			bcls_rec->Tables["TGCTP01"].Rows[i]["NOW_ROW"] = CDateTime::Now().ToString("yyyyMMddHHmmss") +
				bcls_rec->Tables["TGCTP01"].Rows[i]["NOW_ROW"].ToString().Substring(14, 6);

			for (int j = 0; j < bcls_rec->Tables["TGCTP01"].Columns.get_Count(); j++)
			{
				if (bcls_rec->Tables["TGCTP01"].Columns[j].get_DataType() == DT_STRING)
				{
					bcls_rec->Tables["TGCTP01"].Rows[i][j] = bcls_rec->Tables["TGCTP01"].Rows[i][j].ToString().Replace("'", "''");
				}
			}
		}

		tgctp01.CopyFrom(bcls_rec->Tables["TGCTP01"]);
		//tgctp01.Print();
		if (tgctp01.Insert() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		for (int i = 0; i < bcls_rec->Tables["TGCTP02"].Columns.get_Count(); i++)
		{
			if (bcls_rec->Tables["TGCTP02"].Columns[i].get_DataType() == DT_STRING)
			{
				for (int j = 0; j < bcls_rec->Tables["TGCTP02"].Rows.get_Count(); j++)
				{
					bcls_rec->Tables["TGCTP02"].Rows[j][i] = bcls_rec->Tables["TGCTP02"].Rows[j][i].ToString().Replace("'", "''");
				}
			}
		}
		tgctp02.CopyFrom(bcls_rec->Tables["TGCTP02"]);
		if (tgctp02.Insert() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		tgctp04.CopyFrom(bcls_rec->Tables["TGCTP04"]);
		if (tgctp04.Insert() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		tgctp05.CopyFrom(bcls_rec->Tables["TGCTP05"]);
		if (tgctp05.Insert() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		tgctp06.CopyFrom(bcls_rec->Tables["TGCTP06"]);
		if (tgctp06.Insert() < 0)
		{
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		tgctp07.CopyFrom(bcls_rec->Tables["TGCTP07"]);
		if (tgctp07.Insert() < 0)
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