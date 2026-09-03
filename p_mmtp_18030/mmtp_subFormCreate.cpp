#include "CDynaTable.h"

BM2F_ENTERACE(mmtp_subFormCreate)
int f_mmtp_subFormCreate(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//变量定义
	int doFlag = 0;
	CString dataTime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString sqlstr = " ";
	CString sqlstr1 = " ";
	CString sqlstr2 = " ";
	CString sqlInit = " ";
	CString cfgName = " ";
	CString cfgNameCommon = " ";
	CString matLineType = " ";
	CString wholeBacklogCode = " ";
	CString unitCode = " ";
	CString prodTableName = " ";

	CDbCommand cmd(conn);

	try
	{
		CTracer log(__FUNCTION__);
		CDynaTable tmmtp01("TMMTP01", conn);
		CDynaTable tmmtp02("TMMTP02", conn);
		CDynaTable tmmtp03("TMMTP03", conn);

		for (int i = 0; i<bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			sqlstr = "form_name = '" + GetColValueC(bcls_rec->Tables[0], i, "FORM_NAME") + "'";
			if (QueryDataCount("TESFORMPARA", sqlstr, conn) > 0)
			{
				if (!TableDataDelete("TESFORMPARA", sqlstr, conn))
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}

			sqlstr = "INSERT INTO TESFORMPARA (REC_CREATOR,REC_CREATE_TIME,FORM_NAME,FORM_BASE_NAME,PK1,PK1_NAME";

			if (GetColValueC(bcls_rec->Tables[0], i, "PK2").Trim() != "")
			{
				sqlstr += ",PK2,PK2_NAME";
			}

			if (GetColValueC(bcls_rec->Tables[0], i, "PK3").Trim() != "")
			{
				sqlstr += ",PK3,PK3_NAME";
			}

			if (GetColValueC(bcls_rec->Tables[0], i, "PK4").Trim() != "")
			{
				sqlstr += ",PK4,PK4_NAME";
			}

			if (GetColValueC(bcls_rec->Tables[0], i, "PK5").Trim() != "")
			{
				sqlstr += ",PK5,PK5_NAME";
			}

			sqlstr += ") VALUES ('" + (CString)s.userid + "','" + (CString)s.datetime + "','" + GetColValueC(bcls_rec->Tables[0], i, "FORM_NAME") +
				"','" + GetColValueC(bcls_rec->Tables[0], i, "FORM_BASE_NAME") + "','" + GetColValueC(bcls_rec->Tables[0], i, "PK1") +
				"','" + GetColValueC(bcls_rec->Tables[0], i, "PK1_NAME") + "'";

			if (GetColValueC(bcls_rec->Tables[0], i, "PK2").Trim() != "")
			{
				sqlstr += ",'" + GetColValueC(bcls_rec->Tables[0], i, "PK2") + "','" + GetColValueC(bcls_rec->Tables[0], i, "PK2_NAME") + "'";
			}

			if (GetColValueC(bcls_rec->Tables[0], i, "PK3").Trim() != "")
			{
				sqlstr += ",'" + GetColValueC(bcls_rec->Tables[0], i, "PK3") + "','" + GetColValueC(bcls_rec->Tables[0], i, "PK3_NAME") + "'";
			}

			if (GetColValueC(bcls_rec->Tables[0], i, "PK4").Trim() != "")
			{
				sqlstr += ",'" + GetColValueC(bcls_rec->Tables[0], i, "PK4") + "','" + GetColValueC(bcls_rec->Tables[0], i, "PK4_NAME") + "'";
			}

			if (GetColValueC(bcls_rec->Tables[0], i, "PK5").Trim() != "")
			{
				sqlstr += ",'" + GetColValueC(bcls_rec->Tables[0], i, "PK5") + "','" + GetColValueC(bcls_rec->Tables[0], i, "PK5_NAME") + "'";
			}

			sqlstr += ")";
			PrintLog("sqlstr", sqlstr);
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			PrintLog("FORM_BASE_NAME", GetColValueC(bcls_rec->Tables[0], i, "FORM_BASE_NAME"));
			PrintLog("PROD_TABLE_NAME", GetColValueC(bcls_rec->Tables[0], i, "PROD_TABLE_NAME"));
			if (GetColValueC(bcls_rec->Tables[0], i, "FORM_BASE_NAME") == "MMPROD3" &&
				GetColValueC(bcls_rec->Tables[0], i, "PROD_TABLE_NAME").Trim() != "")
			{
				cfgName = GetColValueC(bcls_rec->Tables[0], i, "FORM_NAME");
				prodTableName = GetColValueC(bcls_rec->Tables[0], i, "PROD_TABLE_NAME");
				unitCode = GetColValueC(bcls_rec->Tables[0], i, "PK3");
				wholeBacklogCode = GetColValueC(bcls_rec->Tables[0], i, "PK2");
				matLineType = GetColValueC(bcls_rec->Tables[0], i, "PK1");

				PrintLog("cfgName", cfgName);
				PrintLog("prodTableName", prodTableName);
				PrintLog("unitCode", unitCode);
				PrintLog("wholeBacklogCode", wholeBacklogCode);
				PrintLog("matLineType", matLineType);

				cfgNameCommon = "MM" + matLineType + "PROD";
				sqlInit = "SELECT '" + (CString)s.userid + "','" + dataTime + "',' ',' ',' ',' ',' ',' ','" + cfgName + "'";

				tmmtp03.SetFilterColVal("CFGITM_NAME", cfgName);
				if (tmmtp03.QueryCount() == 0)
				{
					sqlstr1 = "INSERT INTO tmmtp01 " + sqlInit;
					sqlstr2 = "INSERT INTO tmmtp01 " + sqlInit;
					for (int j = 9; j < tmmtp01.GetDataTable().Columns.get_Count(); j++)
					{
						sqlstr1 += ",";
						sqlstr2 += ",";

						if (tmmtp01.GetDataTable().Columns[j].get_ColumnName() == "TABLE_NAME")
						{
							sqlstr1 += "'" + prodTableName + "'";
						}
						else
						{
							sqlstr1 += tmmtp01.GetDataTable().Columns[j].get_ColumnName().ToLower();
						}

						sqlstr2 += tmmtp01.GetDataTable().Columns[j].get_ColumnName().ToLower();
					}

					sqlstr1 += " FROM tmmtp01 WHERE cfgitm_name = '" + cfgNameCommon + "' AND cfggrp_name IN ('PROD','ENTRY','BASIC')";
					sqlstr = sqlstr1;
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();

					sqlstr2 += " FROM tmmtp01 WHERE cfgitm_name = '" + cfgNameCommon + "' AND cfggrp_name NOT IN ('PROD','ENTRY','BASIC')";
					sqlstr = sqlstr2;
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();

					sqlstr1 = "INSERT INTO tmmtp02 " + sqlInit;
					sqlstr2 = "INSERT INTO tmmtp02 " + sqlInit;
					for (int j = 9; j < tmmtp02.GetDataTable().Columns.get_Count(); j++)
					{
						sqlstr1 += ",";
						sqlstr2 += ",";

						if (tmmtp02.GetDataTable().Columns[j].get_ColumnName() == "TABLE_NAME")
						{
							sqlstr1 += "'" + prodTableName + "'";
						}
						else
						{
							sqlstr1 += tmmtp02.GetDataTable().Columns[j].get_ColumnName().ToLower();
						}

						sqlstr2 += tmmtp02.GetDataTable().Columns[j].get_ColumnName().ToLower();
					}

					sqlstr1 += " FROM tmmtp02 WHERE cfgitm_name = '" + cfgNameCommon + "' AND cfggrp_name IN ('PROD','ENTRY','BASIC')";
					sqlstr = sqlstr1;
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();

					sqlstr2 += " FROM tmmtp02 WHERE cfgitm_name = '" + cfgNameCommon + "' AND cfggrp_name NOT IN ('PROD','ENTRY','BASIC')";
					sqlstr = sqlstr2;
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();

					sqlstr1 = "INSERT INTO tmmtp03 " + sqlInit;
					sqlstr2 = "INSERT INTO tmmtp03 " + sqlInit;
					for (int j = 9; j < tmmtp03.GetDataTable().Columns.get_Count(); j++)
					{
						sqlstr1 += ",";
						sqlstr2 += ",";
						if (tmmtp03.GetDataTable().Columns[j].get_ColumnName() == "TABLE_NAME")
						{
							sqlstr1 += "'" + prodTableName + "'";
						}
						else
						{
							sqlstr1 += tmmtp03.GetDataTable().Columns[j].get_ColumnName().ToLower();
						}

						sqlstr2 += tmmtp03.GetDataTable().Columns[j].get_ColumnName().ToLower();
					}

					sqlstr1 += " FROM tmmtp03 WHERE cfgitm_name = '" + cfgNameCommon + "' AND cfggrp_name IN ('PROD','ENTRY','BASIC')";
					sqlstr = sqlstr1;
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();

					sqlstr2 += " FROM tmmtp03 WHERE cfgitm_name = '" + cfgNameCommon + "' AND cfggrp_name NOT IN ('PROD','ENTRY','BASIC')";
					sqlstr = sqlstr2;
					PrintLog("sqlstr", sqlstr);
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
			}
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


