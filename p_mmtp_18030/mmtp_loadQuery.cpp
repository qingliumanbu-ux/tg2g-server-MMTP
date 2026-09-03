#include "CUtils.h"

BM2F_ENTERACE(mmtp_loadQuery)
int f_mmtp_loadQuery(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	//程序内部变量
	int blkNum = 0;
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	int rowNum = 0;
	int multiGrpFlag = 0;
	CDecimal queryFlag = 0;

	CString configName = "";
	CString congrpName = "";
	CString tableName = "";
	CString tableName1 = "";
	CString tableName2 = "";
	CString tableNameC = "";
	CString sqlWhere = "";
	CString sqlWhere1 = "";
	CString sqlWhere2 = "";
	CString sqlWhere3 = "";
	CString codeClass = "";
	CString funcId = "";
	CString prodDiv = "";
	CString prodTableName = "";

	CDataTable dtCode;
	CDataTable dtConfigAdd1;
	CDataTable dtConfigAdd2;
	CDataTable dtConfigAdd3;
	CDataTable dtConfigAdd5;

	CDbCommand cmd_inq(conn);

	try
	{
		CTracer log(__FUNCTION__);

		//QUERY_FLAG: 1.配置画面查询配置; 2.查数据字典; 3.生产实绩画面载入; 4.配置名查询; 5.查询虚拟表
		queryFlag = GetColValueD(bcls_rec->Tables[0], 0, "QUERY_FLAG");
		PrintLog("QUERY_FLAG", queryFlag);

		if (queryFlag == 4)
		{
			sqlstr = "SELECT DISTINCT cfgitm_name FROM tmmtp03";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();

			bcls_ret->Tables[0].set_TableName("CFGITM_NAME");
			return doFlag;
		}

		if (queryFlag == 2 || queryFlag == 5)
		{
			tableName = GetColValueC(bcls_rec->Tables[0], 0, "TABLE_NAME");
			PrintLog("TABLE_NAME", tableName);

			if (tableName.Trim() == "")
			{
				strcpy(s.msg, "没有传入数据表名");
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}
		else if (queryFlag == 3)
		{
			PrintLog("查询机组配置数据");

			//CString backlogTypeCode = GetColValueC(bcls_rec->Tables[0], 0, "PS_BACKLOG_TYPE_CODE");
			//PrintLog("backlogTypeCode", backlogTypeCode);

			//CString wholeBacklogCode = GetColValueC(bcls_rec->Tables[0], 0, "WHOLE_BACKLOG_CODE");
			//PrintLog("wholeBacklogCode", wholeBacklogCode);

			//CString unitCode = GetColValueC(bcls_rec->Tables[0], 0, "UNIT_CODE");
			//PrintLog("unitCode", unitCode);

			//CString matLineType = GetColValueC(bcls_rec->Tables[0], 0, "MAT_LINE_TYPE");
			//PrintLog("matLineType", matLineType);

			//CString dummyCoilFlag = GetColValueC(bcls_rec->Tables[0], 0, "DUMMY_COIL_FLAG");
			//PrintLog("dummyCoilFlag", dummyCoilFlag);

			//int commonFlag = 1;

			//if (unitCode.Trim() != "")
			//{
			//	sqlWhere += " AND unit_code = '" + unitCode + "'";
			//	commonFlag = 0;
			//}

			//if (backlogTypeCode.Trim() != "")
			//{
			//	sqlWhere += " AND ps_backlog_type_code = '" + backlogTypeCode + "'";
			//	commonFlag = 0;
			//}

			//if (wholeBacklogCode.Trim() != "")
			//{
			//	sqlWhere += " AND whole_backlog_code = '" + wholeBacklogCode + "'";
			//	commonFlag = 0;
			//}

			//if (matLineType.Trim() != "")
			//{
			//	sqlWhere += " AND mat_line_type = '" + matLineType + "'";
			//}

			configName = GetColValueC(bcls_rec->Tables[0], 0, "CFGITM_NAME");
			PrintLog("configName", configName);
			sqlstr = "SELECT * FROM tmm00si16 WHERE form_no = '" + configName + "' ORDER BY unit_code";
			PrintLog("sqlstr", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
			bcls_ret->Tables[0].set_TableName("PROD_UNIT");

			if (bcls_ret->Tables[0].Rows.get_Count() == 0)
			{
				strcpy(s.msg, "没有机组配置数据");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//matLineType = bcls_ret->Tables["PROD_UNIT"].Rows[0]["MAT_LINE_TYPE"].ToString();
			//backlogTypeCode = bcls_ret->Tables["PROD_UNIT"].Rows[0]["PS_BACKLOG_TYPE_CODE"].ToString();

			prodTableName = bcls_ret->Tables["PROD_UNIT"].Rows[0]["PROD_TABLE_NAME"].ToString();
			PrintLog("prodTableName", prodTableName);

			//PrintLog("commonFlag", commonFlag);
			//if (commonFlag == 0)
			//{
			//	prodTableName = GetColValueC(bcls_ret->Tables[0], 0, "PROD_TABLE_NAME");
			//	configName = GetColValueC(bcls_ret->Tables[0], 0, "FORM_NO");
			//}
			//else
			//{
			//	prodTableName = "TMM" + matLineType + "20";

			//	if (dummyCoilFlag == "1")
			//	{
			//		configName = "MM" + matLineType + "PROD_DUMMY";
			//	}
			//	else
			//	{
			//		configName = "MM" + matLineType + "PROD";
			//	}
			//}

			bcls_ret->Tables.Add("PROD_DATA");
			SetDataTableColName(prodTableName, bcls_ret->Tables["PROD_DATA"], conn);

			bcls_ret->Tables.Add("ACT_TYPE");
			sqlstr = "SELECT code,code_desc_1_content FROM tep0002 WHERE code_class = 'MC01' ORDER BY code";
			PrintLog("sqlstr", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables["ACT_TYPE"]);
			cmd_inq.Close();

			if (GetColValueC(bcls_ret->Tables["PROD_UNIT"], 0, "FURNACE_TYPE").Trim() != "")
			{
				sqlstr = "SELECT code,code_desc_1_content FROM tep0002 WHERE code_class = '" +
					GetColValueC(bcls_ret->Tables["PROD_UNIT"], 0, "FURNACE_TYPE").Trim() + "' ORDER BY code";
				PrintLog("sqlstr", sqlstr);
				cmd_inq.SetCommandText(sqlstr);
				bcls_ret->Tables.Add("EQU_NO");
				cmd_inq.ExecuteQuery(bcls_ret->Tables["EQU_NO"]);
				cmd_inq.Close();

				PrintDataTable(bcls_ret->Tables["EQU_NO"]);
			}

			sqlWhere2 = "code_class IN ('M007','M008','M031','MC12','M041','MP4A','M09M'";
		}
		else
		{
			configName = GetColValueC(bcls_rec->Tables[0], 0, "CFGITM_NAME");
			congrpName = GetColValueC(bcls_rec->Tables[0], 0, "CFGGRP_NAME");

			PrintLog("CFGITM_NAME", configName);
			PrintLog("CFGGRP_NAME", congrpName);

			if (configName.Trim() == "")
			{
				strcpy(s.msg, "没有传入配置名");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			sqlWhere1 = GetColValueC(bcls_rec->Tables[0], 0, "SQL_WHERE");
		}

		if (queryFlag != 2 && queryFlag != 5)
		{
			sqlstr = "SELECT * FROM tmmtp01 WHERE cfgitm_name = '" + configName + "'";
			if (congrpName.Trim() != "")
			{
				sqlstr += " AND cfggrp_name = '" + congrpName + "'";
			}
			sqlstr += " ORDER BY cfggrp_name, seq_no";

			PrintLog("sqlstr", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			bcls_ret->Tables.Add("TMMTP01");
			cmd_inq.ExecuteQuery(bcls_ret->Tables["TMMTP01"]);
			cmd_inq.Close();

			bcls_ret->Tables["TMMTP01"].Columns.Add(DT_INT32, "SEQ_NO_INT");
			bcls_ret->Tables["TMMTP01"].Columns.Add(DT_INT32, "ROW_SEQ_INT");
			bcls_ret->Tables["TMMTP01"].Columns.Add(DT_INT32, "COL_SEQ_INT");
			bcls_ret->Tables["TMMTP01"].Columns.Add(DT_INT32, "SPACE_X_INT");
			for (int i = 0; i < bcls_ret->Tables["TMMTP01"].Rows.get_Count(); i++)
			{
				bcls_ret->Tables["TMMTP01"].Rows[i]["SEQ_NO_INT"] = bcls_ret->Tables["TMMTP01"].Rows[i]["SEQ_NO"];
				bcls_ret->Tables["TMMTP01"].Rows[i]["ROW_SEQ_INT"] = bcls_ret->Tables["TMMTP01"].Rows[i]["ROW_SEQ"];
				bcls_ret->Tables["TMMTP01"].Rows[i]["COL_SEQ_INT"] = bcls_ret->Tables["TMMTP01"].Rows[i]["COL_SEQ"];
				bcls_ret->Tables["TMMTP01"].Rows[i]["SPACE_X_INT"] = bcls_ret->Tables["TMMTP01"].Rows[i]["SPACE_X"];
			}

			sqlstr = "SELECT * FROM tmmtp02 WHERE cfgitm_name = '" + configName + "'";
			if (congrpName.Trim() != "")
			{
				sqlstr += " AND cfggrp_name = '" + congrpName + "'";
			}
			sqlstr += " ORDER BY cfggrp_name, seq_no";

			PrintLog("sqlstr", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			bcls_ret->Tables.Add("TMMTP02");
			cmd_inq.ExecuteQuery(bcls_ret->Tables["TMMTP02"]);
			cmd_inq.Close();

			bcls_ret->Tables["TMMTP02"].Columns.Add(DT_INT32, "SEQ_NO_INT");
			bcls_ret->Tables["TMMTP02"].Columns.Add(DT_INT32, "ROW_SEQ_INT");
			bcls_ret->Tables["TMMTP02"].Columns.Add(DT_INT32, "COL_SEQ_INT");
			bcls_ret->Tables["TMMTP02"].Columns.Add(DT_INT32, "SPACE_X_INT");
			for (int i = 0; i < bcls_ret->Tables["TMMTP02"].Rows.get_Count(); i++)
			{
				bcls_ret->Tables["TMMTP02"].Rows[i]["SEQ_NO_INT"] = bcls_ret->Tables["TMMTP02"].Rows[i]["SEQ_NO"];
				bcls_ret->Tables["TMMTP02"].Rows[i]["ROW_SEQ_INT"] = bcls_ret->Tables["TMMTP02"].Rows[i]["ROW_SEQ"];
				bcls_ret->Tables["TMMTP02"].Rows[i]["COL_SEQ_INT"] = bcls_ret->Tables["TMMTP02"].Rows[i]["COL_SEQ"];
				bcls_ret->Tables["TMMTP02"].Rows[i]["SPACE_X_INT"] = bcls_ret->Tables["TMMTP02"].Rows[i]["SPACE_X"];
			}

			sqlstr = "SELECT * FROM tmmtp03 WHERE cfgitm_name = '" + configName + "'";
			if (congrpName.Trim() != "")
			{
				sqlstr += " AND cfggrp_name = '" + congrpName + "'";
			}
			sqlstr += " ORDER BY cfggrp_name";

			PrintLog("sqlstr", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			bcls_ret->Tables.Add("TMMTP03");
			cmd_inq.ExecuteQuery(bcls_ret->Tables["TMMTP03"]);
			cmd_inq.Close();

			if (bcls_ret->Tables["TMMTP03"].Rows.get_Count() > 1)
			{
				multiGrpFlag = 1;
			}

			//PrintDataTable(bcls_ret->Tables["TMMTP03"]);
			if (GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TABLE_TYPE") == "1")
			{
				tableName = GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TABLE_ENAME");
			}
			else
			{
				tableName = GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TABLE_NAME");
			}
			PrintLog("TABLE_NAME", tableName);

			tableName1 = GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TABLE_NAME_1");
			PrintLog("TABLE_NAME1", tableName1);

			tableName2 = GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TABLE_NAME_2");
			PrintLog("TABLE_NAME2", tableName2);

			for (int i = 0; i < bcls_ret->Tables["TMMTP03"].Rows.get_Count(); i++)
			{
				if (GetColValueC(bcls_ret->Tables["TMMTP03"], i, "FUNC_ID").Trim() != "")
				{
					funcId = GetColValueC(bcls_ret->Tables["TMMTP03"], i, "FUNC_ID").Trim();
					PrintLog("funcId", funcId);

					switch (conn->DatabaseKind)
					{
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
						sqlstr = "SELECT MAX(row_count) FROM (SELECT a.func_id,a.class_code,CEILING(b.item_count/a.column_count) row_count"
							" FROM ted53 a,(SELECT func_id, class_code, count(1) AS item_count FROM ted54 WHERE item_hide_flag NOT IN ('1','3')"
							" GROUP BY func_id, class_code) b WHERE a.func_id = b.func_id AND a.class_code = b.class_code) WHERE func_id = '" +
							funcId + "' GROUP BY FUNC_ID";
						break;
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default:
						sqlstr = "SELECT MAX(row_count) FROM (SELECT a.func_id,a.class_code,CEIL(b.item_count/a.column_count) row_count"
							" FROM ted53 a,(SELECT func_id, class_code, count(1) AS item_count FROM ted54 WHERE item_hide_flag NOT IN ('1','3')"
							" GROUP BY func_id, class_code) b WHERE a.func_id = b.func_id AND a.class_code = b.class_code) WHERE func_id = '" +
							funcId + "' GROUP BY FUNC_ID";
						break;
					}

					CDbCommand cmd_inq(conn);
					cmd_inq.SetCommandText(sqlstr);
					CDecimal rowCount = cmd_inq.ExecuteScalar();

					PrintLog("i", i);
					PrintLog("CFGGRP_NAME", bcls_ret->Tables["TMMTP03"].Rows[i]["CFGGRP_NAME"].ToString());
					PrintLog("rowCount", rowCount);
					AddColValue(bcls_ret->Tables["TMMTP03"], i, "ROW_NUM", rowCount);
				}
			}

			if (queryFlag == 1)
			{
				sqlstr = "SELECT * FROM tmmtp04 WHERE cfgitm_name = '" + configName + "' ORDER BY seq_no";
				cmd_inq.SetCommandText(sqlstr);
				bcls_ret->Tables.Add("TMMTP04");
				cmd_inq.ExecuteQuery(bcls_ret->Tables["TMMTP04"]);
				cmd_inq.Close();
			}

			sqlstr = "SELECT * FROM tmmtp05 WHERE cfgitm_name = '" + configName + "'";
			if (congrpName.Trim() != "")
			{
				sqlstr += " AND cfggrp_name = '" + congrpName + "'";
			}
			sqlstr += " ORDER BY operation_type";
			cmd_inq.SetCommandText(sqlstr);
			bcls_ret->Tables.Add("TMMTP05");
			cmd_inq.ExecuteQuery(bcls_ret->Tables["TMMTP05"]);
			cmd_inq.Close();

			if (prodDiv == "0" || prodDiv == "1" || prodDiv == "7")
			{
				for (int i = 0; i < dtConfigAdd1.Rows.get_Count(); i++)
				{
					bcls_ret->Tables["TMMTP01"].Rows.Add();
					bcls_ret->Tables["TMMTP01"].Rows[bcls_ret->Tables["TMMTP01"].Rows.get_Count() - 1].Merge(dtConfigAdd1.Rows[i]);
				}

				for (int i = 0; i < dtConfigAdd2.Rows.get_Count(); i++)
				{
					bcls_ret->Tables["TMMTP02"].Rows.Add();
					bcls_ret->Tables["TMMTP02"].Rows[bcls_ret->Tables["TMMTP02"].Rows.get_Count() - 1].Merge(dtConfigAdd2.Rows[i]);
				}

				for (int i = 0; i < dtConfigAdd3.Rows.get_Count(); i++)
				{
					bcls_ret->Tables["TMMTP03"].Rows.Add();
					bcls_ret->Tables["TMMTP03"].Rows[bcls_ret->Tables["TMMTP03"].Rows.get_Count() - 1].Merge(dtConfigAdd3.Rows[i]);
				}

				for (int i = 0; i < dtConfigAdd5.Rows.get_Count(); i++)
				{
					bcls_ret->Tables["TMMTP05"].Rows.Add();
					bcls_ret->Tables["TMMTP05"].Rows[bcls_ret->Tables["TMMTP05"].Rows.get_Count() - 1].Merge(dtConfigAdd5.Rows[i]);
				}
			}

			if (queryFlag != 1)
			{
				PrintLog("查询条件代码映射");
				PrintLog("TMMTP01 RowsCount", bcls_ret->Tables["TMMTP01"].Rows.get_Count());

				codeClass = "";

				for (int i = 0; i < bcls_ret->Tables["TMMTP01"].Rows.get_Count(); i++)
				{
					CString colName = bcls_ret->Tables["TMMTP01"].Rows[i]["COLUMN_NAME"].ToString().Trim();
					CString blkName = "Q" + bcls_ret->Tables["TMMTP01"].Rows[i]["CFGGRP_NAME"].ToString().Trim() + colName;

					if (bcls_ret->Tables["TMMTP01"].Rows[i]["CODE_CLASS"].ToString().Trim() != "")
					{
						codeClass = bcls_ret->Tables["TMMTP01"].Rows[i]["CODE_CLASS"].ToString().Trim();

						if (sqlWhere2.Trim() == "")
						{
							sqlWhere2 = "code_class IN ('" + codeClass + "'";
						}
						else if (sqlWhere2.Find(codeClass) < 0)
						{
							sqlWhere2 += ",'" + codeClass + "'";
						}

						PrintLog("sqlWhere2", sqlWhere2);
					}
					else if (bcls_ret->Tables["TMMTP01"].Rows[i]["SQL_CONTEXT"].ToString().Trim() != "")
					{
						sqlstr = bcls_ret->Tables["TMMTP01"].Rows[i]["SQL_CONTEXT"].ToString().Trim().ToUpper();

						PrintLog("SQL_CONTEXT", sqlstr);
						PrintLog("colName", colName);
						if (bcls_ret->Tables.IndexOf(blkName) < 0)
						{
							bcls_ret->Tables.Add(blkName);
							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.ExecuteQuery(bcls_ret->Tables[blkName]);

							if (bcls_ret->Tables[blkName].Columns.get_Count() > 1)
							{
								bcls_ret->Tables[blkName].Columns[0].set_ColumnName("CODE");
								bcls_ret->Tables[blkName].Columns[1].set_ColumnName("CODE_DESC_1_CONTENT");
							}
						}
					}
				}

				PrintLog("Grid代码映射");
				PrintLog("TMMTP02 RowsCount", bcls_ret->Tables["TMMTP02"].Rows.get_Count());
				for (int i = 0; i < bcls_ret->Tables["TMMTP02"].Rows.get_Count(); i++)
				{
					codeClass = "";
					CString colName = bcls_ret->Tables["TMMTP02"].Rows[i]["COLUMN_NAME"].ToString().Trim();
					CString blkName = "I" + bcls_ret->Tables["TMMTP02"].Rows[i]["CFGGRP_NAME"].ToString().Trim() + colName;

					if (bcls_ret->Tables["TMMTP02"].Rows[i]["CONTROL_CLASS"].ToString() == "L3" && bcls_ret->Tables.IndexOf(blkName) < 0)
					{
						if (GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TABLE_TYPE") == "1")
						{
							sqlstr = "SELECT DISTINCT item_cvalue FROM " + GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TABLE_NAME") +
								" WHERE table_ename = '" + tableName + "' AND item_ename = '" + colName + "' ORDER BY item_cvalue";
						}
						else
						{
							sqlstr = "SELECT DISTINCT " + colName + " FROM " + tableName + " ORDER BY " + colName;
						}

						cmd_inq.SetCommandText(sqlstr);
						PrintLog("sqlstr", sqlstr);
						bcls_ret->Tables.Add(blkName);
						cmd_inq.ExecuteQuery(bcls_ret->Tables[blkName]);
						cmd_inq.Close();
					}
					else if (bcls_ret->Tables["TMMTP02"].Rows[i]["CODE_CLASS"].ToString().Trim() != "")
					{
						codeClass = bcls_ret->Tables["TMMTP02"].Rows[i]["CODE_CLASS"].ToString().Trim();

						if (sqlWhere2.Trim() == "")
						{
							sqlWhere2 = "code_class IN ('" + codeClass + "'";
						}
						else if (sqlWhere2.Find(codeClass) < 0)
						{
							sqlWhere2 += ",'" + codeClass + "'";
						}
					}
					else if (bcls_ret->Tables["TMMTP02"].Rows[i]["SQL_CONTEXT"].ToString().Trim() != "")
					{
						sqlstr = bcls_ret->Tables["TMMTP02"].Rows[i]["SQL_CONTEXT"].ToString().Trim().ToUpper();
						if (bcls_ret->Tables.IndexOf(blkName) < 0)
						{
							bcls_ret->Tables.Add(blkName);
							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.ExecuteQuery(bcls_ret->Tables[blkName]);
							cmd_inq.Close();

							if (bcls_ret->Tables[blkName].Columns.get_Count() > 1)
							{
								bcls_ret->Tables[blkName].Columns[0].set_ColumnName("CODE");
								bcls_ret->Tables[blkName].Columns[1].set_ColumnName("CODE_DESC_1_CONTENT");
							}
						}
					}
				}
			}
			else
			{
				bcls_ret->Tables.Add("COLUMN_NAME");
				sqlstr = "SELECT DISTINCT cfgitm_name FROM tmmtp03";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteQuery(bcls_ret->Tables["COLUMN_NAME"]);
				cmd_inq.Close();
			}
		}

		if (queryFlag != 3)
		{
			//if (queryFlag > 0 ||
			//	(GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TEMPLATE_TYPE") == "04" ||
			//	GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TEMPLATE_TYPE") == "05"))
			//{

			bcls_ret->Tables[0].set_TableName("TABLE");
			if (congrpName == "MATDATA")
			{
				sqlstr = "SELECT * FROM tmmtp04 WHERE cfgitm_name = '" + configName + "' ORDER BY seq_no";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteQuery(bcls_ret->Tables["TABLE"]);
				cmd_inq.Close();

				//if (bcls_ret->Tables["TABLE"].Rows.get_Count() == 0 && configName.GetLength() > 4)
				//{
				//	sqlstr = "SELECT * FROM tmmtp04 WHERE cfgitm_name = '" + configName.Substring(0, 4) + "PROD' ORDER BY seq_no";
				//	cmd_inq.SetCommandText(sqlstr);
				//	cmd_inq.ExecuteQuery(bcls_ret->Tables["TABLE"]);
				//	cmd_inq.Close();
				//}
			}

			if (bcls_ret->Tables["TABLE"].Rows.get_Count() == 0 && tableName.Trim() != "")
			{
				CString tableType = "0";
				if (queryFlag == 5 ||
					(bcls_ret->Tables.Contains("TMMTP03") && GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TABLE_TYPE") == "1"))
				{
					tableType = "1";
					sqlstr = "SELECT * FROM tmmtp06 WHERE table_ename = '" + tableName.Trim() + "' ORDER BY seq_no";
					PrintLog("sqlstr", sqlstr);

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.ExecuteQuery(bcls_ret->Tables["TABLE"]);
					cmd_inq.Close();

					tableNameC = GetColValueC(bcls_ret->Tables["TABLE"], 0, "TABLE_CNAME");

					bcls_ret->Tables[0].Columns.Add(DT_STRING, "ITEM_NAME");
					for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
					{
						bcls_ret->Tables[0].Rows[i]["ITEM_NAME"] = bcls_ret->Tables[0].Rows[i]["ITEM_ENAME"];
					}
				}
				else
				{
					if (tableName.Substring(0, 1) == "V" && tableName.ToUpper().Substring(0, 2) != "V_")
					{
						tableName = "T" + tableName.Substring(1);
					}
					PrintLog("GetTableColName", tableName);
					bcls_ret->Tables["TABLE"] = GetTableColName(tableName, conn);

					switch (conn->DatabaseKind)
					{
					case DB_KIND_MSSQL:	        // MS SQL Server数据库
						sqlstr = "";
						break;
					case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
					case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
						sqlstr = "SELECT remarks FROM SYSIBM.SYSTABLES WHERE name = '" + tableName +
							"' AND creator = (SELECT current schema FROM sysibm.sysdummy1)";
						break;
					case DB_KIND_ORACLE:	    // Oracle 数据库
					default:
						sqlstr = "SELECT comments FROM user_tab_comments WHERE table_name = '" + tableName + "'";
						break;
					}

					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						tableNameC = cmd_inq.GetString(1);
					}
					cmd_inq.Close();
					PrintLog("tableNameC", tableNameC);
				}

				if (tableName1.Trim() != "")
				{
					if (tableName1.Substring(0, 1) == "V" && tableName1.ToUpper().Substring(0, 2) != "V_")
					{
						tableName1 = "T" + tableName1.Substring(1);
					}
					PrintLog("tableName1", tableName1);

					CDataTable dt = GetTableColName(tableName1, conn);
					for (int i = 0; i < dt.Rows.get_Count(); i++)
					{
						CDecimal existsFlag = 0;
						for (int j = 0; j < bcls_ret->Tables[0].Rows.get_Count(); j++)
						{
							if (bcls_ret->Tables[0].Rows[j]["COLUMN_NAME"].ToString() == dt.Rows[i]["COLUMN_NAME"].ToString())
							{
								existsFlag = 1;
								break;
							}
						}

						if (existsFlag == 0)
						{
							PrintLog("Add Column", dt.Rows[i]["COLUMN_NAME"].ToString());
							bcls_ret->Tables[0].Rows.Add();
							bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1].Merge(dt.Rows[i]);
						}
					}
				}

				if (tableName2.Trim() != "")
				{
					if (tableName2.Substring(0, 1) == "V" && tableName2.ToUpper().Substring(0, 2) != "V_")
					{
						tableName2 = "T" + tableName2.Substring(1);
					}
					PrintLog("tableName2", tableName2);

					CDataTable dt = GetTableColName(tableName2, conn);
					for (int i = 0; i < dt.Rows.get_Count(); i++)
					{
						CDecimal existsFlag = 0;
						for (int j = 0; j < bcls_ret->Tables[0].Rows.get_Count(); j++)
						{
							if (bcls_ret->Tables[0].Rows[j]["COLUMN_NAME"].ToString() == dt.Rows[i]["COLUMN_NAME"].ToString())
							{
								existsFlag = 1;
								break;
							}
						}

						if (existsFlag == 0)
						{
							bcls_ret->Tables[0].Rows.Add();
							bcls_ret->Tables[0].Rows[bcls_ret->Tables[0].Rows.get_Count() - 1].Merge(dt.Rows[i]);
						}
					}
				}

				if (tableType == "0")
				{
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "CFGITM_NAME");
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "ITEM_NAME");
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "ITEM_CNAME");
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "ITEM_KIND");
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "ITEM_LEN");
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "TABLE_NAME");
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "TABLE_CNAME");
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "DISPLAY_FLAG");
					bcls_ret->Tables[0].Columns.Add(DT_STRING, "ORIGIN_CODE");

					for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
					{
						bcls_ret->Tables[0].Rows[i]["ITEM_NAME"] = bcls_ret->Tables[0].Rows[i]["COLUMN_NAME"];
						bcls_ret->Tables[0].Rows[i]["ITEM_CNAME"] = bcls_ret->Tables[0].Rows[i]["COLUMN_CNAME"];
						bcls_ret->Tables[0].Rows[i]["CFGITM_NAME"] = configName;
						bcls_ret->Tables[0].Rows[i]["TABLE_NAME"] = tableName;
						bcls_ret->Tables[0].Rows[i]["TABLE_CNAME"] = tableNameC;
						bcls_ret->Tables[0].Rows[i]["COLUMN_NAME"] = "";
						bcls_ret->Tables[0].Rows[i]["ORIGIN_CODE"] = "0";
						bcls_ret->Tables[0].Rows[i]["ITEM_KIND"] = bcls_ret->Tables[0].Rows[i]["DATA_TYPE"];
						bcls_ret->Tables[0].Rows[i]["ITEM_LEN"] = bcls_ret->Tables[0].Rows[i]["DATA_LENGTH"];

						if (bcls_ret->Tables[0].Rows[i]["DATA_TYPE"].ToString() == "N" &&
							bcls_ret->Tables[0].Rows[i]["DATA_SCALE"].ToDecimal() > 0)
						{
							bcls_ret->Tables[0].Rows[i]["ITEM_LEN"] = bcls_ret->Tables[0].Rows[i]["DATA_PRECISION"].ToString() +
								"," + bcls_ret->Tables[0].Rows[i]["DATA_SCALE"].ToString();
						}

						if (bcls_ret->Tables.get_Count() > 2 && (
							GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TEMPLATE_TYPE") == "04" ||
							GetColValueC(bcls_ret->Tables["TMMTP03"], 0, "TEMPLATE_TYPE") == "05"))
						{
							bcls_ret->Tables[0].Rows[i]["DISPLAY_FLAG"] = "0";
							for (int j = 0; j < bcls_ret->Tables[2].Rows.get_Count(); j++)
							{
								if (bcls_ret->Tables[0].Rows[i]["ITEM_NAME"].ToString() == bcls_ret->Tables[2].Rows[j]["COLUMN_NAME"].ToString())
								{
									bcls_ret->Tables[0].Rows[i]["DISPLAY_FLAG"] = "1";
									break;
								}
							}
						}
					}
				}
			}
		}

		PrintLog("Tables Count", bcls_rec->Tables.get_Count());

		if (bcls_rec->Tables.Contains("CODE_SQL") && bcls_rec->Tables["CODE_SQL"].Rows.get_Count() > 0)
		{
			PrintLog("111");
			codeClass = "";
			for (int i = 0; i < bcls_rec->Tables["CODE_SQL"].Rows.get_Count(); i++)
			{
				codeClass = bcls_rec->Tables["CODE_SQL"].Rows[i]["CODE_CLASS"].ToString().Trim();
				if (codeClass != "")
				{
					if (sqlWhere3.Trim() == "")
					{
						sqlWhere3 = "code_class IN ('" + codeClass + "'";
					}
					else if (sqlWhere3.Find(codeClass) < 0)
					{
						sqlWhere3 += ",'" + codeClass + "'";
					}
				}
				PrintLog("sqlWhere3", sqlWhere3);

				sqlstr = bcls_rec->Tables["CODE_SQL"].Rows[i]["SQL_CONTEXT"].ToString().Trim().ToUpper();
				if (sqlstr != "")
				{
					PrintLog("SQL_CONTEXT", sqlstr);

					CString colName = bcls_rec->Tables["CODE_SQL"].Rows[i]["COLUMN_NAME"].ToString().Trim();
					PrintLog("colName", colName);

					if (bcls_ret->Tables.IndexOf(colName) < 0)
					{
						bcls_ret->Tables.Add(colName);
						blkNum = bcls_ret->Tables.IndexOf(colName);
						PrintLog("blkNum", blkNum);

						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.ExecuteQuery(bcls_ret->Tables[blkNum]);
						cmd_inq.Close();

						if (bcls_ret->Tables[blkNum].Columns.get_Count() > 1)
						{
							bcls_ret->Tables[blkNum].Columns[0].set_ColumnName("CODE");
							bcls_ret->Tables[blkNum].Columns[1].set_ColumnName("CODE_DESC_1_CONTENT");
						}
					}
				}
			}
		}

		sqlWhere = "";
		if (sqlWhere1.Trim() != "" || sqlWhere2.Trim() != "" || sqlWhere3.Trim() != "")
		{
			PrintLog("sqlWhere1", sqlWhere1);
			PrintLog("sqlWhere2", sqlWhere2);
			PrintLog("sqlWhere3", sqlWhere3);

			if (sqlWhere2.Trim() != "")
			{
				sqlWhere2 += ")";
			}

			if (sqlWhere3.Trim() != "")
			{
				sqlWhere3 += ")";
			}

			sqlstr = "SELECT code_class,code,code_desc_1_content,code_desc_2_content,code_desc_3_content,code_desc_4_content FROM tep0002";

			if (sqlWhere1.Trim() != "")
			{
				if (sqlWhere.Trim() == "")
				{
					sqlWhere = " WHERE " + sqlWhere1;;
				}
				else
				{
					sqlWhere += " OR " + sqlWhere1;
				}

			}

			if (sqlWhere2.Trim() != "")
			{
				if (sqlWhere.Trim() == "")
				{
					sqlWhere = " WHERE " + sqlWhere2;;
				}
				else
				{
					sqlWhere += " OR " + sqlWhere2;
				}
			}

			if (sqlWhere3.Trim() != "")
			{
				if (sqlWhere.Trim() == "")
				{
					sqlWhere = " WHERE " + sqlWhere3;;
				}
				else
				{
					sqlWhere += " OR " + sqlWhere3;
				}
			}

			sqlstr += sqlWhere + " ORDER BY code_class,code";
			PrintLog("sqlstr", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(dtCode);
			cmd_inq.Close();

			codeClass = dtCode.Rows[0]["CODE_CLASS"];
			//PrintLog("codeClass", codeClass);

			bcls_ret->Tables.Add(codeClass);
			blkNum = bcls_ret->Tables.IndexOf(codeClass);
			//PrintLog("blkNum", blkNum);

			PrintLog("dtCode RowsCount", dtCode.Rows.get_Count());
			for (int i = 0; i < dtCode.Rows.get_Count(); i++)
			{
				if (codeClass != dtCode.Rows[i]["CODE_CLASS"].ToString())
				{
					codeClass = dtCode.Rows[i]["CODE_CLASS"];
					//PrintLog("codeClass", codeClass);

					bcls_ret->Tables.Add(codeClass);
					blkNum = bcls_ret->Tables.IndexOf(codeClass);
					//PrintLog("blkNum", blkNum);

					rowNum = 0;
				}

				bcls_ret->Tables[blkNum].Rows.Add();
				MergDataRow(dtCode.Rows[i], bcls_ret->Tables[blkNum].Rows[rowNum++], true, true);
				//PrintLog("rowNum", rowNum);
			}

		}

		if (queryFlag == 3 && !bcls_ret->Tables.Contains("QPLANPLAN_BACKLOG_CODE"))
		{
			bcls_ret->Tables.Add("QPLANPLAN_BACKLOG_CODE");
			bcls_ret->Tables["QPLANPLAN_BACKLOG_CODE"].Columns.Add(DT_STRING, "CODE");
			bcls_ret->Tables["QPLANPLAN_BACKLOG_CODE"].Columns.Add(DT_STRING, "CODE_DESC_1_CONTENT");
			if (bcls_ret->Tables["PROD_UNIT"].Rows[0]["UNIT_CODE"].ToString() == "LXXX")
			{
				sqlstr = "SELECT unit_code,unit_cname FROM tmm00si16 WHERE unit_code IN (SELECT DISTINCT plan_backlog_code FROM tpscra7) ORDER BY unit_code";
				PrintLog("sqlstr", sqlstr);
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
					bcls_ret->Tables["QPLANPLAN_BACKLOG_CODE"].Rows.Add();
					CDataRow &drUnitCode = bcls_ret->Tables["QPLANPLAN_BACKLOG_CODE"].Rows[bcls_ret->Tables["QPLANPLAN_BACKLOG_CODE"].Rows.get_Count() - 1];
					drUnitCode["CODE"] = cmd_inq.GetString(1);
					drUnitCode["CODE_DESC_1_CONTENT"] = cmd_inq.GetString(2);
				}
				cmd_inq.Close();
			}
			else
			{
				for (int i = 0; i < bcls_ret->Tables["PROD_UNIT"].Rows.get_Count(); i++)
				{
					bcls_ret->Tables["QPLANPLAN_BACKLOG_CODE"].Rows.Add();
					bcls_ret->Tables["QPLANPLAN_BACKLOG_CODE"].Rows[i]["CODE"] = bcls_ret->Tables["PROD_UNIT"].Rows[i]["UNIT_CODE"];
					bcls_ret->Tables["QPLANPLAN_BACKLOG_CODE"].Rows[i]["CODE_DESC_1_CONTENT"] = bcls_ret->Tables["PROD_UNIT"].Rows[i]["UNIT_CNAME"];
				}
			}
		}
		PrintLog("END");
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错,sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台,与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1,事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	//返回-1时事务将回滚,返回为0是事务将提交
	return doFlag;
}