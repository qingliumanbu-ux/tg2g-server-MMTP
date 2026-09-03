
#include "CDynaTable.h"

BM2F_ENTERACE(mmtp_configSave)
int f_mmtp_configSave(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	int doFlag = 0;
	CString sqlstr = "";
	CString procFlag = "";
	CString dbLinkName = "";
	CString tableName = "";

	CDbCommand cmd(conn);

	try
	{
		CTracer log(__FUNCTION__);

		procFlag = GetColValueC(bcls_rec->Tables[0], 0, "PROC_FLAG");
		PrintLog("procFlag", procFlag);

		if (procFlag == "C" || procFlag == "CG")
		{
			CString cfgName = bcls_rec->Tables[0].Rows[0]["CFGITM_NAME"].ToString().Trim();
			CString cfgNameSrc = bcls_rec->Tables[0].Rows[0]["CFGITM_NAME_SRC"].ToString().Trim();
			CString cfgGrpName = "";
			CString cfgGrpNameSrc = "";

			if (cfgName == "" || cfgNameSrc == "")
			{
				strcpy(s.msg, "配置号为空");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			dbLinkName = GetColValueC(bcls_rec->Tables[0], 0, "DB_LINK_NAME");
			PrintLog("dbLinkName", dbLinkName);
			if (dbLinkName.Trim() != "")
			{
				PrintLog("Commit");
				CTransactionManager::Commit(0);
			}

			if (procFlag == "CG")
			{
				cfgGrpName = bcls_rec->Tables[0].Rows[0]["CFGGRP_NAME"].ToString().Trim();
				cfgGrpNameSrc = bcls_rec->Tables[0].Rows[0]["CFGGRP_NAME_SRC"].ToString().Trim();

				Log::Trace("", __FUNCTION__, "复制配置[{0}.{1}]->[{2}.{3}]", cfgNameSrc, cfgGrpNameSrc, cfgName, cfgGrpName);

				cmd.Parameters.Set("cfgName", cfgName);
				cmd.Parameters.Set("cfgGrpName", cfgGrpName);
				cmd.Parameters.Set("cfgNameSrc", cfgNameSrc);
				cmd.Parameters.Set("cfgGrpNameSrc", cfgGrpNameSrc);
				cmd.Parameters.Set("recCreator", (CString)s.userid);
				cmd.Parameters.Set("recCreateTime", CDateTime::Now().ToString("yyyyMMddHHmmss"));

				sqlstr = "DELETE FROM tmmtp01 WHERE cfgitm_name = @cfgName AND cfggrp_name = @cfgGrpName";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				sqlstr = "DELETE FROM tmmtp02 WHERE cfgitm_name = @cfgName AND cfggrp_name = @cfgGrpName";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				sqlstr = "DELETE FROM tmmtp03 WHERE cfgitm_name = @cfgName AND cfggrp_name = @cfgGrpName";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				sqlstr = "DELETE FROM tmmtp05 WHERE cfgitm_name = @cfgName AND cfggrp_name = @cfgGrpName";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				PrintLog("删除目的配置源数据成功");

				sqlstr = "INSERT INTO tmmtp01 SELECT @recCreator,@recCreateTime,' ',' ',' ',' ',' ',' ',@cfgName,@cfgGrpName,table_name,user_id,"
					"form_name,seq_no,column_name,column_cname,control_class,control_width,control_height,code_class,sql_context,"
					"disp_col_flag,cal_sign_code,default_value,foreign_key_seq,row_seq,col_seq,column_count,qry_tab_where,space_x,"
					"filter_column_name,ext_item1,ext_item2,ext_item3,ext_item4,ext_item5,ext_item6,ext_item7,ext_item8,ext_item9"
					" FROM tmmtp01 WHERE cfgitm_name = @cfgNameSrc AND cfggrp_name = @cfgGrpNameSrc";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("INSERT TMMTP01成功");

				sqlstr = "INSERT INTO tmmtp02 SELECT @recCreator,@recCreateTime,' ',' ',' ',' ',' ',' ',@cfgName,@cfgGrpName,table_name,user_id,"
					"form_name,seq_no,column_name,column_cname,data_type,code_class,sql_context,disp_col_flag,keyword_flag,key_col_flag,"
					"revise_flag,order_mark,lock_flag,group_flag,group_mark,foreign_key_seq,query_sql,cnd_relation,default_value,row_seq,col_seq,"
					"column_count,control_class,control_width,unit,trans_factor,code_ret_col_name1,code_ret_col_name2,space_x,func_name,key_1,keyvalue_1,"
					"item_upd_mode,item_upd_type,page_id,page_name,font_size,font_color,lable_color,col_type,filter_column_name,operation_type,"
					"add_flag,total_len_char,decimal_digit,ext_item1,ext_item2,ext_item3,ext_item4,ext_item5,ext_item6,ext_item7,ext_item8,ext_item9"
					" FROM tmmtp02 WHERE cfgitm_name = @cfgNameSrc AND cfggrp_name = @cfgGrpNameSrc";
				PrintLog("sqlstr", sqlstr);
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("INSERT TMMTP02成功");

				sqlstr = "INSERT INTO tmmtp03 SELECT @recCreator,@recCreateTime,' ',' ',' ',' ',' ',' ',@cfgName,cfgitm_desc,@cfgGrpName,page_name,area_desc,"
					"table_name,table_type,table_ename,table_name_1,table_name_2,joint_type_code,user_id,form_name,func_id,template_type,location,"
					"tc_no,sheet_num,cust_flag,page_num,font_size,use_flag,column_count,control_width,select_flag,ext_item1,ext_item2,ext_item3,"
					"ext_item4,ext_item5,ext_item6,ext_item7,ext_item8,ext_item9 FROM tmmtp03 WHERE cfgitm_name = @cfgNameSrc AND cfggrp_name = @cfgGrpNameSrc";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("INSERT TMMTP03成功");

				sqlstr = "INSERT INTO tmmtp05 SELECT @recCreator,@recCreateTime,' ',' ',' ',' ',' ',' ',@cfgName,cfggrp_name,operation_type,"
					"operate_type,operate_mode,operate_side,object_area,item_upd_mode,svc_name,func_name,key_1,keyvalue_1,key_2,keyvalue_2,"
					"key_3,keyvalue_3,call_form FROM tmmtp05 WHERE cfgitm_name = @cfgNameSrc AND cfggrp_name = @cfgGrpNameSrc";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				PrintLog("INSERT TMMTP05成功");
			}
			else
			{
				Log::Trace("", __FUNCTION__, "复制配置[{0}]->[{1}]", cfgNameSrc, cfgName);

				cmd.Parameters.Set("cfgName", cfgName);
				cmd.Parameters.Set("cfgNameSrc", cfgNameSrc);
				cmd.Parameters.Set("recCreator", (CString)s.userid);
				cmd.Parameters.Set("recCreateTime", CDateTime::Now().ToString("yyyyMMddHHmmss"));

				sqlstr = "DELETE FROM tmmtp01 WHERE cfgitm_name = @cfgName";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				sqlstr = "DELETE FROM tmmtp02 WHERE cfgitm_name = @cfgName";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				sqlstr = "DELETE FROM tmmtp03 WHERE cfgitm_name = @cfgName";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				sqlstr = "DELETE FROM tmmtp05 WHERE cfgitm_name = @cfgName";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				PrintLog("删除目的配置源数据成功");

				tableName = "tmmtp01";
				if (dbLinkName.Trim() != "")
				{
					tableName += "@" + dbLinkName.Trim();
				}

				sqlstr = "INSERT INTO tmmtp01 SELECT @recCreator,@recCreateTime,' ',' ',' ',' ',' ',' ',@cfgName,cfggrp_name,table_name,"
					"user_id,form_name,seq_no,column_name,column_cname,control_class,control_width,control_height,code_class,sql_context,"
					"disp_col_flag,cal_sign_code,default_value,foreign_key_seq,row_seq,col_seq,column_count,qry_tab_where,space_x,"
					"filter_column_name,ext_item1,ext_item2,ext_item3,ext_item4,ext_item5,ext_item6,ext_item7,ext_item8,ext_item9"
					" FROM " + tableName + " WHERE cfgitm_name = @cfgNameSrc";
				cmd.SetCommandText(sqlstr);
				try
				{
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
				catch (CDbException& ex)
				{
					cmd.Close();
					CTransactionManager::Abort(0);
					CTransactionManager::Begin(0, 0);

					CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
					CMessageFormat::Format(s.msg, "Database Error，sqlcode=[{0},{1}]", arguments, 2);
					CString str = sqlstr + "\r\n" + ex.GetMsg();
					strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);

					s.flag = -1;
					doFlag = -1;

					return doFlag;
				}
				PrintLog("INSERT TMMTP01成功");

				tableName = "tmmtp02";
				if (dbLinkName.Trim() != "")
				{
					tableName += "@" + dbLinkName.Trim();
				}

				sqlstr = "INSERT INTO tmmtp02 SELECT @recCreator,@recCreateTime,' ',' ',' ',' ',' ',' ',@cfgName,cfggrp_name,table_name,user_id,"
					"form_name,seq_no,column_name,column_cname,data_type,code_class,sql_context,disp_col_flag,keyword_flag,key_col_flag,"
					"revise_flag,order_mark,lock_flag,group_flag,group_mark,foreign_key_seq,query_sql,cnd_relation,default_value,row_seq,col_seq,"
					"column_count,control_class,control_width,unit,trans_factor,code_ret_col_name1,code_ret_col_name2,space_x,func_name,key_1,keyvalue_1,"
					"item_upd_mode,item_upd_type,page_id,page_name,font_size,font_color,lable_color,col_type,filter_column_name,operation_type,"
					"add_flag,total_len_char,decimal_digit,ext_item1,ext_item2,ext_item3,ext_item4,ext_item5,ext_item6,ext_item7,ext_item8,ext_item9 FROM "
					+ tableName + " WHERE cfgitm_name = @cfgNameSrc";
				PrintLog("sqlstr", sqlstr);
				cmd.SetCommandText(sqlstr);
				try
				{
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
				catch (CDbException& ex)
				{
					cmd.Close();
					CTransactionManager::Abort(0);
					CTransactionManager::Begin(0, 0);

					CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
					CMessageFormat::Format(s.msg, "Database Error，sqlcode=[{0},{1}]", arguments, 2);
					CString str = sqlstr + "\r\n" + ex.GetMsg();
					strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);

					s.flag = -1;
					doFlag = -1;

					return doFlag;
				}
				PrintLog("INSERT TMMTP02成功");

				tableName = "tmmtp03";
				if (dbLinkName.Trim() != "")
				{
					tableName += "@" + dbLinkName.Trim();
				}

				sqlstr = "INSERT INTO tmmtp03 SELECT @recCreator,@recCreateTime,' ',' ',' ',' ',' ',' ',@cfgName,cfgitm_desc,cfggrp_name,page_name,area_desc,"
					"table_name,table_type,table_ename,table_name_1,table_name_2,joint_type_code,user_id,form_name,func_id,template_type,location,"
					"tc_no,sheet_num,cust_flag,page_num,font_size,use_flag,column_count,control_width,select_flag,ext_item1,ext_item2,ext_item3,"
					"ext_item4,ext_item5,ext_item6,ext_item7,ext_item8,ext_item9 FROM " + tableName + " WHERE cfgitm_name = @cfgNameSrc";
				cmd.SetCommandText(sqlstr);
				try
				{
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
				catch (CDbException& ex)
				{
					cmd.Close();
					CTransactionManager::Abort(0);
					CTransactionManager::Begin(0, 0);

					CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
					CMessageFormat::Format(s.msg, "Database Error，sqlcode=[{0},{1}]", arguments, 2);
					CString str = sqlstr + "\r\n" + ex.GetMsg();
					strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);

					s.flag = -1;
					doFlag = -1;

					return doFlag;
				}
				PrintLog("INSERT TMMTP03成功");

				tableName = "tmmtp05";
				if (dbLinkName.Trim() != "")
				{
					tableName += "@" + dbLinkName.Trim();
				}

				sqlstr = "INSERT INTO tmmtp05 SELECT @recCreator,@recCreateTime,' ',' ',' ',' ',' ',' ',@cfgName,cfggrp_name,operation_type,"
					"operate_type,operate_mode,operate_side,object_area,item_upd_mode,svc_name,func_name,key_1,keyvalue_1,key_2,keyvalue_2,"
					"key_3,keyvalue_3,call_form FROM " + tableName + " WHERE cfgitm_name = @cfgNameSrc";
				cmd.SetCommandText(sqlstr);
				try
				{
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
				catch (CDbException& ex)
				{
					cmd.Close();
					CTransactionManager::Abort(0);
					CTransactionManager::Begin(0, 0);

					CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
					CMessageFormat::Format(s.msg, "Database Error，sqlcode=[{0},{1}]", arguments, 2);
					CString str = sqlstr + "\r\n" + ex.GetMsg();
					strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);

					s.flag = -1;
					doFlag = -1;

					return doFlag;
				}
				PrintLog("INSERT TMMTP05成功");

				if (dbLinkName.Trim() != "")
				{
					PrintLog("Commit");
					CTransactionManager::Commit(0);
					CTransactionManager::Begin(0, 0);
				}
			}

			PrintLog("Return");
			return doFlag;
		}
		else if (procFlag == "D")
		{
			CString cfgName = bcls_rec->Tables[0].Rows[0]["CFGITM_NAME"].ToString().Trim();
			CString cfgGrpName = bcls_rec->Tables[0].Rows[0]["CFGGRP_NAME"].ToString().Trim();

			Log::Trace("", __FUNCTION__, "删除配置[{0}][{1}]", cfgName, cfgGrpName);

			if (cfgName == "")
			{
				strcpy(s.msg, "配置号为空");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			cmd.Parameters.Set("cfgName", cfgName);
			cmd.Parameters.Set("cfgGrpName", cfgGrpName);

			sqlstr = "DELETE FROM tmmtp01 WHERE cfgitm_name = @cfgName AND cfggrp_name = @cfgGrpName";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tmmtp02 WHERE cfgitm_name = @cfgName AND cfggrp_name = @cfgGrpName";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tmmtp03 WHERE cfgitm_name = @cfgName AND cfggrp_name = @cfgGrpName";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "DELETE FROM tmmtp05 WHERE cfgitm_name = @cfgName AND cfggrp_name = @cfgGrpName";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			return doFlag;
		}

		CDynaTable tmmtp01("TMMTP01", conn);
		CDynaTable tmmtp02("TMMTP02", conn);
		CDynaTable tmmtp03("TMMTP03", conn);
		CDynaTable tmmtp04("TMMTP04", conn);
		CDynaTable tmmtp05("TMMTP05", conn);

		if (bcls_rec->Tables[0].Rows.get_Count() > 0)
		{
			tmmtp01.CopyFrom(bcls_rec->Tables[0]);
			//PrintDataTable(tmmtp01.GetDataTable());
			tmmtp01.AddFilterColName("CFGITM_NAME");
			if (procFlag != "I")
			{
				tmmtp01.AddFilterColName("CFGGRP_NAME");
			}

			if (tmmtp01.Delete() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			PrintLog("TMMTP01删除成功");

			if (tmmtp01.GetColValString("CONTROL_CLASS").Trim() != "")
			{
				if (procFlag != "I")
				{
					for (int i = 0; i < tmmtp01.GetDataTable().Rows.get_Count(); i++)
					{
						tmmtp01.SetColVal("SEQ_NO", (CDecimal)(i + 1), i);
						tmmtp01.SetColVal("USER_ID", s.userid, i);
					}
				}

				if (tmmtp01.Insert() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				PrintLog("TMMTP01新增成功");
			}
		}

		if (bcls_rec->Tables[1].Rows.get_Count() > 0)
		{
			tmmtp02.CopyFrom(bcls_rec->Tables[1]);
			//tmmtp02.Print();

			tmmtp02.AddFilterColName("CFGITM_NAME");
			if (procFlag != "I")
			{
				tmmtp02.AddFilterColName("CFGGRP_NAME");
				//tmmtp02.AddFilterColName("PAGE_ID");

				//if (tmmtp02.GetColValString("PAGE_ID").Trim() == "")
				//{
				//	tmmtp02.SetColVal("PAGE_ID", "0");
				//}
			}

			if (tmmtp02.Delete() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			PrintLog("TMMTP02删除成功");

			//if (tmmtp02.GetColValString("DATA_TYPE").Trim() != "")
			//{
				if (procFlag != "I")
				{
					for (int i = 0; i < tmmtp02.GetDataTable().Rows.get_Count(); i++)
					{
						tmmtp02.SetColVal("SEQ_NO", (CDecimal)(i + 1), i);
						tmmtp02.SetColVal("USER_ID", s.userid, i);

						if (tmmtp02.GetColValString("PAGE_ID", i).Trim() == "")
						{
							tmmtp02.SetColVal("PAGE_ID", "0", i);
						}
					}
				}

				if (tmmtp02.Insert() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				PrintLog("TMMTP02新增成功");
			//}
		}

		if (bcls_rec->Tables[2].Rows.get_Count() > 0)
		{
			tmmtp03.CopyFrom(bcls_rec->Tables[2]);
			tmmtp03.AddFilterColName("CFGITM_NAME");
			if (procFlag != "I")
			{
				tmmtp03.AddFilterColName("CFGGRP_NAME");
			}

			if (tmmtp03.Delete() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			PrintLog("TMMTP03删除成功");

			tmmtp03.SetColVal("USER_ID", s.userid);
			if (tmmtp03.Insert() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			PrintLog("TMMTP03新增成功");

			if (tmmtp03.GetColValString("CFGGRP_NAME").Trim() != "")
			{
				tmmtp03.AddUpdateColName("TEMPLATE_TYPE");
				tmmtp03.AddUpdateColName("SHEET_NUM");
				//tmmtp03.AddUpdateColName("FUNC_ID");
				tmmtp03.ClearFilterColName();
				tmmtp03.AddFilterColName("CFGITM_NAME");
				if (tmmtp03.Update() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}
		}

		if (bcls_rec->Tables[3].Rows.get_Count() > 0)
		{
			tmmtp04.CopyFrom(bcls_rec->Tables[3]);
			tmmtp04.AddFilterColName("CFGITM_NAME");
			if (tmmtp04.Delete() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			PrintLog("TMMTP04删除成功");

			for (int i = 0; i < tmmtp04.GetDataTable().Rows.get_Count(); i++)
			{
				tmmtp04.SetColVal("SEQ_NO", (CDecimal)(i + 1), i);
			}

			if (tmmtp04.Insert() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			PrintLog("TMMTP04新增成功");
		}

		if (bcls_rec->Tables[4].Rows.get_Count() > 0)
		{
			tmmtp05.CopyFrom(bcls_rec->Tables[4]);
			//tmmtp05.Print();
			tmmtp05.AddFilterColName("CFGITM_NAME");
			if (procFlag != "I")
			{
				tmmtp05.AddFilterColName("CFGGRP_NAME");
			}

			if (tmmtp05.Delete() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			PrintLog("TMMTP05删除成功");

			if (tmmtp05.GetColValString("OPERATION_TYPE").Trim() != "")
			{
				if (tmmtp05.Insert() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				PrintLog("TMMTP05新增成功");
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
