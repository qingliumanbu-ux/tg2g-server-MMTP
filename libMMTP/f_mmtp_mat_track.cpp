#include "CDynaTable.h"

BM2_FUNCTION_IMPORT		//源物料数据校验函数
int f_mm0099_00(CDynaTable * matData, CString eventId);

BM2_FUNCTION_IMPORT		//物料状态计算函数
int f_mm0099_04(CDynaTable * matData, CDbConnection * conn);

BM2_FUNCTION_IMPORT		//记录仓库履历函数
int f_mm0099_08(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);

#if defined _SYS_MMS || defined _SYS_MES    //MMS层或MES系统
BM2_FUNCTION_IMPORT		//物料跟踪抛成本函数
int f_mm0099_ac(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);
#endif

BM2_FUNCTION_IMPORT		//物料跟踪抛收发存函数
int f_mm0099_ym(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);

BM2_FUNCTION_IMPORT		//同步四级电文
int f_mmsm_t80rs0_snd(EIClass* bcls_rec, EIClass* bcls_ret, CDbConnection* conn);


BM2_FUNCTION_EXPORT
int f_mmtp_mat_track(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CString matKind, CString eventId, CString eventLineType)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int callFlag = 0;
	int tcSendFlag = 0;

	CString strSql = "";;
	CString eventDesc = "";
	CString eventUserId = (CString)s.userid;
	CString eventTime = "";
	CString keyValue1 = "";
	CString keyValue2 = "";
	CString keyValue3 = "";
	CString keyValue4 = "";
	CString keyValue5 = "";
	CString keyValue6 = "";
	CString matNo = "";
	CString tcNoRec = "";
	CString tcNo = "";
	

	CDataTable dtMatDataColumn;
	CDataTable dtMatTraceColumn;
	//CDataTable dtMatTreeColumn;

	//CDynaTable tmm0005(conn);
	CDynaTable matData(conn);
	CDynaTable matDataHistory(conn);
	CDynaTable matTrace(conn);

	CDbCommand cmd(conn);

	try
	{
		//判断是否存在指定块
		if (!bcls_rec->Tables.Contains("MM0099") || bcls_rec->Tables["MM0099"].Rows.get_Count() == 0)
		{
			strcpy(s.msg, "没有传入物料跟踪数据");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		//获取传入MM0099块数据
		CDataTable& dtReceive = bcls_rec->Tables["MM0099"];

		//检查输入参数合法性		
		if (matKind.Trim() == "")
		{
			matKind = dtReceive.Rows[0]["MAT_KIND"].ToString();
			if (matKind.Trim() == "")
			{
				strcpy(s.msg, "没有传入物料类型");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}

		if (eventId == "")
		{
			eventId = dtReceive.Rows[0]["EVENT_ID"].ToString();
			if (eventId.Trim() == "")
			{
				strcpy(s.msg, "没有传入事件号!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}

		if (eventLineType.Trim() == "")
		{
			eventLineType = "00";
		}

		if (!dtReceive.Columns.Contains("EVENT_ID"))
		{
			AddColValue(dtReceive, "EVENT_ID", eventId);
		}

		if (!dtReceive.Columns.Contains("EVENT_LINE_TYPE"))
		{
			AddColValue(dtReceive, "EVENT_LINE_TYPE", eventLineType);
		}

		if (!dtReceive.Columns.Contains("SYSTEM_ID"))
		{
			AddColValue(dtReceive, "SYSTEM_ID", "MM" + matKind);
		}

		if (!dtReceive.Columns.Contains("FUNC_ID"))
		{
			AddColValue(dtReceive, "FUNC_ID", s.svc_name);
		}

		//PrintLog("MAT_KIND", matKind);
		PrintLog("EVENT_ID", eventId);
		//PrintLog("EVENT_LINE_TYPE", eventLineType);
		Log::Trace("", __FUNCTION__, "EVENT_ID	= [{0}]MAT_KIND	= [{1}]EVENT_LINE_TYPE	= [{2}]CONUT	= [{3}]", eventId, matKind, eventLineType, bcls_rec->Tables["MM0099"].Rows.get_Count());
		//新增事件数据表
		if (bcls_rec->Tables.IndexOf("EVENT_DATA") < 0)
		{
			bcls_rec->Tables.Add("EVENT_DATA");
		}

		//查询物料跟踪事件表TMM0097
		strSql = "SELECT * FROM tmm0097 WHERE event_id = @eventId AND event_line_type = @eventLineType AND mat_kind = @matKind";
		cmd.Parameters.Set("eventId", eventId);
		cmd.Parameters.Set("eventLineType", eventLineType);
		cmd.Parameters.Set("matKind", matKind);
		cmd.SetCommandText(strSql);
		cmd.ExecuteQuery(bcls_rec->Tables["EVENT_DATA"]);
		cmd.Close();

		if (bcls_rec->Tables["EVENT_DATA"].Rows.get_Count() == 0)
		{
			sprintf(s.msg, "事件号" + eventId + "物料" + matKind + "事件产线类型" + eventLineType + "在MM0097A1画面中无此事件,请确定接口!");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		CDataRow &drEventData = bcls_rec->Tables["EVENT_DATA"].Rows[0];
		if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
		{
			PrintLog("EVENT_USE_FLAG", drEventData["EVENT_USE_FLAG"].ToString());
			PrintLog("EVENT_CALL_TYPE_CODE", drEventData["EVENT_CALL_TYPE_CODE"].ToString());
			PrintLog("EVENT_PROC_WAY", drEventData["EVENT_PROC_WAY"].ToString());
		}

		//校验事件是否使用
		if (drEventData["EVENT_USE_FLAG"].ToString() != "1")	//1-生效
		{
			sprintf(s.msg, "事件号[%s]物料[%s]产线类型[%s]在MM0097A1画面中的生效标记是未生效,请修改为生效!",
				(const char*)eventId, (const char*)matKind, (const char*)eventLineType);

			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		//校验电文发送类型不能为空
		if (drEventData["TC_SEND_FLAG"].ToString() != "N0" &&				//N0-不发送电文
			drEventData["TC_SEND_FLAG"].ToString() != "Y0" &&				//Y0-根据事件发送电文
			drEventData["TC_SEND_FLAG"].ToString() != "Y1")					//Y1-根据事件配置表发送电文
		{
			strcpy(s.msg, "事件号[" + eventId + "]物料[" + matKind + "]产线类型[" + eventLineType + "]电文发送类型[" +
				drEventData["TC_SEND_FLAG"].ToString() + "]错误,应该为N0/Y0/Y1,请在MM0097A1画面修改!");

			throw CApplicationException(-1, s.msg, log.Location);
		}

		//新增事件接口参数数据表
		if (bcls_rec->Tables.IndexOf("EVENT_PARA") < 0)
		{
			bcls_rec->Tables.Add("EVENT_PARA");
		}

		//查询物料跟踪事件接口参数表TMM0099
		strSql = "SELECT * FROM tmm0099 WHERE event_id = @eventId AND event_line_type = @eventLineType AND mat_kind = @matKind";
		cmd.SetCommandText(strSql);
		cmd.ExecuteQuery(bcls_rec->Tables["EVENT_PARA"]);
		cmd.Close();
		if (bcls_rec->Tables["EVENT_PARA"].Rows.get_Count() == 0)
		{
			sprintf(s.msg, "事件号" + eventId + "物料" + matKind + "事件产线类型" + eventLineType + "在MM0097A1画面中无此事件对应参数,请确定接口!");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		//参数校验
		for (int i = 0; i < bcls_rec->Tables["EVENT_PARA"].Rows.get_Count(); i++)
		{
			CDataRow &drEventPara = bcls_rec->Tables["EVENT_PARA"].Rows[i];

			CString itemEName = drEventPara["ITEM_ENAME"].ToString();
			CString itemCName = drEventPara["ITEM_CNAME"].ToString();
			int parmUpdRow = 0;

			//校验字段修改方式
			if (drEventPara["ITEM_UPD_MODE"].ToString() != "0"  &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "1"  &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "11" &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "12" &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "13" &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "14" &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "2"  &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "3"  &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "31" &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "4"  &&
				drEventPara["ITEM_UPD_MODE"].ToString() != "5")
			{
				strcpy(s.msg, "TMM0099表中字段[" + itemEName + "]字段修改方式[" + drEventPara["ITEM_UPD_MODE"].ToString() + "]错误!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//校验字段种类
			if (drEventPara["ITEM_KIND"].ToString() != "S" && 	//S-字符型
				drEventPara["ITEM_KIND"].ToString() != "D")		//D-数值型
			{
				strcpy(s.msg, "TMM0099表中字段[" + itemEName + "]的字段种类[" + drEventPara["ITEM_KIND"].ToString() + "]错误, 应该为S或D!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//校验字段是否传入
			if (!dtReceive.Columns.Contains(itemEName) && drEventPara["ITEM_PARA_ALLOW_NULL"].ToString() != "Y" &&
				drEventPara["ITEM_UPD_MODE"].ToString().Substring(0, 1) == "1")
			{
				strcpy(s.msg, "MM0099块中必须有列[" + itemCName + "][" + itemEName + "]!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//接口字段中配置列 EVENT_DESC,则获取该列的值
			if (dtReceive.Columns.Contains("EVENT_DESC"))
			{
				eventDesc = dtReceive.Rows[0]["EVENT_DESC"].ToString().Trim();
			}
			else if (itemEName.Trim() == "KEYVALUE_1" && dtReceive.Columns.Contains("KEYVALUE_1"))
			{
				keyValue1 = dtReceive.Rows[0]["KEYVALUE_1"].ToString().Trim();
			}
			else if (itemEName.Trim() == "KEYVALUE_2" && dtReceive.Columns.Contains("KEYVALUE_2"))
			{
				keyValue2 = dtReceive.Rows[0]["KEYVALUE_2"].ToString().Trim();
			}
			else if (itemEName.Trim() == "KEYVALUE_3" && dtReceive.Columns.Contains("KEYVALUE_3"))
			{
				keyValue3 = dtReceive.Rows[0]["KEYVALUE_3"].ToString().Trim();
			}
			else if (itemEName.Trim() == "KEYVALUE_4" && dtReceive.Columns.Contains("KEYVALUE_4"))
			{
				keyValue4 = dtReceive.Rows[0]["KEYVALUE_4"].ToString().Trim();
			}
			else if (itemEName.Trim() == "KEYVALUE_5" && dtReceive.Columns.Contains("KEYVALUE_5"))
			{
				keyValue5 = dtReceive.Rows[0]["KEYVALUE_5"].ToString().Trim();
			}
			else if (itemEName.Trim() == "KEYVALUE_6" && dtReceive.Columns.Contains("KEYVALUE_6"))
			{
				keyValue6 = dtReceive.Rows[0]["KEYVALUE_6"].ToString().Trim();
			}
		}

		if (eventDesc.Trim() == "")
		{
			eventDesc = drEventData["EVENT_DESC"].ToString();
		}

		if (drEventData["EVENT_CALL_TYPE_CODE"].ToString() == "2")		//单独函数
		{
			if (drEventData["EVENT_SPEC_PRO"].ToString() != "")
			{
				//设置EDCALL参数
				strcpy(e.func_name[0], "MMTP_MAT_TRACK_CALL");
				strcpy(e.pk_name[0], "EVENT_SPEC_PRO");
				strcpy(e.pk_val[0], drEventData["EVENT_SPEC_PRO"].ToString());
				bcls_rec->SetED(e);

				doFlag = f_epedcall(bcls_rec, bcls_ret);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}
			else
			{
				sprintf(s.msg, "事件号[%s]物料[%s]产线类型[%s]的调用类型[%s]是修改类为单独函数,请联系MM配置相应的单独函数!",
					(const char*)eventId, (const char*)matKind, (const char*)eventLineType, (const char*)drEventData["EVENT_CALL_TYPE_CODE"].ToString());
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}
		else if (drEventData["EVENT_CALL_TYPE_CODE"].ToString() == "11" || drEventData["EVENT_CALL_TYPE_CODE"].ToString() == "12")
		{
			if (drEventData["EVENT_SPEC_PRO"].ToString().Trim() != "")
			{
				//设置EDCALL参数
				strcpy(e.func_name[0], "MMTP_MAT_TRACK_CALL");
				strcpy(e.pk_name[0], "EVENT_SPEC_PRO");
				strcpy(e.pk_val[0], drEventData["EVENT_SPEC_PRO"].ToString());
				bcls_rec->SetED(e);

				if (drEventData["EVENT_CALL_TYPE_CODE"].ToString() == "11")
				{
					callFlag = 1;
				}
				else
				{
					callFlag = 2;
				}
			}
			else
			{
				sprintf(s.msg, "事件号[%s]物料[%s]产线类型[%s]的调用类型[%s]是修改类为配置&单独函数,请联系MM配置相应的单独函数!",
					(const char*)eventId, (const char*)matKind, (const char*)eventLineType, (const char*)drEventData["EVENT_CALL_TYPE_CODE"].ToString());
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}

		//新增修改前物料数据表
		if (bcls_rec->Tables.IndexOf("OLDMM_TABLE") < 0)
		{
			bcls_rec->Tables.Add("OLDMM_TABLE");
		}

		//新增物料事件数据表
		if (bcls_rec->Tables.IndexOf("NEWMM_TABLE") < 0)
		{
			bcls_rec->Tables.Add("NEWMM_TABLE");
		}
		else
		{
			bcls_rec->Tables["NEWMM_TABLE"].Clear();
		}

		if (bcls_ret->Tables.Contains("MAT_TRACK_CALL"))
		{
			bcls_ret->Tables["MAT_TRACK_CALL"].Clear();
		}

		PrintLog("***************表结构初始化开始***************");
		if (bcls_rec->Tables.Contains("MAT_DATA") && bcls_rec->Tables["MAT_DATA"].Rows.get_Count() > 0)
		{
			dtMatDataColumn = bcls_rec->Tables["MAT_DATA"];
		}
		else
		{
			dtMatDataColumn = GetTableColName2("TMM" + matKind + "01", conn);
		}

		if (bcls_rec->Tables.Contains("MAT_TRACE") && bcls_rec->Tables["MAT_TRACE"].Rows.get_Count() > 0)
		{
			dtMatTraceColumn = bcls_rec->Tables["MAT_TRACE"];
		}
		else
		{
			dtMatTraceColumn = GetTableColName2("TMM" + matKind + "96", conn);
		}

		/*if (bcls_rec->Tables.Contains("MAT_TREE") && bcls_rec->Tables["MAT_TREE"].Rows.get_Count() > 0)
		{
			dtMatTreeColumn = bcls_rec->Tables["MAT_TREE"];
		}
		else
		{
			dtMatTreeColumn = GetTableColName2("TMM0005", conn);
		}

		tmm0005.SetTableName("TMM0005", dtMatTreeColumn);
		tmm0005.AddFilterColName("MAT_ID");*/

#if defined _LINE_HP
		//定义中厚板目的材料表
		CDynaTable matAimData(conn);
		CDataTable dtHP02;
		if (bcls_rec->Tables.Contains("MAT_AIM_DATA") && bcls_rec->Tables["MAT_AIM_DATA"].Rows.get_Count() > 0)
		{
			dtHP02 = bcls_rec->Tables["MAT_AIM_DATA"];
		}
		else
		{
			dtHP02 = GetTableColName2("TMMHP02", conn);
		}

		matAimData.SetTableName("TMMHP02", dtHP02);
		matAimData.AddFilterColName("MAT_NO");
#endif

		if (drEventData["EVENT_PROC_WAY_3"].ToString() == "2")
		{
			//定义物料数据表名
			matData.SetTableName("HMM" + matKind + "01", dtMatDataColumn);
			matData.AddFilterColName("MAT_ID");

			//定义物料跟踪履历表名
			matTrace.SetTableName("HMM" + matKind + "96", dtMatTraceColumn);
		}
		else
		{
			//定义物料数据表名
			matData.SetTableName("TMM" + matKind + "01", dtMatDataColumn);
			matData.AddFilterColName("MAT_NO");

			//定义物料数据历史表名
			matDataHistory.SetTableName("HMM" + matKind + "01", dtMatDataColumn);

			//定义物料跟踪履历表名
			matTrace.SetTableName("TMM" + matKind + "96", dtMatTraceColumn);
		}

		PrintLog("***************表结构初始化结束***************");

		matTrace.AddHoldColName("REC_CREATOR");
		matTrace.AddHoldColName("REC_CREATE_TIME");
		matTrace.AddHoldColName("REC_REVISOR");
		matTrace.AddHoldColName("REC_REVISE_TIME");
		matTrace.AddHoldColName("REC_ERASOR");
		matTrace.AddHoldColName("REC_ERASE_TIME");
		matTrace.AddHoldColName("ARCHIVE_FLAG");

		for (int i = 0; i < dtReceive.Rows.get_Count(); i++)
		{
			matNo = dtReceive.Rows[i]["MAT_NO"].ToString().Trim();
			if (matNo == "")
			{
				sprintf(s.msg, "材料号不能为空", (const char*)matNo);
				throw CApplicationException(-1, s.msg, log.Location);
			}

			Log::Trace("", __FUNCTION__, "处理第[{0}]行数据，材料号[{1}]，共[{2}]行记录", i, matNo, dtReceive.Rows.get_Count());

			if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
			{
				PrintLog("打印传入物料跟踪数据");
				PrintDataTable(dtReceive, i);
			}

			//不是物料回档或者新增事件,如果有传入源物料数据则取传入,没有则查询数据库
			int existsFlag = 0;

			if (drEventData["EVENT_PROC_WAY_1"].ToString() != "1" &&
				drEventData["EVENT_PROC_WAY_2"].ToString() != "1" &&
				drEventData["EVENT_PROC_WAY_2"].ToString() != "2" &&
				drEventData["EVENT_PROC_WAY_2"].ToString() != "3")
			{
				if (drEventData["EVENT_PROC_WAY_3"].ToString() == "2")
				{
					//修改历史表，查询最后一条历史表数据
					CDataTable dtMatHistory;
					strSql = "SELECT * FROM HMM" + matKind + "01 WHERE mat_no = '" + matNo + "' ORDER BY mat_id DESC";
					cmd.SetCommandText(strSql);
					cmd.ExecuteQuery(dtMatHistory);
					cmd.Close();

					if (dtMatHistory.Rows.get_Count() == 0)
					{
						sprintf(s.msg, "材料号[%s]不在历史表中,无法修改", (const char*)matNo);
						throw CApplicationException(-1, s.msg, log.Location);
					}

					matData.MergeFrom(dtMatHistory.Rows[0]);
					matData.MergeTo(bcls_rec->Tables["OLDMM_TABLE"], 0, true, true);
				}
				else
				{
					if (bcls_rec->Tables["OLDMM_TABLE"].Rows.get_Count() > 0)
					{
						//PrintDataTable(bcls_rec->Tables["OLDMM_TABLE"]);
						for (int j = 0; j < bcls_rec->Tables["OLDMM_TABLE"].Rows.get_Count() > 0; j++)
						{
							if (bcls_rec->Tables["OLDMM_TABLE"].Rows[j]["MAT_NO"].ToString() == matNo)
							{
								matData.MergeFrom(bcls_rec->Tables["OLDMM_TABLE"].Rows[j]);
								PrintLog("有传入源物料数据");
								existsFlag = 1;
								break;
							}
						}
					}

					PrintLog("existsFlag", existsFlag);
					if (existsFlag == 0)
					{
						//用材料号查询物料主表写入物料类
						matData.SetFilterColVal("MAT_NO", matNo);
						if (matData.Query() > 0)
						{
							//物料主表数据保存
							matData.MergeTo(bcls_rec->Tables["OLDMM_TABLE"], 0, true, true);
							//PrintDataTable(bcls_rec->Tables["OLDMM_TABLE"]);
						}
						else
						{
							sprintf(s.msg, "材料号[%s]的物料信息不存在", (const char*)matNo);
							throw CApplicationException(-1, s.msg, log.Location);
						}
					}

					//源物料数据校验
					doFlag = f_mm0099_00(&matData, eventId);
					if (doFlag < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
			}

			//EVENT_PROC_WAY_1	物料回档
			//EVENT_PROC_WAY_2	物料新增
			//EVENT_PROC_WAY_3	物料修改
			//EVENT_PROC_WAY_4	物料状态
			//EVENT_PROC_WAY_5	物料履历
			//EVENT_PROC_WAY_6	物料归档
			//EVENT_PROC_WAY_7	物料删除

			//物料回档
			if (drEventData["EVENT_PROC_WAY_1"].ToString() == "1" && drEventData["EVENT_CALL_TYPE_CODE"].ToString() != "2")
			{
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					Log::Trace("", __FUNCTION__, "按事件处理规则进行处理,第1位 - 物料回档,材料号[{0}];记录行数[{1}];", matNo, i + 1);
				}

				matData.SetFilterColVal("MAT_NO", matNo);
				if (matData.QueryCount() > 0)
				{
					sprintf(s.msg, "材料号[%s]在物料主档中,无法回档", (const char*)matNo);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//查询最后一条历史表数据
				matDataHistory.ClearFilterColName();
				matDataHistory.SetFilterColVal("MAT_NO", matNo);
				matDataHistory.AddOrderByDescColName("MAT_ID");
				if (matDataHistory.Query() <= 0)
				{
					sprintf(s.msg, "材料号[%s]不在历史表中,无法回档", (const char*)matNo);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				bcls_rec->Tables["OLDMM_TABLE"].Rows.Add();
				MergDataRow(matDataHistory.GetDataRow(),
					bcls_rec->Tables["OLDMM_TABLE"].Rows[bcls_rec->Tables["OLDMM_TABLE"].Rows.get_Count() - 1], true, true);

				//历史表数据回档
				matData.MergeFrom(matDataHistory.GetDataRow());
				if (matData.Insert(4) < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				//历史表表数据删除
				matDataHistory.AddFilterColName("MAT_ID");
				if (matDataHistory.Delete() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				//履历表数据回档新增
				strSql = "INSERT INTO TMM" + matKind + "96 SELECT * FROM HMM" + matKind + "96 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					PrintLog("履历表数据回档新增", strSql);
				}

				cmd.SetCommandText(strSql);
				cmd.ExecuteNonQuery();
				cmd.Close();

				//履历表数据回档历史表删除
				strSql = "DELETE FROM HMM" + matKind + "96 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					PrintLog("履历表数据回档历史表删除", strSql);
				}

				cmd.SetCommandText(strSql);
				cmd.ExecuteNonQuery();
				cmd.Close();

#if defined _LINE_HP
				if (matKind == "SM")
				{
					//目的材料数据回档新增
					strSql = "INSERT INTO TMMSM03 SELECT * FROM HMMSM03 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据回档", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//目的材料数据回档历史表删除
					strSql = "DELETE FROM HMMSM03 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据回档历史表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据回档新增
					strSql = "INSERT INTO TMMSM04 SELECT * FROM HMMSM04 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据回档新增", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据历史表删除
					strSql = "DELETE FROM HMMSM04 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据历史表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}

				if (matKind == "HP")
				{
					//目的材料数据回档新增
					strSql = "INSERT INTO TMMHP02 SELECT * FROM HMMHP02 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据回档", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//目的材料数据回档历史表删除
					strSql = "DELETE FROM HMMHP02 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据回档历史表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据回档新增
					strSql = "INSERT INTO TMMHP03 SELECT * FROM HMMHP03 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据回档新增", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据历史表删除
					strSql = "DELETE FROM HMMHP03 WHERE mat_id = '" + matDataHistory.GetColValString("MAT_ID") + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据历史表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
#endif
			}

			//物料新增
			if (drEventData["EVENT_PROC_WAY_2"].ToString() != "0" && drEventData["EVENT_CALL_TYPE_CODE"].ToString() != "2")
			{
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					Log::Trace("", __FUNCTION__, "按事件处理规则进行处理,第2位 - 物料新增,材料号[{0}];记录行数[{1}];", matNo, i + 1);
				}

				//检查物料是否已存在
				matData.SetFilterColVal("MAT_NO", matNo);
				if (matData.QueryCount() > 0)
				{
					sprintf(s.msg, "材料号[%s]已存在,无法新增", (const char*)matNo);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//将传入数据块写写入物料类
				matData.MergeFrom(dtReceive.Rows[i]);

				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					matData.Print();
				}

				//新增物料数据校验
				doFlag = f_mm0099_00(&matData, eventId);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (GetColValueC(dtReceive, i, "MAT_ID").Trim() == "")
				{
					//生成MAT_ID
					CString seqName = "MM00_MAT_ID";
					CString matId = CDateTime::Now().ToString("yyyyMMddHHmmss") + GetSeqence(seqName, 4, conn);
					//PrintLog("MAT_ID", matId);
					matData.SetColVal("MAT_ID", matId);

					if (drEventData["EVENT_PROC_WAY_2"].ToString() != "1")
					{
						matData.SetColVal("MAT_TRACK_NO", matId);
					}
				}

				if (matData.GetColValDecimal("MAT_NUM") == 0)
				{
					matData.SetColVal("MAT_NUM", (CDecimal)1);
					matData.SetColVal("MAT_TUBE", (CDecimal)1);
				}

				if (matData.GetColValDecimal("PASS_BACKLOG_SEQ_NO") == 0)
				{
					matData.SetColVal("PASS_BACKLOG_SEQ_NO", (CDecimal)1);
				}

				/*if (matData.GetColValString("PRODUCT_FLAG").Trim() == "")
				{
					matData.SetColVal("PRODUCT_FLAG", "0");
				}*/

				if (matData.GetColValString("PRODUCT_PACK_FLAG").Trim() == "")
				{
					matData.SetColVal("PRODUCT_PACK_FLAG", "0");
				}

				if (matData.GetColValString("HOLD_FLAG").Trim() == "")
				{
					matData.SetColVal("HOLD_FLAG", "0");
				}

				if (matData.GetColValString("TRANSFER_FLAG").Trim() == "")
				{
					matData.SetColVal("TRANSFER_FLAG", "0");
				}

				if (matData.GetColValString("CONFM_FLAG").Trim() == "")
				{
					matData.SetColVal("CONFM_FLAG", "0");
				}

				if (matData.GetColValString("PCH_JUDGE_ABN").Trim() == "")
				{
					matData.SetColVal("PCH_JUDGE_ABN", "0");
				}

				if (matData.GetColValString("COMPLEX_DECIDE_CODE").Trim() == "")
				{
					matData.SetColVal("COMPLEX_DECIDE_CODE", "0");
				}

				if (matData.GetColValString("APP_DECIDE_FLAG").Trim() == "")
				{
					matData.SetColVal("APP_DECIDE_FLAG", "0");
				}

				if (matData.GetColValString("IN_FLAG").Trim() == "")
				{
					matData.SetColVal("IN_FLAG", "0");
				}

				if (matData.GetColValString("DUMMY_COIL_FLAG").Trim() == "")
				{
					matData.SetColVal("DUMMY_COIL_FLAG", "0");
				}

				if (matData.GetColValString("RETURN_MAT_FLAG").Trim() == "")
				{
					matData.SetColVal("RETURN_MAT_FLAG", "0");
				}

				if (matData.GetColValString("COLD_HOT_FLAG").Trim() == "")
				{
					matData.SetColVal("COLD_HOT_FLAG", "0");
				}

				if (matData.GetColValString("REPAIR_FLAG").Trim() == "")
				{
					matData.SetColVal("REPAIR_FLAG", "0");
				}

				if (matData.GetColValString("SURFACE_DECIDE_CODE").Trim() == "")
				{
					matData.SetColVal("SURFACE_DECIDE_CODE", "1");
					matData.SetColVal("SURFACE_DECIDE_TIME", CDateTime::Now().ToString("yyyyMMddHHmmss"));
					matData.SetColVal("SURFACE_DECIDE_MAKER", (CString)s.userid);
				}
			}

			//物料修改
			if (drEventData["EVENT_PROC_WAY_3"].ToString() != "0" && drEventData["EVENT_CALL_TYPE_CODE"].ToString() != "2")
			{
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					Log::Trace("", __FUNCTION__, "按事件处理规则进行处理,第3位 - 物料修改,材料号[{0}];记录行数[{1}];", matNo, i + 1);
				}

				for (int j = 0; j < bcls_rec->Tables["EVENT_PARA"].Rows.get_Count(); j++)
				{
					CDataRow &drEventPara = bcls_rec->Tables["EVENT_PARA"].Rows[j];
					if (drEventPara["ITEM_UPD_MODE"].ToString() != "0")
					{
						CString itemEName = drEventPara["ITEM_ENAME"].ToString();
						CString itemCName = drEventPara["ITEM_CNAME"].ToString();
						if (!matData.GetDataTable().Columns.Contains(itemEName))
						{
							strcpy(s.msg, "TMM0099表中修改类的字段[" + itemEName + "]必须是主档表中的字段!");
							throw CApplicationException(-1, s.msg, log.Location);
						}

						CString itemProcValue = drEventPara["ITEM_PROC_VALUE"].ToString().Trim();
						CString itemCheckValue = drEventPara["ITEM_CHECK_VALUE"].ToString().Trim();

						if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
						{
							PrintLog("itemEName", itemEName);
							PrintLog("itemProcValue", itemProcValue);
						}

						//TMM0099中字段列ITEM_ENAME 字段修改方式 
						//1 - 接口值;
						//11- 接口值为@@则不修改,否则按接口值修改;
						//2 - 固定值;
						//3 - 表中字段;
						//4 - 特殊处理
						//5 - 按字段设置值中字段的接口值修改
						//tmm0099.Print("ITEM_UPD_MODE", j);
						if (drEventPara["ITEM_UPD_MODE"].ToString().Substring(0, 1) == "1")
						{
							//PrintLog("接口值");

							//校验 MM0099 块中必须有该列
							//if (!dtReceive.Columns.Contains(itemEName))
							//{
							//	strcpy(s.msg, "MM0099块中必须有列[" + itemCName + "][" + itemEName + "]!");
							//	throw CApplicationException(-1, s.msg, log.Location);
							//}

							//校验传入块MM0099中接口参数不可为空的字段必须有值
							if (drEventPara["ITEM_PARA_ALLOW_NULL"].ToString() == "N")
							{
								//接口参数不可为空
								if (drEventPara["ITEM_KIND"].ToString() == "S")	//S-字符型
								{
									if (dtReceive.Rows[i][itemEName].ToString().Trim() == "")
									{
										strcpy(s.msg, "传入参数校验错误!材料号[" + matNo + "]字段[" + itemCName + "][" + itemEName + "]不允许为空!");
										throw CApplicationException(-1, s.msg, log.Location);
									}
								}
								else	//D-数值型
								{
									if (dtReceive.Rows[i][itemEName].ToDecimal() == 0)
									{
										strcpy(s.msg, "传入参数校验错误!材料号[" + matNo + "]字段[" + itemCName + "][" + itemEName + "]不允许为0!");
										throw CApplicationException(-1, s.msg, log.Location);
									}
								}
							}

							if (dtReceive.Columns.Contains(itemEName))
							{
								int updFlag = 0;

								//设置主表修改字段,修改方式为1-接口值为字符型  默认 @@则不修改，数值型-9999则不修改
								if (drEventPara["ITEM_UPD_MODE"].ToString() == "1")
								{
									if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
									{
										Log::Trace("", __FUNCTION__, "字段修改方式为11-接口值为@@则不修改,字段[{0}],值[{1}]",
											itemEName, dtReceive.Rows[i][itemEName].ToString());
									}

									if (drEventPara["ITEM_KIND"].ToString() == "S" && dtReceive.Rows[i][itemEName].ToString().Trim() == "@@")
									{
										if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
										{
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
									else if (drEventPara["ITEM_KIND"].ToString() == "D" && dtReceive.Rows[i][itemEName].ToDecimal() == -9999)
									{
										if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
										{
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
									else
									{
										updFlag = 1;
									}
								}
								//设置主表修改字段,修改方式为11-接口值为字符型 @@则不修改，数值型-9999则不修改
								else if (drEventPara["ITEM_UPD_MODE"].ToString() == "11")
								{
									if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
									{
										Log::Trace("", __FUNCTION__, "字段修改方式为11-接口值为@@则不修改,字段[{0}],值[{1}]",
											itemEName, dtReceive.Rows[i][itemEName].ToString());
									}

									if (drEventPara["ITEM_KIND"].ToString() == "S" && dtReceive.Rows[i][itemEName].ToString().Trim() == "@@")
									{
										if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
										{
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
									else if (drEventPara["ITEM_KIND"].ToString() == "D" && dtReceive.Rows[i][itemEName].ToDecimal() == -9999)
									{
										if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
										{
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
									else
									{
										updFlag = 1;
									}
								}
								//设置主表修改字段,修改方式为12-接口值为字符型 主表当前值非空则不修改，数值型-主表当前值非0则不修改
								else if (drEventPara["ITEM_UPD_MODE"].ToString() == "12")
								{
									if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
									{
										Log::Trace("", __FUNCTION__, "字段修改方式为12-主表当前值非空非0则不修改,字段[{0}],传入值[{1}]",
											itemEName, dtReceive.Rows[i][itemEName].ToString());
										matData.Print(itemEName);
									}

									if (drEventPara["ITEM_KIND"].ToString() == "S" && matData.GetColValString(itemEName).Trim() != "")
									{
										if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
										{
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
									else if (drEventPara["ITEM_KIND"].ToString() == "D" && matData.GetColValDecimal(itemEName) != 0)
									{
										if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
										{
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
									else
									{
										updFlag = 1;
									}
								}
								//设置主表修改字段,修改方式为13-主表当前数据前拼接口值，修改方式为14-主表当前数据后拼接口值，若超长截位原数据
								else if (drEventPara["ITEM_KIND"].ToString() == "S" &&
									(drEventPara["ITEM_UPD_MODE"].ToString() == "13" || drEventPara["ITEM_UPD_MODE"].ToString() == "14"))
								{
									if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
									{
										Log::Trace("", __FUNCTION__, "字段修改方式为主表当前值拼接接口值，字段[{0}],传入值[{1}]",
											itemEName, dtReceive.Rows[i][itemEName].ToString());
										matData.Print(itemEName);
									}

									CString tableName = "TMM" + matKind + "01";
									switch (conn->DatabaseKind)
									{
									case DB_KIND_MSSQL:	        // MS SQL Server数据库
										strSql = "SELECT max_length FROM sys.columns WHERE object_id = object_id('" + tableName +
											"') AND name = '" + itemEName + "'";
										break;
									case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
									case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
										strSql = "SELECT length FROM sysibm.syscolumns WHERE tbname ='" + tableName +
											"' AND name = '" + itemEName + "' AND tbcreator = (SELECT current schema FROM sysibm.sysdummy1)";
										break;
									case DB_KIND_ORACLE:	    // Oracle 数据库
									default:
										strSql = "SELECT data_length FROM user_tab_columns WHERE table_name = '" + tableName +
											"' AND column_name = '" + itemEName + "'";
										break;
									}

									cmd.SetCommandText(strSql);
									CDecimal itemLength = cmd.ExecuteScalar();
									cmd.Close();
									PrintLog("itemLength", itemLength);

									if (matData.GetColValString(itemEName).Trim() == "")
									{
										matData.CopyColVal(itemEName, dtReceive.Rows[i]);
									}
									else
									{
										CString strSource = matData.GetColValString(itemEName);
										list <CString> listStrSplit;

										while (strSource.Find("/") > 0)
										{
											listStrSplit.push_back(strSource.Substring(0, strSource.Find("/")));
											strSource = strSource.Substring(strSource.Find("/") + 1);
										}

										if (strSource.Trim() != "")
										{
											listStrSplit.push_back(strSource);
										}

										CString strMerge = dtReceive.Rows[i][itemEName].ToString();
										if (drEventPara["ITEM_UPD_MODE"].ToString() == "13")
										{
											for (list<CString>::const_iterator iter = listStrSplit.begin(); iter != listStrSplit.end(); iter++)
											{
												PrintLog("NOW LENGTH", strMerge.GetLength() + 1 + (*iter).GetLength());
												if (strMerge.GetLength() + 1 + (*iter).GetLength() > itemLength)
												{
													break;
												}

												strMerge += "/" + *iter;
											}
										}
										else
										{
											for (list<CString>::reverse_iterator iter = listStrSplit.rbegin(); iter != listStrSplit.rend(); iter++)
											{
												if (strMerge.GetLength() + 1 + (*iter).GetLength() > itemLength)
												{
													break;
												}

												strMerge = *iter + "/" + strMerge;
											}
										}

										PrintLog("strMerge", strMerge);
										matData.SetColVal(itemEName, strMerge);
									}

									updFlag = 2;
								}
								else
								{
									updFlag = 1;
								}

								if (updFlag >= 1)
								{
									if (updFlag == 1)
									{
										//写修改值
										matData.CopyColVal(itemEName, dtReceive.Rows[i]);

										/************* 此段不删 说明 BEGIN ************/
										/* 下列代码 保留 可以替换 matData.CopyColVal(itemEName, dtReceive.Rows[i]); 这句话，
										   一旦这句话调用耗时，则用下面这段代码替换*/
										/*
										if (dtReceive.Columns[itemEName].get_DataType() == DT_STRING)
										{
											matData.SetColVal(itemEName, dtReceive.Rows[i][itemEName].ToString());
										}
										else
										{
											matData.SetColVal(itemEName, dtReceive.Rows[i][itemEName].ToDecimal());
										}
										*/
										/************* 此段不删 说明 END ************/
									}

									//设置修改列
									matData.AddUpdateColName(itemEName);

									if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
									{
										Log::Trace("", __FUNCTION__, "加载修改列[{0}]", itemEName);
										matData.Print(itemEName);
									}

									/*if (tmm0005.Contains(itemEName))
									{
										tmm0005.CopyColVal(itemEName, matData.GetDataRow(0));
										tmm0005.AddUpdateColName(itemEName);
									}*/
								}
							}
						}
						else if (drEventPara["ITEM_UPD_MODE"].ToString().Trim() == "2")		//2-固定值
						{
							/* 获取该字段的字段设置值 */
							if (itemProcValue == "")
							{
								strcpy(s.msg, "TMM0099表中字段[" + itemEName + "]字段修改方式是固定值,字段设置值[" + itemProcValue + "]必须有值!");
								throw CApplicationException(-1, s.msg, log.Location);
							}

							if (itemProcValue == "@")		//固定值为空
							{
								if (drEventPara["ITEM_KIND"].ToString() == "S")	//S-字符型
								{
									//PrintLog("置空", itemEName);
									matData.SetColVal(itemEName, " ");
								}
								else
								{
									matData.SetColVal(itemEName, (CDecimal)0);
								}
							}
							else if (itemProcValue == "s.userid")		//固定值为登录者
							{
								matData.SetColVal(itemEName, (CString)s.userid);
							}
							else if (itemProcValue == "s.datetime")		//固定值为系统时刻 14位
							{
								matData.SetColVal(itemEName, CDateTime::Now().ToString("yyyyMMddHHmmss"));
							}
							else
							{
								matData.CopyColVal(itemEName, drEventPara, "ITEM_PROC_VALUE");
							}

							//设置修改列
							matData.AddUpdateColName(itemEName);

							/*if (tmm0005.Contains(itemEName))
							{
								tmm0005.CopyColVal(itemEName, matData.GetDataRow(0));
								tmm0005.AddUpdateColName(itemEName);
							}*/

						}
						else if (drEventPara["ITEM_UPD_MODE"].ToString().Substring(0, 1) == "3")		//3-表中字段    31-非空表中字段
						{
							//获取该字段的字段设置值
							if (itemProcValue == "")
							{
								strcpy(s.msg, "TMM0099表中字段[" + itemEName + "]字段修改方式是表中字段,字段设置值[" +
									itemProcValue + "]必须有值!");
								throw CApplicationException(-1, s.msg, log.Location);
							}

							if (!matData.GetDataTable().Columns.Contains(itemProcValue))
							{
								strcpy(s.msg, "TMM0099表中表中字段的字段[" + itemEName + "]字段设置值[" + itemProcValue +
									"]必须为主档表中的字段!");
								throw CApplicationException(-1, s.msg, log.Location);
							}


							//获取字段值
							int noCopyFlag = 0;
							if (bcls_rec->Tables["OLDMM_TABLE"].Rows.get_Count() > 0)
							{
								CDataRow& drMat = bcls_rec->Tables["OLDMM_TABLE"].Rows[bcls_rec->Tables["OLDMM_TABLE"].Rows.get_Count() - 1];
								if (itemEName == "OLD_" + itemProcValue)
								{
									// itemEName = OLD_ORDER_NO,itemProcValue = ORDER_NO，OLD_ORDER_NO的原值 = ORDER_NO的原值，则OLD_ORDER_NO不修改
									// SET OLD_ORDER_NO = ORDER_NO
									if (drEventPara["ITEM_KIND"].ToString() == "S")
									{
										if (drMat[itemProcValue].ToString() == matData.GetColValString(itemEName))
										{
											noCopyFlag = 1;
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
									else
									{
										if (drMat[itemProcValue].ToDecimal() == matData.GetColValDecimal(itemEName))
										{
											noCopyFlag = 1;
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
								}

								if (drEventPara["ITEM_UPD_MODE"].ToString().Trim() == "31")  //31-非空表中字段
								{
									// itemEName = OLD_ORDER_NO,itemProcValue = ORDER_NO，ORDER_NO的原值为空,则OLD_ORDER_NO不修改，否则OLD_ORDER_NO值也会被清空
									// SET OLD_ORDER_NO = ORDER_NO
									if (drEventPara["ITEM_KIND"].ToString() == "S")
									{
										if (drMat[itemProcValue].ToString().Trim() == "")
										{
											noCopyFlag = 1;
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
									else
									{
										if (drMat[itemProcValue].ToDecimal() == 0)
										{
											noCopyFlag = 1;
											Log::Trace("", __FUNCTION__, "字段[{0}]不修改", itemEName);
										}
									}
								}
								matData.CopyColVal(itemEName, drMat, itemProcValue);
							}
							else
							{
								matData.CopyColVal(itemEName, itemProcValue);
							}

							if (noCopyFlag == 0)
							{
								//设置修改列
								matData.AddUpdateColName(itemEName);

								/*if (tmm0005.Contains(itemEName))
								{
									tmm0005.CopyColVal(itemEName, matData.GetDataRow(0));
									tmm0005.AddUpdateColName(itemEName);
								}*/
							}
						}
						else if (drEventPara["ITEM_UPD_MODE"].ToString().Trim() == "4")		//4-特殊处理
						{
							if (itemEName.Trim() == "HOLD_FLAG")
							{
								if (itemProcValue == "")
								{
									//传入块中无此列或无传入值则不处理
									if (!dtReceive.Columns.Contains("HOLD_FLAG") || dtReceive.Rows[i]["HOLD_FLAG"].ToString().Trim() == "")
									{
										continue;
									}

									itemProcValue = dtReceive.Rows[i]["HOLD_FLAG"].ToString();
									//tmm0099.CopyColVal("ITEM_PROC_VALUE", dtReceive.Rows[i]["HOLD_FLAG"].ToString());
								}
								else if (itemProcValue != "P0" &&					//P0-管理释放
									itemProcValue != "Q0" &&					//Q0-质量释放
									itemProcValue != "1" &&					//1-管理封锁
									itemProcValue != "2")						//2-质量封锁
								{
									strcpy(s.msg, "HOLD_FLAG的值[" + itemProcValue + "]必须为P0/Q0/1/2!");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								//主档表当前封锁标记是0-释放
								if (matData.GetColValString("HOLD_FLAG").Trim() == "0")
								{
									//设置 封锁标记 
									if (itemProcValue == "P0" ||					//P0-管理释放
										itemProcValue == "Q0")						//Q0-质量释放	
									{
										matData.SetColVal("HOLD_FLAG", "0");		//0-释放
									}
									else											//事件要求封锁标记为 1-管理封锁,2-质量封锁
									{
										matData.SetColVal("HOLD_FLAG", itemProcValue);
									}
								}
								//主档表当前封锁标记是1-管理封锁
								else if (matData.GetColValString("HOLD_FLAG").Trim() == "1")
								{
									//设置封锁标记
									if (itemProcValue == "P0")						//事件要求封锁标记为 P0-管理释放
									{
										matData.SetColVal("HOLD_FLAG", "0");		//0-释放
									}
									else if (itemProcValue == "Q0")					//事件要求封锁标记为 Q0-质量释放
									{
										matData.SetColVal("HOLD_FLAG", "1");		//1-管理封锁
									}
									else if (itemProcValue == "1")					//事件要求封锁标记为 1-管理封锁
									{
										matData.SetColVal("HOLD_FLAG", "1");		//1-管理封锁
									}
									else 											//事件要求封锁标记为 2-质量封锁
									{
										matData.SetColVal("HOLD_FLAG", "3");		//3-双重封锁
									}
								}
								//主档表当前封锁标记是2-质量封锁
								else if (matData.GetColValString("HOLD_FLAG").Trim() == "2")
								{
									//设置 封锁标记
									if (itemProcValue == "P0")						//事件要求封锁标记为 P0-管理释放
									{
										matData.SetColVal("HOLD_FLAG", "2");		//2-质量封锁
									}
									else if (itemProcValue == "Q0")					//事件要求封锁标记为 Q0-质量释放
									{
										matData.SetColVal("HOLD_FLAG", "0");		//0-释放
									}
									else if (itemProcValue == "1")					//事件要求封锁标记为 1-管理封锁
									{
										matData.SetColVal("HOLD_FLAG", "3");		//3-双重封锁
									}
									else 											//事件要求封锁标记为 2-质量封锁
									{
										matData.SetColVal("HOLD_FLAG", "2");		//2-质量封锁
									}
								}
								//主档表当前封锁标记是3-双重封锁
								else if (matData.GetColValString("HOLD_FLAG").Trim() == "3")
								{
									//设置 封锁标记
									if (itemProcValue == "P0")						//事件要求封锁标记为 P0-管理释放
									{
										matData.SetColVal("HOLD_FLAG", "2");		//2-质量封锁
									}
									else if (itemProcValue == "Q0")					//事件要求封锁标记为 Q0-质量释放
									{
										matData.SetColVal("HOLD_FLAG", "1");		//1-管理封锁
									}
									//else if (itemProcValue == "1")					//事件要求封锁标记为 1-管理封锁
									//{
									//	matData.SetColVal("HOLD_FLAG", "3");		//3-双重封锁
									//}
									else											//事件要求封锁标记为 2-质量封锁
									{
										matData.SetColVal("HOLD_FLAG", "3");		//3-双重封锁
									}
								}
								else
								{
									strcpy(s.msg, "材料号[" + matNo + "]的当前封锁标记[" + matData.GetColValString("HOLD_FLAG") + "]错误!");
									throw CApplicationException(-1, s.msg, log.Location);
								}

								matData.AddUpdateColName("HOLD_FLAG");

								/*tmm0005.CopyColVal("HOLD_FLAG", matData.GetDataRow(0));
								tmm0005.AddUpdateColName("HOLD_FLAG");*/
							}
							else if (itemEName.Trim() == "NEXT_WHOLE_BACKLOG_CODE")
							{
								//固定值为PRODUCT_FLAG 则传入参数中NEXT_WHOLE_BACKLOG_CODE值是9A 成品标记是1-成品，否则是0-在制品
								if (itemProcValue == "PRODUCT_FLAG")
								{
									//传入块中无此列或无传入值则报错
									if (!dtReceive.Columns.Contains("NEXT_WHOLE_BACKLOG_CODE") || dtReceive.Rows[i]["NEXT_WHOLE_BACKLOG_CODE"].ToString().Trim() == "")
									{
										strcpy(s.msg, "没有传入下工序代码!");
										throw CApplicationException(-1, s.msg, log.Location);
									}


									//先注释，后面待确认   mfj  20240131   太钢定制
									/*if (dtReceive.Rows[i]["NEXT_WHOLE_BACKLOG_CODE"].ToString().Trim() == "9A")
									{
										matData.SetColVal("PRODUCT_FLAG", "1");
									}
									else
									{
										matData.SetColVal("PRODUCT_FLAG", "0");
									}*/
									//厚板产线特殊判断
									if (matKind.Trim() == "HP")
									{
										// 固定值为PRODUCT_FLAG 传入参数中NEXT_WHOLE_BACKLOG_CODE值是9开头并且不是91工序,比如材合-92,准发-9A,成品标记是1,否则成品标记是0
										// 厚板判形合后,成品标记是1.
										if (dtReceive.Rows[i]["NEXT_WHOLE_BACKLOG_CODE"].ToString().Trim().Substring(0, 1) == "9"
											&&	dtReceive.Rows[i]["NEXT_WHOLE_BACKLOG_CODE"].ToString().Trim() != "91")
										{
											matData.SetColVal("PRODUCT_FLAG", "1");
										}
										else
										{
											matData.SetColVal("PRODUCT_FLAG", "0");
										}
									}

									matData.CopyColVal("NEXT_WHOLE_BACKLOG_CODE", dtReceive.Rows[i]);

									//设置修改列
									matData.AddUpdateColName("PRODUCT_FLAG");
									matData.AddUpdateColName("NEXT_WHOLE_BACKLOG_CODE");

									/*tmm0005.CopyColVal("PRODUCT_FLAG", matData.GetDataRow(0));
									tmm0005.AddUpdateColName("PRODUCT_FLAG");*/
								}
							}
							else if (itemEName.Trim() == "PRODUCT_FLAG")
							{
								//先注释，后面待确认   mfj  20240131   太钢定制
								//材料主档当前下工序是9A,成品标记是1,否则成品标记是0
								/*if (matData.GetColValString("NEXT_WHOLE_BACKLOG_CODE").Trim() == "9A")
								{
									matData.SetColVal("PRODUCT_FLAG", "1");
								}
								else
								{
									matData.SetColVal("PRODUCT_FLAG", "0");
								}*/

								//厚板产线特殊判断
								if (matKind.Trim() == "HP")
								{
									// 材料主档当前下工序是9开头并且不是91工序,比如材合-92,准发-9A,成品标记是1,否则成品标记是0
									// 厚板判形合后,成品标记是1.
									if (matData.GetColValString("NEXT_WHOLE_BACKLOG_CODE").Trim().Substring(0, 1) == "9"
										&&	matData.GetColValString("NEXT_WHOLE_BACKLOG_CODE").Trim() != "91")
									{
										matData.SetColVal("PRODUCT_FLAG", "1");
									}
									else
									{
										matData.SetColVal("PRODUCT_FLAG", "0");
									}
								}

								//设置修改列
								matData.AddUpdateColName("PRODUCT_FLAG");

								/*tmm0005.CopyColVal("PRODUCT_FLAG", matData.GetDataRow(0));
								tmm0005.AddUpdateColName("PRODUCT_FLAG");*/
							}
							else if (itemEName.Trim() == "")	//其他字段 可定制
							{
							}
							else
							{
								strcpy(s.msg, "TMM0099表中字段[" + itemEName + "]必须有值!");
								throw CApplicationException(-1, s.msg, log.Location);
							}
						}
						else if (drEventPara["ITEM_UPD_MODE"].ToString().Trim() == "5")		//5-按字段设置值中字段的接口值修改
						{
							//PrintLog("itemProcValue", itemProcValue);

							//获取该字段的字段设置值
							if (itemProcValue == "")
							{
								strcpy(s.msg, "TMM0099表中字段[" + itemEName + "]的修改方式是不需传入的接口值,字段设置值[" +
									itemProcValue + "]必须有值, 并且是本事件的字段修改方式为接口值的字段之一!");
								throw CApplicationException(-1, s.msg, log.Location);
							}

							int fitFlag = 0;
							for (int k = 0; k < bcls_rec->Tables["EVENT_PARA"].Rows.get_Count(); k++)
							{
								if (bcls_rec->Tables["EVENT_PARA"].Rows[k]["ITEM_ENAME"].ToString() == itemProcValue)
								{
									//PrintLog("ITEM_UPD_MODE", tmm0099.GetColValString("ITEM_UPD_MODE", k));

									if (bcls_rec->Tables["EVENT_PARA"].Rows[k]["ITEM_UPD_MODE"].ToString().Trim() != "" &&
										bcls_rec->Tables["EVENT_PARA"].Rows[k]["ITEM_UPD_MODE"].ToString().Substring(0, 1) == "1")
									{
										fitFlag = 1;
									}

									break;
								}
							}

							if (fitFlag == 0)
							{
								strcpy(s.msg, "TMM0099表中字段[" + itemEName + "]的修改方式是按字段设置值中字段的接口值修改, 字段设置值[" +
									itemProcValue + "]必须是本事件的字段修改方式为接口值的字段之一!");
								throw CApplicationException(-1, s.msg, log.Location);
							}

							matData.CopyColVal(itemEName, dtReceive.Rows[i], itemProcValue);
							if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")	//Y-打印履历
							{
								Log::Trace("", __FUNCTION__, "字段的值 - 按字段设置值中字段的接口值修改 S-字符型 {0}===[{1}]",
									itemEName, matData.GetColValString(itemEName));
							}

							//设置修改列
							matData.AddUpdateColName(itemEName);

							/*if (tmm0005.Contains(itemEName))
							{
								tmm0005.CopyColVal(itemEName, matData.GetDataRow(0));
								tmm0005.AddUpdateColName(itemEName);
							}*/
						}

#if defined _LINE_HP
						//TMM0099中字段列ITEM_CHECK_VALUE 暂定 厚板专用目的档字段
						if (matKind == "HP" && itemCheckValue != "")
						{
							if (!matAimData.GetDataTable().Columns.Contains(itemCheckValue))
							{
								strcpy(s.msg, "TMM0099表中厚板转用目的档字段[" + itemCheckValue + "]必须为目的档表中的字段!");
								throw CApplicationException(-1, s.msg, log.Location);
							}

							//设置修改列
							matAimData.CopyColVal(itemCheckValue, matData.GetDataRow(), itemEName);
							matAimData.AddUpdateColName(itemCheckValue);

							if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
							{
								Log::Trace("", __FUNCTION__, "中厚板转用目的档字段：ITEM_ENAME = [{0}]，ITEM_CHECK_VALUE = [{1}]",
									itemEName, itemCheckValue);
								matAimData.Print(itemCheckValue);
							}
						}
#endif
					}
				}
			}

			if (callFlag == 1)		//调用其他函数
			{
				if (bcls_rec->Tables.IndexOf("MAT_TRACK_CALL") < 0)
				{
					bcls_rec->Tables.Add("MAT_TRACK_CALL");
				}
				bcls_rec->Tables["MAT_TRACK_CALL"].Clear();
				matData.MergeTo(bcls_rec->Tables["MAT_TRACK_CALL"], 0, true, true);
				doFlag = f_epedcall(bcls_rec, bcls_ret);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				if (bcls_ret->Tables.Contains("MAT_TRACK_CALL") && bcls_ret->Tables["MAT_TRACK_CALL"].Rows.get_Count() > 0)
				{
					for (int j = 0; j < bcls_ret->Tables["MAT_TRACK_CALL"].Columns.get_Count(); j++)
					{
						CString retColName = bcls_ret->Tables["MAT_TRACK_CALL"].Columns[j].get_ColumnName();
						if (matData.Contains(retColName))
						{
							matData.CopyColVal(retColName, bcls_ret->Tables["MAT_TRACK_CALL"].Rows[0]);
							matData.AddUpdateColName(retColName);
						}

						/*if (tmm0005.Contains(retColName))
						{
							tmm0005.CopyColVal(retColName, bcls_ret->Tables["MAT_TRACK_CALL"].Rows[0]);
							tmm0005.AddUpdateColName(retColName);
						}*/
					}
				}
			}

			//物料状态
			if (drEventData["EVENT_PROC_WAY_4"].ToString() == "1")
			{
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					Log::Trace("", __FUNCTION__, "按事件处理规则进行处理,第4位 - 物料状态确定,材料号[{0}];记录行数[{1}];", matNo, i + 1);

					PrintLog("COLD_HOT_FLAG", matData.GetColValString("COLD_HOT_FLAG"));
					PrintLog("ORDER_NO", matData.GetColValString("ORDER_NO"));
					PrintLog("PRODUCT_FLAG", matData.GetColValString("PRODUCT_FLAG"));
					PrintLog("HOLD_FLAG", matData.GetColValString("HOLD_FLAG"));
					PrintLog("PLAN_NO", matData.GetColValString("PLAN_NO"));
					PrintLog("COMPLEX_DECIDE_CODE", matData.GetColValString("COMPLEX_DECIDE_CODE"));
					PrintLog("PRODUCT_PACK_FLAG", matData.GetColValString("PRODUCT_PACK_FLAG"));
					PrintLog("CONFM_FLAG", matData.GetColValString("CONFM_FLAG"));
					PrintLog("APP_DECIDE_FLAG", matData.GetColValString("APP_DECIDE_FLAG"));
					PrintLog("TRANSFER_FLAG", matData.GetColValString("TRANSFER_FLAG"));
					PrintLog("SURFACE_DECIDE_CODE", matData.GetColValString("SURFACE_DECIDE_CODE"));
				}

				doFlag = f_mm0099_04(&matData, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					Log::Trace("", __FUNCTION__, "材料状态确定为[{0}]", matData.GetColValString("MAT_STATUS"));
					if (matData.Contains("CUST_MAT_STATUS"))
					{
						Log::Trace("", __FUNCTION__, "股份材料状态确定为[{0}]", matData.GetColValString("CUST_MAT_STATUS"));
					}
				}

				//设置修改列
				matData.AddUpdateColName("MAT_STATUS");
				matData.AddUpdateColName("MAT_STATUS_UPDATE_TIME");
				matData.AddUpdateColName("CUST_MAT_STATUS");
				matData.AddUpdateColName("CUST_MAT_STATUS_UPDATE_TIME");

				/*tmm0005.CopyColVal("MAT_STATUS", matData.GetDataRow(0));
				tmm0005.AddUpdateColName("MAT_STATUS");*/
			}

			//物料履历
			matTrace.MergeFrom(drEventData);
			matTrace.MergeFrom(matData.GetDataRow());

			//根据配置决定是否自动生成履历责任者和履历时间
			for (int j = 0; j < bcls_rec->Tables["EVENT_PARA"].Rows.get_Count(); j++)
			{
				if (bcls_rec->Tables["EVENT_PARA"].Rows[j]["ITEM_ENAME"].ToString() == "REC_CREATOR")
				{
					if (bcls_rec->Tables["EVENT_PARA"].Rows[j]["ITEM_UPD_MODE"].ToString().Substring(0, 1) == "1" &&
						dtReceive.Columns.Contains("REC_CREATOR"))
					{
						eventUserId = dtReceive.Rows[i]["REC_CREATOR"].ToString();
						PrintLog("eventUserId", eventUserId);
					}

					break;
				}
			}

			for (int j = 0; j < bcls_rec->Tables["EVENT_PARA"].Rows.get_Count(); j++)
			{
				if (bcls_rec->Tables["EVENT_PARA"].Rows[j]["ITEM_ENAME"].ToString() == "REC_CREATE_TIME")
				{
					if (bcls_rec->Tables["EVENT_PARA"].Rows[j]["ITEM_UPD_MODE"].ToString().Substring(0, 1) == "1" &&
						dtReceive.Columns.Contains("REC_CREATE_TIME"))
					{
						eventTime = dtReceive.Rows[i]["REC_CREATE_TIME"].ToString();
						PrintLog("eventTime", eventTime);
					}

					break;
				}
			}

			if (eventTime.Trim() == "")
			{
				eventTime = CDateTime::Now().ToString("yyyyMMddHHmmss");
			}

			matTrace.SetColVal("REC_CREATOR", eventUserId);
			matTrace.SetColVal("REC_CREATE_TIME", eventTime);
			matTrace.SetColVal("REC_REVISOR", " ");
			matTrace.SetColVal("REC_REVISE_TIME", " ");
			matTrace.SetColVal("REC_ERASOR", " ");
			matTrace.SetColVal("REC_ERASE_TIME", " ");
			matTrace.SetColVal("ARCHIVE_FLAG", " ");
			matTrace.SetColVal("EVENT_DESC", eventDesc);
			matTrace.SetColVal("FORM_NAME", s.formname);
			matTrace.SetColVal("FUNC_ID", s.svc_name);
			matTrace.CopyColVal("SYSTEM_ID", dtReceive[i]);

			if (keyValue1.Trim() != "")
			{
				matTrace.SetColVal("KEYVALUE_1", keyValue1);
			}

			if (keyValue2.Trim() != "")
			{
				matTrace.SetColVal("KEYVALUE_2", keyValue2);
			}

			if (keyValue3.Trim() != "")
			{
				matTrace.SetColVal("KEYVALUE_3", keyValue3);
			}

			if (keyValue4.Trim() != "")
			{
				matTrace.SetColVal("KEYVALUE_4", keyValue4);
			}

			if (keyValue5.Trim() != "")
			{
				matTrace.SetColVal("KEYVALUE_5", keyValue5);
			}

			if (keyValue6.Trim() != "")
			{
				matTrace.SetColVal("KEYVALUE_6", keyValue6);
			}

			if (drEventData["EVENT_PROC_WAY_5"].ToString() == "1")
			{
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					Log::Trace("", __FUNCTION__, "按事件处理规则进行处理,第5位 - 记履历,材料号[{0}];记录行数[{1}];", matNo, i + 1);
				}

				CString callFuncCoding = matData.GetUpdateColString();
				if (callFuncCoding.GetLength() >= 300)
				{
					callFuncCoding = callFuncCoding.Substring(0, 299);
				}
				matTrace.SetColVal("CALL_FUNC_CODING", callFuncCoding);

				//生成履历流水号		
				matTrace.SetColVal("RESUME_SEQ_NO", GetTrackSeqNo("MM00_RESUME_SEQ_NO", 20, conn));

				//插入履历数据
				if (drEventData["EVENT_PROC_WAY_7"].ToString() == "1" &&
					dtReceive.Columns.Contains("REC_ERASE_TIME") && dtReceive.Rows[i]["REC_ERASE_TIME"].ToString().Trim() != "")
				{
					matTrace.CopyColVal("REC_ERASE_TIME", dtReceive.Rows[i]);
					matTrace.CopyColVal("REC_ERASOR", dtReceive.Rows[i]);
				}
				else
				{
					matTrace.SetColVal("REC_ERASE_TIME", CDateTime::Now().ToString("yyyyMMddHHmmss"));
					matTrace.SetColVal("REC_ERASOR", (CString)s.userid);
				}
			}

			//物料事件数据保存
			matTrace.MergeTo(bcls_rec->Tables["NEWMM_TABLE"], 0, true, true);
			

			//数据库处理(新增/修改)
			if (drEventData["EVENT_PROC_WAY_2"].ToString() != "0" && drEventData["EVENT_CALL_TYPE_CODE"].ToString() != "2")
			{
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					PrintLog("新增主表数据");
					matData.Print();
				}

				if (matData.Insert() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				PrintLog("新增主表数据end");

				
			}
			else if ((drEventData["EVENT_PROC_WAY_3"].ToString() != "0" || drEventData["EVENT_PROC_WAY_4"].ToString() == "1") &&
				matData.GetUpdateColString().Trim() != "")
			{
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					PrintLog("修改主表数据", matData.GetUpdateColString());
				}

				//修改主表数据
				matData.ClearFilterColName();
				if (drEventData["EVENT_PROC_WAY_3"].ToString() == "2")
				{
					matData.AddFilterColName("MAT_ID");
				}
				else
				{
					matData.AddFilterColName("MAT_NO");
				}

				//修改主表数据
				if (matData.Update() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
				Log::Info("", __FUNCTION__, "AAAAA 修改主表数据		matKind= [{0}]", matKind);

#if defined _LINE_HP
				if (matKind == "HP")
				{
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("修改目的材料表数据", matAimData.GetUpdateColString());
					}

					if (matAimData.GetUpdateColString().Trim() != "")
					{
						//修改目的材料表数据
						matAimData.SetFilterColVal("MAT_NO", matData.GetColValString("MAT_NO"));
						if (matAimData.Update() < 0)
						{
							throw CApplicationException(-1, s.msg, s.svc_name);
						}
					}
				}
#endif

				//if (eventId.Substring(0, 2) == "MM")
				//{
				//	if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				//	{
				//		PrintLog("修改物料跟踪树表", tmm0005.GetUpdateColString());
				//	}

				//	if (tmm0005.GetUpdateColString().Trim() != "")
				//	{
				//		//修改物料跟踪树表
				//		tmm0005.SetFilterColVal("MAT_ID", matData.GetColValString("MAT_ID"));
				//		if (tmm0005.Update() < 0)
				//		{
				//			throw CApplicationException(-1, s.msg, s.svc_name);
				//		}
				//	}
				//}
			}

			//物料归档
			if ((drEventData["EVENT_PROC_WAY_6"].ToString() == "1" || drEventData["EVENT_PROC_WAY_6"].ToString() == "2")
				&& drEventData["EVENT_CALL_TYPE_CODE"].ToString() != "2") //1-主档归档 2-主档/目的/工序档归档
			{
				matTrace.SetTableName("HMM" + matKind + "96");

				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					Log::Trace("", __FUNCTION__, "按事件处理规则进行处理,第6位 - 物料归档,材料号[{0}];记录行数[{1}];", matNo, i + 1);
				}

				//物料主表记录归档
				if (matData.GetRowCount() == 0)
				{
					sprintf(s.msg, "材料号[%s]不在主表中,无法归档", (const char*)matNo);
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//新增历史表数据
				matDataHistory.MergeFrom(matData.GetDataTable().Rows[0]);
				if (matDataHistory.Insert(1) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				//主表数据删除
				if (matData.Delete() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				//新增历史履历表数据
				strSql = "INSERT INTO HMM" + matKind + "96 SELECT * FROM TMM" + matKind + "96 WHERE mat_no = '" + matNo + "'";
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					PrintLog("新增历史履历表数据", strSql);
				}

				cmd.SetCommandText(strSql);
				cmd.ExecuteNonQuery();
				cmd.Close();

				//在线履历表数据删除
				strSql = "DELETE FROM TMM" + matKind + "96 WHERE mat_no = '" + matNo + "'";
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					PrintLog("在线履历表数据删除", strSql);
				}

				cmd.SetCommandText(strSql);
				cmd.ExecuteNonQuery();
				cmd.Close();

#if defined _LINE_HP
				if (matKind == "SM" && drEventData["EVENT_PROC_WAY_6"].ToString() == "2") //1-主档归档 2-主档/目的/工序档归档
				{
					//目的材料数据归档新增
					strSql = "INSERT INTO HMMSM03 SELECT * FROM TMMSM03 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据归档新增", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//目的材料数据回档在线表删除
					strSql = "DELETE FROM TMMSM03 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据归档在线表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据归档新增
					strSql = "INSERT INTO HMMSM04 SELECT * FROM TMMSM04 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据归档新增", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据在线表删除
					strSql = "DELETE FROM TMMSM04 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据在线表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}

				//如果是中厚板成品则将目的厚板表和小板通过工序表一同归档
				if (matKind == "HP" && drEventData["EVENT_PROC_WAY_6"].ToString() == "2") //1-主档归档 2-主档/目的/工序档归档
				{
					//目的材料数据归档新增
					strSql = "INSERT INTO HMMHP02 SELECT * FROM TMMHP02 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据归档新增", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//目的材料数据回档在线表删除
					strSql = "DELETE FROM TMMHP02 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据归档在线表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据归档新增
					strSql = "INSERT INTO HMMHP03 SELECT * FROM TMMHP03 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据归档新增", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据在线表删除
					strSql = "DELETE FROM TMMHP03 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据在线表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
#endif
			}

			//物料删除
			if (drEventData["EVENT_PROC_WAY_7"].ToString() == "1" && drEventData["EVENT_CALL_TYPE_CODE"].ToString() != "2")
			{
				matTrace.SetTableName("HMM" + matKind + "96");

				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					Log::Trace("", __FUNCTION__, "按事件处理规则进行处理,第7位 - 物料删除,材料号[{0}];记录行数[{1}];", matNo, i + 1);
				}

				//物料主表记录删除
				if (matData.Delete() < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				////物料跟踪树数据删除
				//tmm0005.SetFilterColVal("MAT_ID", matData.GetColValString("MAT_ID"));
				//if (tmm0005.Delete() < 0)
				//{
				//	throw CApplicationException(-1, s.msg, s.svc_name);
				//}

				//新增历史履历表数据
				strSql = "INSERT INTO HMM" + matKind + "96 SELECT * FROM TMM" + matKind + "96 WHERE mat_no = '" + matNo + "'";
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					PrintLog("新增历史履历表数据", strSql);
				}

				cmd.SetCommandText(strSql);
				cmd.ExecuteNonQuery();
				cmd.Close();

				//在线履历表数据删除
				strSql = "DELETE FROM TMM" + matKind + "96 WHERE mat_no = '" + matNo + "'";
				if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
				{
					PrintLog("在线履历表数据删除", strSql);
				}

				cmd.SetCommandText(strSql);
				cmd.ExecuteNonQuery();
				cmd.Close();

#if defined _LINE_HP
				if (matKind == "SM")
				{
					//目的材料数据回档在线表删除
					strSql = "DELETE FROM TMMSM03 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据归档在线表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据在线表删除
					strSql = "DELETE FROM TMMSM04 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据在线表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}

				if (matKind == "HP")
				{
					//目的材料数据回档在线表删除
					strSql = "DELETE FROM TMMHP02 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("目的材料数据归档在线表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();

					//材料工序数据在线表删除
					strSql = "DELETE FROM TMMHP03 WHERE mat_no = '" + matNo + "'";
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						PrintLog("材料工序数据在线表删除", strSql);
					}

					cmd.SetCommandText(strSql);
					cmd.ExecuteNonQuery();
					cmd.Close();
				}
#endif
			}

			//物料跟踪履历新增
			if (drEventData["EVENT_PROC_WAY_5"].ToString() == "1")
			{
				if (matTrace.Insert(4) < 0)
				{
					throw CApplicationException(-1, s.msg, s.svc_name);
				}
			}

			PrintLog("222end");

			//物料跟踪同步电文发送
			CString outSystemRec = "";
			CString outSystemSnd = "";
			tcSendFlag = 1;
			if ((drEventData["TC_SEND_FLAG"].ToString() == "Y0" || drEventData["TC_SEND_FLAG"].ToString() == "Y1") &&
				matTrace.GetRowCount() > 0)
			{
				//判断同步电文是否发送，如果此物料跟踪是接收电文程序调用，不再同步到源系统，防止循环同步
				strSql = "SELECT b.tc_no,b.outer_system FROM ted10 a,text1 b WHERE a.call_method = '" + (CString)s.svc_name +
					"' AND a.tc_no = b.tc_no";
				cmd.SetCommandText(strSql);
				cmd.ExecuteReader();
				while (cmd.Read())
				{
					tcNoRec = cmd.GetString(1);
					outSystemRec = cmd.GetString(2);
					Log::Trace("", __FUNCTION__, "接收电文[{0}]，外部系统[{1}]", tcNoRec, outSystemRec);
					break;
				}
				cmd.Close();
			}
			Log::Trace("", __FUNCTION__, "新增履历材料号[{0}]，外部系统[{1}]，tcSendFlag[{2}]", matNo, outSystemRec, tcSendFlag);
			Log::Trace("", __FUNCTION__, "TC_SEND_FLAG[{0}]，外部系统[{1}]，tcSendFlag[{2}]", drEventData["TC_SEND_FLAG"].ToString(), outSystemRec, tcSendFlag);
			if (drEventData["TC_SEND_FLAG"].ToString() == "Y0")
			{
				tcNo = drEventData["TC_NO"].ToString().Trim();
				tcSendFlag = 1;
				if (outSystemRec.Trim() != "")
				{
					strSql = "SELECT outer_system FROM text1 WHERE tc_no = '" + tcNo + "'";
					cmd.SetCommandText(strSql);
					cmd.ExecuteReader();
					if (cmd.Read())
					{
						outSystemSnd = cmd.GetString(1);
						Log::Trace("", __FUNCTION__, "发送电文[{0}]，外部系统[{1}]", tcNo, outSystemSnd);
					}
					cmd.Close();

					if (outSystemSnd == outSystemRec)
					{
						PrintLog("循环同步,不发电文");
						tcSendFlag = 0;

						/* 特殊处理 */
						if (tcNoRec.Trim() == "7000YA") //长材提出，该电文号需循环下发到L3
						{
							tcSendFlag = 1;
						}
						if (eventId.Trim() == "MM23") //材料产出配被归并合同号，需循环下发到L3
						{
							tcSendFlag = 1;
						}
					}
				}

				if (tcSendFlag == 1)
				{
					if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
					{
						Log::Trace("", __FUNCTION__, "材料号[{0}]向产供销发送电文[{1}];记录行数[{2}];", matNo, tcNo, i + 1);
					}

					EIClass bcls_rec_mat;
					matTrace.SetColVal("TC_NO", tcNo);
					bcls_rec_mat.Tables[0].Copy(matTrace.GetDataTable());
					bcls_rec_mat.Tables[0].set_TableName("BODY");
					PrintDataTable(bcls_rec_mat.Tables[0]);
					PrintLog(tcNo);
					if (!SendTelegram(tcNo, &bcls_rec_mat, conn, matNo))
					{
						throw CApplicationException(-1, s.msg, s.svc_name);
					}
				}
			}

			//其他电文发送
			if (drEventData["TC_SEND_FLAG"].ToString() == "Y1" && tcSendFlag == 1)
			{
				//新增电文配置数据表
				if (bcls_rec->Tables.IndexOf("MM00_MESSAGE") < 0)
				{
					bcls_rec->Tables.Add("MM00_MESSAGE");
				}

				CDynaTable tmm009a("TMM009A", conn);
				tmm009a.SetFilterColVal("EVENT_ID", eventId);
				tmm009a.SetFilterColVal("MAT_KIND", matKind);
				tmm009a.SetFilterColVal("EVENT_LINE_TYPE", eventLineType);
				tmm009a.SetFilterColVal("TC_SEND_FLAG", "Y");
				tmm009a.AddOrderByAscColName("TC_NO");
				if (tmm009a.Query() <= 0)
				{
					sprintf(s.msg, "事件号" + eventId + "物料" + matKind + "事件产线类型" + eventLineType + "在电文配置表中无数据!");
					throw CApplicationException(-1, s.msg, s.svc_name);
				}

				list<CString> listTcNo;
				list<CString> listTcNoRemove;
				for (int j = 0; j < tmm009a.GetRowCount(); j++)
				{
					CString colName = tmm009a.GetColValString("TC_KEYVALUE_ENAME", j);
					CString colValue = tmm009a.GetColValString("TC_KEYVALUE", j);
					CString colDataType = tmm009a.GetColValString("TC_KEYVALUE_TYPE", j);

					tcNo = tmm009a.GetColValString("TC_NO", j).Trim();
					if (tcNo.Trim() == "")
					{
						sprintf(s.msg, "事件号" + eventId + "物料" + matKind + "事件产线类型" + eventLineType + "的电文号是空,请录入电文号!");
						throw CApplicationException(-1, s.msg, s.svc_name);
					}
					Log::Trace("", __FUNCTION__, "新增履历材料号for tmm009a.GetRowCount 材料号[{0}]，colName[{1}]，colValue[{2}]，colDataType[{3}]，tcNo[{4}]"
						, matNo, colName, colValue, colDataType, tcNo);

					if (colName.Trim() == "" ||
						(colDataType == "D" && matTrace.GetColValDecimal(colName) == atof(colValue)) ||
						matTrace.GetColValString(colName) == colValue)
					{
						int addFlag = 1;
						for (list<CString>::const_iterator iter = listTcNo.begin(); iter != listTcNo.end(); iter++)
						{
							if (*iter == tcNo)
							{
								addFlag = 0;
								break;
							}
						}

						for (list<CString>::const_iterator iter = listTcNoRemove.begin(); iter != listTcNoRemove.end(); iter++)
						{
							if (*iter == tcNo)
							{
								addFlag = 0;
								break;
							}
						}

						if (addFlag == 1)
						{
							listTcNo.push_back(tcNo);
						}
					}
					else
					{
						int existsFlag = 0;
						for (list<CString>::const_iterator iter = listTcNoRemove.begin(); iter != listTcNoRemove.end(); iter++)
						{
							if (*iter == tcNo)
							{
								existsFlag = 1;
								break;
							}
						}

						if (existsFlag == 0)
						{
							listTcNoRemove.push_back(tcNo);
						}

						for (list<CString>::const_iterator iter = listTcNo.begin(); iter != listTcNo.end(); iter++)
						{
							if (*iter == tcNo)
							{
								listTcNo.remove(tcNo);
								break;
							}
						}
					}
				}

				for (list<CString>::const_iterator iter = listTcNo.begin(); iter != listTcNo.end(); iter++)
				{
					tcNo = *iter;
					Log::Trace("", __FUNCTION__, "新增履历材料号for list 材料号[{0}]，tcNo[{1}]，outSystemRec[{2}]，outSystemSnd[{3}]，tcNo[{4}]"
						, matNo, tcNo, outSystemRec, outSystemSnd, tcNo);

					tcSendFlag = 1;
					if (outSystemRec.Trim() != "")
					{
						strSql = "SELECT outer_system FROM text1 WHERE tc_no = '" + tcNo + "'";
						cmd.SetCommandText(strSql);
						cmd.ExecuteReader();
						if (cmd.Read())
						{
							outSystemSnd = cmd.GetString(1);
							Log::Trace("", __FUNCTION__, "发送电文[{0}]，外部系统[{1}]", tcNo, outSystemSnd);
						}
						cmd.Close();

						if (outSystemSnd == outSystemRec)
						{
							PrintLog("循环同步,不发电文");
							tcSendFlag = 0;
							Log::Trace("", __FUNCTION__, "发送电文[{0}]，外部系统[{1}]，外部系统(接收)[{2}]，tcSendFlag[{3}]", tcNo, outSystemSnd, outSystemRec, tcSendFlag);

							/* 特殊处理 */
							//if (tcNoRec.Trim() = "7000YA") //长材提出，该电文号需循环下发到L3  
							if (tcNoRec.Trim() == "7000YA") //长材提出，该电文号需循环下发到L3  
							{
								tcSendFlag = 1;
								Log::Trace("", __FUNCTION__, "tcNoRec[{0}]，tcSendFlag[{1}]", tcNoRec, tcSendFlag);
							}
							//if (eventId.Trim() = "MM23") //材料产出配被归并合同号，需循环下发到L3
							if (eventId.Trim() == "MM23") //材料产出配被归并合同号，需循环下发到L3
							{
								tcSendFlag = 1;
								Log::Trace("", __FUNCTION__, "eventId[{0}]，tcSendFlag[{1}]", eventId, tcSendFlag);
							}
						}
					}

					if (tcSendFlag == 1)
					{
						if (drEventData["EVENT_LOG_SWITCH"].ToString() == "Y")		//Y-打印履历
						{
							Log::Trace("", __FUNCTION__, "材料号[{0}]发送电文[{1}]", matNo, tcNo);
						}

						Log::Trace("", __FUNCTION__, "发送电文[{0}]，外部系统[{1}]，外部系统(接收)[{2}]，tcSendFlag[{3}]", tcNo, outSystemSnd, outSystemRec, tcSendFlag);

						EIClass bcls_rec_mat;
						matTrace.SetColVal("TC_NO", tcNo);
						bcls_rec_mat.Tables[0].Copy(matTrace.GetDataTable());
						bcls_rec_mat.Tables[0].set_TableName("BODY");
						if (!SendTelegram(tcNo, &bcls_rec_mat, conn, matNo))
						{
							throw CApplicationException(-1, s.msg, s.svc_name);
						}
					}
				}
			}
		}

		//记录仓库履历
		if (drEventData["EVENT_PROC_WAY_8"].ToString() == "1")
		{
			doFlag = f_mm0099_08(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}

#if defined _SYS_MMS || defined _SYS_MES    //MMS层或MES系统
		//成本抛帐处理, EVENT_PROC_WAY_AC: 0-不抛;1-事件配置;2-程序判断;3-外部调用成本,99不调用
		if (drEventData["EVENT_PROC_WAY_AC"].ToString() == "1" || drEventData["EVENT_PROC_WAY_AC"].ToString() == "2")
		{
			doFlag = f_mm0099_ac(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

		}
#endif

		//收发存抛帐处理
		if (drEventData["EVENT_PROC_WAY_KC"].ToString() == "1")
		{
			doFlag = f_mm0099_ym(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}

		if (callFlag == 2)		//调用其他函数
		{
			doFlag = f_epedcall(bcls_rec, bcls_ret);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
		}

		Log::Info("", __FUNCTION__, "NEWMM_TABLE 表数据条数		matKind= [{0}]", bcls_rec->Tables["NEWMM_TABLE"].Rows.get_Count());
		if (bcls_rec->Tables["NEWMM_TABLE"].Rows.get_Count() > 0) {
			doFlag = f_mmsm_t80rs0_snd(bcls_rec, bcls_ret, conn);
			if (doFlag < 0)
			{
				s.flag = 0;
				doFlag = 0;
			}
		}

		bcls_rec->Tables["OLDMM_TABLE"].Rows.Clear();
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

BM2_FUNCTION_EXPORT
int f_mmtp_mat_track(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CString matKind, CString eventId)
{
	CString eventLineType = GetColValueC(bcls_rec->Tables["MM0099"], 0, "EVENT_LINE_TYPE").Trim();
	return f_mmtp_mat_track(bcls_rec, bcls_ret, conn, matKind, eventId, eventLineType);
}

BM2_FUNCTION_EXPORT
int f_mmtp_mat_track(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CString matKind)
{
	CString eventId = GetColValueC(bcls_rec->Tables["MM0099"], 0, "EVENT_ID").Trim().ToUpper();
	CString eventLineType = GetColValueC(bcls_rec->Tables["MM0099"], 0, "EVENT_LINE_TYPE").Trim();
	return f_mmtp_mat_track(bcls_rec, bcls_ret, conn, matKind, eventId, eventLineType);
}

BM2_FUNCTION_EXPORT
int f_mmtp_mat_track(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CString matKind = GetColValueC(bcls_rec->Tables["MM0099"], 0, "MAT_KIND").Trim().ToUpper();
	CString eventId = GetColValueC(bcls_rec->Tables["MM0099"], 0, "EVENT_ID").Trim().ToUpper();
	CString eventLineType = GetColValueC(bcls_rec->Tables["MM0099"], 0, "EVENT_LINE_TYPE").Trim();
	return f_mmtp_mat_track(bcls_rec, bcls_ret, conn, matKind, eventId, eventLineType);
}

BM2_FUNCTION_EXPORT
int f_mmtp_mat_track(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CDataTable dtRecData,
CString matKind, CString eventId, CString eventLineType)
{
	if (dtRecData.Rows.get_Count() == 0)
	{
		strcpy(s.msg, "没有传入物料跟踪数据");
		throw CApplicationException(-1, s.msg, s.svc_name);
	}

	//检查输入参数合法性		
	if (matKind == "")
	{
		strcpy(s.msg, "没有传入物料类型");
		throw CApplicationException(-1, s.msg, s.svc_name);
	}

	if (eventId == "")
	{
		strcpy(s.msg, "没有传入事件号!");
		throw CApplicationException(-1, s.msg, s.svc_name);
	}

	if (bcls_rec->Tables.IndexOf("MM0099") < 0)
	{
		bcls_rec->Tables.Add("MM0099");
	}

	bcls_rec->Tables["MM0099"] = dtRecData;

	return f_mmtp_mat_track(bcls_rec, bcls_ret, conn, matKind, eventId, eventLineType);
}

BM2_FUNCTION_EXPORT
int f_mmtp_mat_track(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn, CDataTable dtRecData, CDataTable dtOldData,
CString matKind, CString eventId, CString eventLineType)
{
	if (dtRecData.Rows.get_Count() == 0)
	{
		strcpy(s.msg, "没有传入物料跟踪数据");
		throw CApplicationException(-1, s.msg, s.svc_name);
	}

	//检查输入参数合法性		
	if (matKind == "")
	{
		strcpy(s.msg, "没有传入物料类型");
		throw CApplicationException(-1, s.msg, s.svc_name);
	}

	if (eventId == "")
	{
		strcpy(s.msg, "没有传入事件号!");
		throw CApplicationException(-1, s.msg, s.svc_name);
	}

	if (bcls_rec->Tables.IndexOf("MM0099") < 0)
	{
		bcls_rec->Tables.Add("MM0099");
	}

	bcls_rec->Tables["MM0099"] = dtRecData;

	if (bcls_rec->Tables.IndexOf("OLDMM_TABLE") < 0)
	{
		bcls_rec->Tables.Add("OLDMM_TABLE");
	}

	bcls_rec->Tables["OLDMM_TABLE"] = dtOldData;

	return f_mmtp_mat_track(bcls_rec, bcls_ret, conn, matKind, eventId, eventLineType);
}

BM2_FUNCTION_EXPORT
int f_mmtp_mat_track(CString matNo, CString matKind, CString eventId, CString eventLineType, CDbConnection * conn)
{
	EIClass * bcls_rec;
	EIClass * bcls_ret;

	bcls_rec->Tables[0].set_TableName("MM0099");
	bcls_rec->Tables["MM0099"].Rows.Add();
	bcls_rec->Tables["MM0099"].Rows[0]["MAT_NO"] = matNo;

	return f_mmtp_mat_track(bcls_rec, bcls_ret, conn, matKind, eventId, eventLineType);
}