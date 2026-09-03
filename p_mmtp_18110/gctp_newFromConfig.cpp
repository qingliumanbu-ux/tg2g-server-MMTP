#include "CDynaTable2.h"

BM2F_ENTERACE(gctp_newFromConfig)

BM2_FUNCTION_IMPORT
int f_gctp_configDataNew(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);

BM2_FUNCTION_EXPORT
int f_gctp_newFromConfig(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn)
{
	CTracer log(__FUNCTION__);

	//程序内部变量
	int doFlag = 0;
	CString sqlstr = " ";

	//应用变量
	CDecimal dAclid = 0;

	//动态数据表定义
	CDynaTable2 tgctp04("TGCTP04", conn);
	CDynaTable2 tgctp05("TGCTP05", conn);
	CDynaTable2 tgctp06("TGCTP06", conn);

	try
	{
		if (bcls_rec->Tables.Contains("FORM") && bcls_rec->Tables["FORM"].Rows.get_Count() > 0)
		{
			tgctp04.CopyFrom(bcls_rec->Tables["FORM"]);
			if (tgctp04.GetColValString("TEMPLATE_TYPE") == "0")
			{
				tgctp04.SetColVal("LOCATION", "00");
				tgctp04.SetColVal("MAIN_FLAG", "0");
			}
			else if (tgctp04.GetColValString("TEMPLATE_TYPE") == "1")
			{
				tgctp04.SetColVal("LOCATION", "01");
				tgctp04.SetColVal("MAIN_FLAG", "0");
			}
			else if (tgctp04.GetColValString("TEMPLATE_TYPE") == "2")
			{
				tgctp04.SetColVal("LOCATION", "03");
				tgctp04.SetColVal("MAIN_FLAG", "0");
			}

			tgctp04.SetColVal("FUNCTION_ID", CDateTime::Now().ToString("yyMMdd") + GetSeqence("GCTP_SEQ_ID", 4, conn));
			if (tgctp04.Insert() < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			if (GetColValueC(bcls_rec->Tables["FORM"], 0, "CFGITM_NAME2").Trim() != "")
			{
				tgctp04.SetColVal("CFGITM_NAME", GetColValueC(bcls_rec->Tables["FORM"], 0, "CFGITM_NAME2").Trim());

				if (tgctp04.GetColValString("TEMPLATE_TYPE") == "1")
				{
					tgctp04.SetColVal("LOCATION", "02");
					tgctp04.SetColVal("MAIN_FLAG", "1");
				}
				else if (tgctp04.GetColValString("TEMPLATE_TYPE") == "2")
				{
					tgctp04.SetColVal("LOCATION", "04");
					tgctp04.SetColVal("MAIN_FLAG", "1");
				}

				tgctp04.SetColVal("FUNCTION_ID", CDateTime::Now().ToString("yyMMdd") + GetSeqence("GCTP_SEQ_ID", 4, conn));
				if (tgctp04.Insert() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}


			if (GetColValueC(bcls_rec->Tables["FORM"], 0, "FUNC_DIV").Trim() != "")
			{
				PrintLog("自动添加按钮配置");
				if (GetColValueC(bcls_rec->Tables["FORM"], 0, "FUNC_DIV") != "3")
				{
					dAclid = 1000000000 + GetSeqence("GCTP_BUTTON_SEQ_NO", conn);
					PrintLog("dAclid", dAclid);

					//F2 查询按钮
					//操作配置表数据新增
					tgctp05.SetColVal("ID", dAclid);
					tgctp05.SetColVal("NAME", "F2");
					tgctp05.SetColVal("SEQ_NO", (CDecimal)1);
					tgctp05.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp05.SetColVal("DATASET_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp05.SetColVal("OPERATE_TYPE", "01");
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "01");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PAGE_ID");
					tgctp05.SetColVal("ITEM_CVALUE", "1");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输入参数配置表数据新增
					tgctp06.SetColVal("ID", dAclid);
					tgctp06.SetColVal("NAME", "F2");
					tgctp06.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp06.SetColVal("NOW_ROW", tgctp05.GetColValString("NOW_ROW"));
					tgctp06.SetColVal("PAGE_ID", "1");
					tgctp06.SetColVal("HANDLE_DIV", "0");
					tgctp06.SetColVal("PROC_SEQ_NO", (CDecimal)1);
					tgctp06.SetColVal("PARA_TYPE", "01");
					tgctp06.SetColVal("CFGITM_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp06.SetColVal("DATA_ORIGIN", "01");
					tgctp06.SetColVal("ITEM_MUST_FLAG", "0");
					tgctp06.SetColVal("OPERATE_OBJECT", " ");
					tgctp06.SetColVal("OPERATE_MODE", " ");

					if (tgctp06.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输出参数配置表数据新增
					tgctp06.SetColVal("HANDLE_DIV", "1");
					tgctp06.SetColVal("DATA_ORIGIN", " ");
					tgctp06.SetColVal("OBJECT_AREA", "02");
					tgctp06.SetColVal("SHOW_FLAG", "0");

					if (tgctp06.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}
				}

				if (GetColValueC(bcls_rec->Tables["FORM"], 0, "FUNC_DIV") == "1")
				{
					//F3 新增按钮
					dAclid = 1000000000 + GetSeqence("GCTP_BUTTON_SEQ_NO", conn);
					PrintLog("dAclid", dAclid);

					//操作配置表数据新增
					tgctp05.ClearDataRow();
					tgctp05.SetColVal("ID", dAclid);
					tgctp05.SetColVal("NAME", "F3");
					tgctp05.SetColVal("SEQ_NO", (CDecimal)1);
					tgctp05.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp05.SetColVal("DATASET_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp05.SetColVal("OPERATE_TYPE", "02");
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "02");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PARTITION");
					tgctp05.SetColVal("ITEM_CVALUE", GetColValueC(bcls_rec->Tables["FORM"], 0, "ABBREV"));

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PAGE_ID");
					tgctp05.SetColVal("ITEM_CVALUE", "1");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输入参数配置表数据新增
					tgctp06.SetColVal("ID", dAclid);
					tgctp06.SetColVal("NAME", "F3");
					tgctp06.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp06.SetColVal("NOW_ROW", tgctp05.GetColValString("NOW_ROW"));
					tgctp06.SetColVal("PAGE_ID", "1");
					tgctp06.SetColVal("HANDLE_DIV", "0");
					tgctp06.SetColVal("PROC_SEQ_NO", (CDecimal)1);
					tgctp06.SetColVal("PARA_TYPE", "01");
					tgctp06.SetColVal("CFGITM_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp06.SetColVal("DATA_ORIGIN", "02");
					tgctp06.SetColVal("ITEM_MUST_FLAG", "0");
					tgctp06.SetColVal("OPERATE_OBJECT", "0");
					tgctp06.SetColVal("OPERATE_MODE", "1");

					if (tgctp06.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//第二行操作配置，复制F2功能自动刷新
					tgctp05.SetColVal("SEQ_NO", (CDecimal)2);
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "10");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输出参数配置表数据新增
					//tgctp06.SetColVal("HANDLE_DIV", "1");
					//tgctp06.SetColVal("DATA_ORIGIN", " ");
					//tgctp06.SetColVal("OBJECT_AREA", "02");
					//tgctp06.SetColVal("SHOW_FLAG", "0");

					//if (tgctp06.Insert() < 0)
					//{
					//	throw CApplicationException(-1, s.msg, s.svc_name);
					//}

					//F4 修改按钮
					dAclid = 1000000000 + GetSeqence("GCTP_BUTTON_SEQ_NO", conn);
					PrintLog("dAclid", dAclid);

					//操作配置表数据新增
					tgctp05.ClearDataRow();
					tgctp05.SetColVal("ID", dAclid);
					tgctp05.SetColVal("NAME", "F4");
					tgctp05.SetColVal("SEQ_NO", (CDecimal)1);
					tgctp05.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp05.SetColVal("DATASET_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp05.SetColVal("OPERATE_TYPE", "02");
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "03");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PARTITION");
					tgctp05.SetColVal("ITEM_CVALUE", GetColValueC(bcls_rec->Tables["FORM"], 0, "ABBREV"));

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PAGE_ID");
					tgctp05.SetColVal("ITEM_CVALUE", "1");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输入参数配置表数据新增
					tgctp06.SetColVal("ID", dAclid);
					tgctp06.SetColVal("NAME", "F4");
					tgctp06.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp06.SetColVal("NOW_ROW", tgctp05.GetColValString("NOW_ROW"));
					tgctp06.SetColVal("PAGE_ID", "1");
					tgctp06.SetColVal("HANDLE_DIV", "0");
					tgctp06.SetColVal("PROC_SEQ_NO", (CDecimal)1);
					tgctp06.SetColVal("PARA_TYPE", "01");
					tgctp06.SetColVal("CFGITM_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp06.SetColVal("DATA_ORIGIN", "02");
					tgctp06.SetColVal("ITEM_MUST_FLAG", "0");
					tgctp06.SetColVal("OPERATE_OBJECT", "0");
					tgctp06.SetColVal("OPERATE_MODE", "0");

					if (tgctp06.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//第二行操作配置，复制F2功能自动刷新
					tgctp05.SetColVal("SEQ_NO", (CDecimal)2);
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "10");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "COPY_OPERATE_NAME");
					tgctp05.SetColVal("ITEM_CVALUE", "F2");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PARTITION");
					tgctp05.SetColVal("ITEM_CVALUE", GetColValueC(bcls_rec->Tables["FORM"], 0, "ABBREV"));

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PAGE_ID");
					tgctp05.SetColVal("ITEM_CVALUE", "1");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输出参数配置表数据新增
					//tgctp06.SetColVal("HANDLE_DIV", "1");
					//tgctp06.SetColVal("DATA_ORIGIN", " ");
					//tgctp06.SetColVal("OBJECT_AREA", "02");
					//tgctp06.SetColVal("SHOW_FLAG", "0");

					//if (tgctp06.Insert() < 0)
					//{
					//	throw CApplicationException(-1, s.msg, s.svc_name);
					//}

					//F5 删除按钮
					dAclid = 1000000000 + GetSeqence("GCTP_BUTTON_SEQ_NO", conn);
					PrintLog("dAclid", dAclid);

					//操作配置表数据新增
					tgctp05.ClearDataRow();
					tgctp05.SetColVal("ID", dAclid);
					tgctp05.SetColVal("NAME", "F5");
					tgctp05.SetColVal("SEQ_NO", (CDecimal)1);
					tgctp05.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp05.SetColVal("DATASET_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp05.SetColVal("OPERATE_TYPE", "02");
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "04");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PARTITION");
					tgctp05.SetColVal("ITEM_CVALUE", GetColValueC(bcls_rec->Tables["FORM"], 0, "ABBREV"));

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PAGE_ID");
					tgctp05.SetColVal("ITEM_CVALUE", "1");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输入参数配置表数据新增
					tgctp06.SetColVal("ID", dAclid);
					tgctp06.SetColVal("NAME", "F5");
					tgctp06.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp06.SetColVal("NOW_ROW", tgctp05.GetColValString("NOW_ROW"));
					tgctp06.SetColVal("PAGE_ID", "1");
					tgctp06.SetColVal("HANDLE_DIV", "0");
					tgctp06.SetColVal("PROC_SEQ_NO", (CDecimal)1);
					tgctp06.SetColVal("PARA_TYPE", "01");
					tgctp06.SetColVal("CFGITM_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp06.SetColVal("DATA_ORIGIN", "02");
					tgctp06.SetColVal("ITEM_MUST_FLAG", "0");
					tgctp06.SetColVal("OPERATE_OBJECT", "0");
					tgctp06.SetColVal("OPERATE_MODE", "0");

					if (tgctp06.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//第二行操作配置，复制F2功能自动刷新
					tgctp05.SetColVal("SEQ_NO", (CDecimal)2);
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "10");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "COPY_OPERATE_NAME");
					tgctp05.SetColVal("ITEM_CVALUE", "F2");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PARTITION");
					tgctp05.SetColVal("ITEM_CVALUE", GetColValueC(bcls_rec->Tables["FORM"], 0, "ABBREV"));

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PAGE_ID");
					tgctp05.SetColVal("ITEM_CVALUE", "1");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}
				}
				else if (GetColValueC(bcls_rec->Tables["FORM"], 0, "FUNC_DIV") == "2")
				{
					//F3 维护按钮
					dAclid = 1000000000 + GetSeqence("GCTP_BUTTON_SEQ_NO", conn);
					PrintLog("dAclid", dAclid);

					//操作配置表数据新增
					tgctp05.ClearDataRow();
					tgctp05.SetColVal("ID", dAclid);
					tgctp05.SetColVal("SEQ_NO", (CDecimal)1);
					tgctp05.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp05.SetColVal("DATASET_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp05.SetColVal("NAME", "F3");
					tgctp05.SetColVal("OPERATE_TYPE", "02");
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "05");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PARTITION");
					tgctp05.SetColVal("ITEM_CVALUE", GetColValueC(bcls_rec->Tables["FORM"], 0, "ABBREV"));

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_PAGE_ID");
					tgctp05.SetColVal("ITEM_CVALUE", "1");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输入参数配置表数据新增
					tgctp06.SetColVal("ID", dAclid);
					tgctp06.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp06.SetColVal("NOW_ROW", tgctp05.GetColValString("NOW_ROW"));
					tgctp06.SetColVal("PAGE_ID", "1");
					tgctp06.SetColVal("HANDLE_DIV", "0");
					tgctp06.SetColVal("PROC_SEQ_NO", (CDecimal)1);
					tgctp06.SetColVal("PARA_TYPE", "01");
					tgctp06.SetColVal("CFGITM_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp06.SetColVal("DATA_ORIGIN", "02");
					tgctp06.SetColVal("ITEM_MUST_FLAG", "0");
					tgctp06.SetColVal("OPERATE_OBJECT", "3");
					tgctp06.SetColVal("OPERATE_MODE", "5");

					if (tgctp06.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//第二行操作配置，复制F2功能自动刷新
					tgctp05.SetColVal("SEQ_NO", (CDecimal)2);
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "10");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}

					//输出参数配置表数据新增
					//tgctp06.SetColVal("HANDLE_DIV", "1");
					//tgctp06.SetColVal("DATA_ORIGIN", " ");
					//tgctp06.SetColVal("OBJECT_AREA", "02");
					//tgctp06.SetColVal("SHOW_FLAG", "0");

					//if (tgctp06.Insert() < 0)
					//{
					//	throw CApplicationException(-1, s.msg, s.svc_name);
					//}
				}

				if (GetColValueC(bcls_rec->Tables["FORM"], 0, "FUNC_DIV") != "3")
				{
					//F11 重载按钮
					//PrintLog("****** 重载,配置按钮");
					//dAclid = 1000000000 + GetSeqence("GCTP_BUTTON_SEQ_NO", conn);
					//PrintLog("dAclid", dAclid);

					//操作配置表数据新增
					//tgctp05.ClearDataRow();
					//tgctp05.SetColVal("ID", dAclid);
					//tgctp05.SetColVal("SEQ_NO", (CDecimal)1);
					//tgctp05.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					//tgctp05.SetColVal("NAME", "F11");
					//tgctp05.SetColVal("OPERATE_TYPE", "01");
					//tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					//tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					//tgctp05.SetColVal("ITEM_CVALUE", "98");

					//if (tgctp05.Insert() < 0)
					//{
					//	throw CApplicationException(-1, s.msg, s.svc_name);
					//}

					//F12 配置按钮
					dAclid = 1000000000 + GetSeqence("GCTP_BUTTON_SEQ_NO", conn);
					PrintLog("dAclid", dAclid);

					//操作配置表数据新增
					tgctp05.ClearDataRow();
					tgctp05.SetColVal("ID", dAclid);
					tgctp05.SetColVal("SEQ_NO", (CDecimal)1);
					tgctp05.SetColVal("FORM_NO", tgctp04.GetColValString("FORM_NO"));
					tgctp05.SetColVal("DATASET_NAME", tgctp04.GetColValString("CFGITM_NAME"));
					tgctp05.SetColVal("NAME", "F12");
					tgctp05.SetColVal("OPERATE_TYPE", "01");
					tgctp05.SetColVal("NOW_ROW", GetTrackSeqNo("GCTP_VTABLE_ROWID", 20, conn));
					tgctp05.SetColVal("ITEM_ENAME", "OPERATE_FUNCTION");
					tgctp05.SetColVal("ITEM_CVALUE", "99");

					if (tgctp05.Insert() < 0)
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}
				}
			}
		}

		if (bcls_rec->Tables.Contains("DS_MAIN") && bcls_rec->Tables["DS_MAIN"].Rows.get_Count() > 0)
		{
			doFlag = f_gctp_configDataNew(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
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

	//if (doFlag == 0)
	//{
	//	CTransactionManager::Commit(0);
	//	CTransactionManager::Begin(0, 0);
	//}
	//else
	//{
	//	CTransactionManager::Abort(0);
	//	CTransactionManager::Begin(0, 0);
	//}

	return doFlag;
}