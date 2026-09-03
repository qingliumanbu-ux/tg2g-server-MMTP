#include "CDynaTable.h"

//-----------------------------------------------------------------------
//      概述:
//               实体对象类的构造函数。
//      参数:
//               conn                                      - 数据库连接
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CDynaTable::CDynaTable(CDbConnection* conn)
{
	this->cdbConn = conn;
	this->Reset();
}

//-----------------------------------------------------------------------
//      概述:
//               实体对象类的构造函数。
//      参数:
//				 tableName                                 - 数据库表名
//               conn                                      - 数据库连接
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CDynaTable::CDynaTable(const CString& tableName, CDbConnection* conn)
{
	this->cdbConn = conn;
	this->Reset();
	this->SetTableName(tableName);
}

//-----------------------------------------------------------------------
//      概述:
//               析构函数。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CDynaTable::~CDynaTable(void)
{

}

//-----------------------------------------------------------------------
//      概述:
//               清空类属性数据。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::Reset(int resetFlag)
{
	if (resetFlag == 0)
	{
		this->sTableName = "";
	}

	this->dtTable.Clear();
}

//-----------------------------------------------------------------------
//      概述:
//               设定数据库表名,初始化dtTable列名。
//      参数:
//				 tableName                                 - 数据库表名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetTableName(const CString& tableName)
{
	this->sTableName = tableName;
	PrintLog("SetTableName,TableName", this->sTableName);

	//添加dtTable列名
	if (this->sTableName.Substring(0, 1) == "&")
	{
		this->dtTableColumnM = SetDataTableColName("T" + this->sTableName.Substring(1), this->dtTable, this->cdbConn);
	}
	else
	{
		this->dtTableColumnM = SetDataTableColName(this->sTableName, this->dtTable, this->cdbConn);
	}

	this->dtTableM.Clone(this->dtTable);

	//将数据库表名设置为dtTable表名
	this->dtTable.set_TableName(this->sTableName);

	//添加数据库主键字段
	this->SetPkCols();
}

//-----------------------------------------------------------------------
//      概述:
//               设定数据库表名,初始化dtTable列名。
//      参数:
//				 tableName                                 - 数据库表名
//				 dtColumn                                  - 数据库表列
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetTableName(const CString& tableName, CDataTable& dtColumn)
{
	this->sTableName = tableName;
	this->dtTableColumnM = dtColumn;
	SetDataTableColName(dtColumn, this->dtTable, this->cdbConn);
	this->dtTableM.Clone(this->dtTable);

	//将数据库表名设置为dtTable表名
	this->dtTable.set_TableName(this->sTableName);

	//添加数据库主键字段
	//this->SetPkCols();
}

//-----------------------------------------------------------------------
//      概述:
//               设定连接数据库表名
//      参数:
//				 tableName                                 - 数据库表名
//				 sJoinType                                 - 连接类型(I:内连接,默认;L:左连接)
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetJoinTable(const CString& tableName1, const CString& tableName2, const CString& joinType)
{
	CTracer log(__FUNCTION__);

	this->sJoinTableName1 = tableName1;
	this->sJoinTableName2 = tableName2;
	this->sJoinType = joinType;

	if (this->sJoinTableName1.Trim() == "")
	{
		return;
	}
	else
	{
		//添加dtTable1列名
		if (this->sJoinTableName1.Substring(0, 1) == "&")
		{
			this->dtTableColumn1 = SetDataTableColName("T" + this->sJoinTableName1.Substring(1), this->dtTable1, this->cdbConn);
		}
		else
		{
			this->dtTableColumn1 = SetDataTableColName(this->sJoinTableName1, this->dtTable1, this->cdbConn);
		}

		//dtTable1列合并到dtTable
		MergeDataTableColumn(this->dtTable, this->dtTable1, this->cdbConn);

		if (this->sJoinTableName2.Trim() != "")
		{
			if (this->sJoinTableName2.Substring(0, 1) == "&")
			{
				this->dtTableColumn2 = SetDataTableColName("T" + this->sJoinTableName2.Substring(1), this->dtTable1, this->cdbConn);
			}
			else
			{
				this->dtTableColumn2 = SetDataTableColName(this->sJoinTableName2, this->dtTable2, this->cdbConn);
			}

			//dtTable1列合并到dtTable
			MergeDataTableColumn(this->dtTable, this->dtTable2, this->cdbConn);
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               添加联合数据库表名
//      参数:
//				 tableName                                 - 联合数据库表名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddUnionTable(const CString& tableName)
{
	for (list<CString>::const_iterator iter = this->listUnionTableName.begin(); iter != this->listUnionTableName.end(); iter++)
	{
		if (*iter == tableName)
		{
			return;
		}
	}

	this->listUnionTableName.push_back(tableName);
}

//-----------------------------------------------------------------------
//      概述:
//               设定联合数据库表名，添加列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetUnionTable()
{
	CTracer log(__FUNCTION__);

	if (this->listUnionTableName.size() > 0)
	{
		CDataTable dtUnionTable;
		PrintLog("dtTable列数", this->dtTable.Columns.get_Count());
		for (list<CString>::const_iterator iter = this->listUnionTableName.begin(); iter != this->listUnionTableName.end(); iter++)
		{
			CString tableName = *iter;
			if (tableName.Substring(0, 1) == "&")
			{
				SetDataTableColName("T" + tableName.Substring(1), dtUnionTable, this->cdbConn);
				MergeDataTableColumn(this->dtTable, dtUnionTable, this->cdbConn);
			}
		}

		this->sJoinType = "U";
	}
}

//-----------------------------------------------------------------------
//      概述:
//               设定数据库主键列(数据库表原主键)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetPkCols()
{
	this->listPkColName = GetTablePkColName(this->sTableName, this->cdbConn);
}

//-----------------------------------------------------------------------
//      概述:
//               添加数据库主键列名， 一次一个。
//      参数:
//				 colName                                   - 数据库主键列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddFilterColName(const CString& colName)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		for (list<CString>::const_iterator iter = this->listFilterColName.begin(); iter != this->listFilterColName.end(); iter++)
		{
			if (*iter == colName)
			{
				return;
			}
		}

		this->listFilterColName.push_back(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               清空数据库主键列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearFilterColName()
{
	this->listFilterColName.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加连接数据库表1连接列名， 一次一个。
//      参数:
//				 colName                                   - 连接数据库表1连接列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddJoinColName1(const CString& colName)
{
	for (list<CString>::const_iterator iter = this->listJoinColName1.begin(); iter != this->listJoinColName1.end(); iter++)
	{
		if (*iter == colName)
		{
			return;
		}
	}

	PrintLog("AddJoinColName1", colName);
	this->listJoinColName1.push_back(colName);
}

//-----------------------------------------------------------------------
//      概述:
//               清空连接数据库表1连接列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearJoinColName1()
{
	this->listJoinColName1.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加连接数据库表2连接列名， 一次一个。
//      参数:
//				 colName                                   - 连接数据库表1连接列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddJoinColName2(const CString& colName)
{
	for (list<CString>::const_iterator iter = this->listJoinColName2.begin(); iter != this->listJoinColName2.end(); iter++)
	{
		if (*iter == colName)
		{
			return;
		}
	}

	PrintLog("AddJoinColName2", colName);
	this->listJoinColName2.push_back(colName);
}

//-----------------------------------------------------------------------
//      概述:
//               清空连接数据库表2连接列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearJoinColName2()
{
	this->listJoinColName2.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加数据库查询列名， 一次一个。
//      参数:
//				 colName                                   - 数据库查询列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddSelectColName(const CString& colName)
{
	if (this->dtTableM.Columns.Contains(colName))
	{
		for (list<CString>::const_iterator iter = this->listSelectColName.begin(); iter != this->listSelectColName.end(); iter++)
		{
			if (*iter == colName)
			{
				return;
			}
		}

		this->listSelectColName.push_back(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               清空数据库查询列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearSelectColName()
{
	this->listSelectColName.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加连接数据库表1查询列名， 一次一个。
//      参数:
//				 colName                                   - 数据库查询列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddJoinSelectColName1(const CString& colName)
{
	for (list<CString>::const_iterator iter = this->listJoinSelectColName1.begin(); iter != this->listJoinSelectColName1.end(); iter++)
	{
		if (*iter == colName)
		{
			return;
		}
	}

	//PrintLog("AddJoinSelectColName1", colName);
	this->listJoinSelectColName1.push_back(colName);
}

//-----------------------------------------------------------------------
//      概述:
//               清空连接数据库表1查询列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearJoinSelectColName1()
{
	this->listJoinSelectColName1.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加连接数据库表2查询列名， 一次一个。
//      参数:
//				 colName                                   - 数据库查询列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddJoinSelectColName2(const CString& colName)
{
	for (list<CString>::const_iterator iter = this->listJoinSelectColName2.begin(); iter != this->listJoinSelectColName2.end(); iter++)
	{
		if (*iter == colName)
		{
			return;
		}
	}

	//PrintLog("AddJoinSelectColName2", colName);
	this->listJoinSelectColName2.push_back(colName);
}

//-----------------------------------------------------------------------
//      概述:
//               清空连接数据库表2查询列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearJoinSelectColName2()
{
	this->listJoinSelectColName2.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加数据库表查询分组字段， 一次一个。
//      参数:
//				 colName                                   - 数据库表查询分组列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddGroupByColName(const CString& colName)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		for (list<CString>::const_iterator iter = this->listGroupByColName.begin(); iter != this->listGroupByColName.end(); iter++)
		{
			if (*iter == colName)
			{
				return;
			}
		}

		this->listGroupByColName.push_back(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               清空数据库表查询分组列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearGroupByColName()
{
	this->listGroupByColName.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加数据库表查询正排序列名， 一次一个。
//      参数:
//				 colName                                   - 数据库表查询正排序列名1查询列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddOrderByAscColName(const CString& colName)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		for (list<CString>::const_iterator iter = this->listOrderByAscColName.begin(); iter != this->listOrderByAscColName.end(); iter++)
		{
			if (*iter == colName)
			{
				return;
			}
		}

		this->listOrderByAscColName.push_back(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               清空数据库表查询正排序列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearOrderByAscColName()
{
	this->listOrderByAscColName.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加数据库表查询逆排序列名， 一次一个。
//      参数:
//				 colName                                   - 数据库表查询逆排序列名1查询列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddOrderByDescColName(const CString& colName)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		for (list<CString>::const_iterator iter = this->listOrderByDescColName.begin(); iter != this->listOrderByDescColName.end(); iter++)
		{
			if (*iter == colName)
			{
				return;
			}
		}

		this->listOrderByDescColName.push_back(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               清空数据库表查询逆排序列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearOrderByDescColName()
{
	this->listOrderByDescColName.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加数据库修改列名， 一次一个。
//      参数:
//				 colName								   - 数据库修改列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddUpdateColName(const CString& colName)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		for (list<CString>::const_iterator iter = this->listUpdateColName.begin(); iter != this->listUpdateColName.end(); iter++)
		{
			if (*iter == colName)
			{
				return;
			}
		}

		for (list<CString>::const_iterator iter = this->listPkColName.begin(); iter != this->listPkColName.end(); iter++)
		{
			if (*iter == colName)
			{
				return;
			}
		}

		for (list<CString>::const_iterator iter = this->listFilterColName.begin(); iter != this->listFilterColName.end(); iter++)
		{
			if (*iter == colName)
			{
				return;
			}
		}

		this->listUpdateColName.push_back(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               清空数据库修改列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearUpdateColName()
{
	this->listUpdateColName.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               添加数据库保留列名， 一次一个, 保留列名在MergFrom时保持不变。
//      参数:
//				 colName								   - 数据库修改列名
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddHoldColName(const CString& colName)
{
	for (list<CString>::const_iterator iter = this->listHoldColName.begin(); iter != this->listHoldColName.end(); iter++)
	{
		if (*iter == colName)
		{
			return;
		}
	}

	this->listHoldColName.push_back(colName);
}

//-----------------------------------------------------------------------
//      概述:
//               清空数据库保留列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearHoldColName()
{
	this->listHoldColName.clear();
}

//-----------------------------------------------------------------------
//      概述:
//               根据查询列拼接SELECT语句。
//      返回值:
//               拼接的查询语句。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::BuildSelectString()
{
	CString strSelect = "SELECT ";

	for (list<CString>::const_iterator iter = this->listJoinSelectColName2.begin(); iter != this->listJoinSelectColName2.end(); iter++)
	{
		if (strSelect == "SELECT ")
		{
			strSelect += "t2." + *iter;
		}
		else
		{
			strSelect += ",t2." + *iter;
		}
	}

	for (list<CString>::const_iterator iter = this->listJoinSelectColName1.begin(); iter != this->listJoinSelectColName1.end(); iter++)
	{
		if (strSelect == "SELECT ")
		{
			strSelect += "t1." + *iter;
		}
		else
		{
			strSelect += ",t1." + *iter;
		}
	}

	if (this->listSelectColName.size() > 0)
	{
		for (list<CString>::const_iterator iter = this->listSelectColName.begin(); iter != this->listSelectColName.end(); iter++)
		{
			if (strSelect == "SELECT ")
			{
				strSelect += "t." + *iter;
			}
			else
			{
				strSelect += ",t." + *iter;
			}
		}
	}
	else
	{
		if (strSelect == "SELECT ")
		{
			strSelect += "t.*";
		}
		else
		{
			strSelect += ",t.*";
		}
	}

	return strSelect;
}

//-----------------------------------------------------------------------
//      概述:
//               根据查询数据表拼接FROM语句。
//      返回值:
//               拼接的查询语句。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::BuildFromString()
{
	CString strFrom = " FROM ";
	CString tableName = this->sTableName;
	CString strTable = this->sTableName;
	if (this->sTableName.Substring(0, 1) == "&")
	{
		tableName = "T" + this->sTableName.Substring(1);
		strTable = "(SELECT * FROM " + tableName + " UNION ALL SELECT * FROM H" + this->sTableName.Substring(1) + ")";
	}

	if (this->sJoinType == "U")
	{
		strFrom += " (" + SetInqSQL(tableName, this->dtTable, this->cdbConn);
		if (this->sTableName.Substring(0, 1) == "&")
		{
			strFrom += " UNION ALL " + SetInqSQL("H" + this->sTableName.Substring(1), this->dtTable, this->cdbConn);
		}

		for (list<CString>::const_iterator iter = this->listUnionTableName.begin(); iter != this->listUnionTableName.end(); iter++)
		{
			CString unionTableName = *iter;
			if (unionTableName.Substring(0, 1) == "&")
			{
				CString unionOnlineTableName = "T" + unionTableName.Substring(1);
				CString unionHistoryTableName = "H" + unionTableName.Substring(1);

				strFrom += " UNION ALL " + SetInqSQL(unionOnlineTableName, this->dtTable, this->cdbConn);
				strFrom += " UNION ALL " + SetInqSQL(unionHistoryTableName, this->dtTable, this->cdbConn);
			}
			else
			{
				strFrom += " UNION ALL " + SetInqSQL(unionTableName, this->dtTable, this->cdbConn);
			}
		}
		strFrom += ") t";
	}
	else
	{
		strFrom += strTable + " t";
		if (this->sJoinTableName1.Trim() != "")
		{
			CString tableName1 = this->sJoinTableName1;
			CString strTable1 = this->sJoinTableName1;

			if (this->sJoinTableName1.Substring(0, 1) == "&")
			{
				tableName1 = "T" + this->sJoinTableName1.Substring(1);
				strTable1 = "(SELECT * FROM " + tableName1 + " UNION ALL SELECT * FROM H" + this->sJoinTableName1.Substring(1) + ")";
			}

			if (this->sJoinType == "L")
			{
				strFrom += " LEFT OUTER JOIN " + strTable1;
			}
			else if (this->sJoinType == "R")
			{
				strFrom += " RIGHT OUTER JOIN " + strTable1;
			}
			else
			{
				strFrom += " INNER JOIN " + strTable1;
			}
			strFrom += " t1";

			if (this->listJoinColName1.size() > 0)
			{
				for (list<CString>::const_iterator iter = this->listJoinColName1.begin(); iter != this->listJoinColName1.end(); iter++)
				{
					if (iter == this->listJoinColName1.begin())
					{
						strFrom += " ON ";
					}
					else
					{
						strFrom += " AND ";
					}

					strFrom += *iter;
				}
			}
			else if (listPkColName.size() > 0)
			{
				for (list<CString>::const_iterator iter = this->listPkColName.begin(); iter != this->listPkColName.end(); iter++)
				{
					if (iter == this->listPkColName.begin())
					{
						strFrom += " ON ";
					}
					else
					{
						strFrom += " AND ";
					}

					strFrom += "t." + *iter + " = t1." + *iter;
				}
			}
		}

		if (this->sJoinTableName2.Trim() != "")
		{
			CString tableName2 = this->sJoinTableName2;
			CString strTable2 = this->sJoinTableName2;

			if (this->sJoinTableName2.Substring(0, 1) == "&")
			{
				tableName2 = "T" + this->sJoinTableName2.Substring(1);
				strTable2 = "(SELECT * FROM " + tableName2 + " UNION ALL SELECT * FROM H" + this->sJoinTableName2.Substring(1) + ")";
			}

			if (this->sJoinType == "L")
			{
				strFrom += " LEFT OUTER JOIN " + strTable2;
			}
			else if (this->sJoinType == "R")
			{
				strFrom += " RIGHT OUTER JOIN " + strTable2;
			}
			else
			{
				strFrom += " INNER JOIN " + strTable2;
			}
			strFrom += " t2";

			if (this->listJoinColName2.size() > 0)
			{
				for (list<CString>::const_iterator iter = this->listJoinColName2.begin(); iter != this->listJoinColName2.end(); iter++)
				{
					if (iter == this->listJoinColName2.begin())
					{
						strFrom += " ON ";
					}
					else
					{
						strFrom += " AND ";
					}

					strFrom += *iter;
				}
			}
			else if (listPkColName.size() > 0)
			{
				for (list<CString>::const_iterator iter = this->listPkColName.begin(); iter != this->listPkColName.end(); iter++)
				{
					if (iter == this->listPkColName.begin())
					{
						strFrom += " ON ";
					}
					else
					{
						strFrom += " AND ";
					}

					strFrom += "t." + *iter + " = t2." + *iter;
				}
			}
		}
	}

	return strFrom;
}

//-----------------------------------------------------------------------
//      概述:
//               根据筛选条件列拼接WHERE语句。
//      参数:
//               rowNum								   - CDataTable 行号(默认0)。
//      返回值:
//               拼接的条件语句。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::BuildWhereString(int rowNum)
{
	CString colName = "";
	CString colValue = "";
	CString condition = "";
	int iCount = 0;

	//PrintLog("过滤条件列数", listFilterColName.size());
	//PrintLog("主键列数", listPkColName.size());

	if (listFilterColName.size() > 0)
	{
		PrintLog("自定义条件列");
		for (list<CString>::const_iterator iter = this->listFilterColName.begin(); iter != this->listFilterColName.end(); iter++)
		{
			colName = *iter;
			colValue = this->dtTable.Rows[rowNum][colName].ToString();

			//PrintLog("colName", colName);
			//PrintLog("colValue", colValue);

			if (colValue == "")
			{
				continue;
			}

			//PrintLog("colName", colName);
			//PrintLog("colValue", colValue);

			if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
			{
				if (iCount > 0)
				{
					condition += " AND ";
				}

				condition += colName + " = '" + colValue + "'";
				iCount++;
			}
			else
			{
				if (iCount > 0)
				{
					condition += " AND ";
				}

				condition += colName + " = " + colValue + "";
				iCount++;
			}

			//PrintLog("condition", condition);
		}
	}
	else if (listPkColName.size() > 0)
	{
		for (list<CString>::const_iterator iter = this->listPkColName.begin(); iter != this->listPkColName.end(); iter++)
		{
			colName = *iter;
			PrintLog("colName", colName);

			colValue = this->dtTable.Rows[rowNum][colName].ToString();

			if (colValue == "")
			{
				continue;
			}

			//PrintLog("colName", colName);
			//PrintLog("colValue", colValue);

			if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
			{
				if (iCount > 0)
				{
					condition += " AND ";
				}

				condition += colName + " = '" + colValue + "'";
				iCount++;
			}
			else
			{
				if (colValue != "0")
				{
					if (iCount > 0)
					{
						condition += " AND ";
					}

					condition += colName + " = " + colValue + "";
					iCount++;
				}
			}
		}
	}
	else
	{
		PrintLog("无条件列");
	}

	return condition;
}

//-----------------------------------------------------------------------
//      概述:
//               根据根据传入条件数据行拼接WHERE语句。
//      参数:
//               rowCondition                          - 查询条件CDataRow 对象
//      返回值:
//               拼接的条件语句。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::BuildWhereString(CDataRow& rowCondition)
{
	CString strCondition = "";
	CString colName = "";
	CString colNameRec = "";
	CString colVal = "";
	CString strOperator = "";
	int iCount = 0;

	for (int i = 0; i < rowCondition.get_Table().Columns.get_Count(); i++)
	{
		colNameRec = rowCondition.get_Table().Columns[i].get_ColumnName().ToUpper();
		colVal = rowCondition[colNameRec].ToString();

		if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 1) == ">")
		{
			if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 2) == ">=")
			{
				colName = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(2).ToUpper();
				strOperator = " >= ";
			}
			else
			{
				colName = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(1).ToUpper();
				strOperator = " > ";
			}
		}
		else if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 1) == "<")
		{
			if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 2) == "<=")
			{
				colName = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(2).ToUpper();
				strOperator = " <= ";
			}
			else
			{
				colName = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(1).ToUpper();
				strOperator = " < ";
			}
		}
		else if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 1) == "%")
		{
			if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 2) == "%*" ||
				rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 2) == "%%")
			{
				colName = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(2).ToUpper();
				strOperator = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 2);
			}
			else
			{
				colName = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(1).ToUpper();
				strOperator = "%";
			}
		}
		else if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 1) == "^")
		{
			colName = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(1).ToUpper();
			strOperator = "^";
		}
		else if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 1) == "&")
		{
			strOperator = " INSTR ";
		}
		else if (rowCondition.get_Table().Columns[i].get_ColumnName().Substring(0, 1) == "=")
		{
			colName = rowCondition.get_Table().Columns[i].get_ColumnName().Substring(1).ToUpper();
			strOperator = " LIKE ";
		}
		else
		{
			colName = colNameRec;
			strOperator = " = ";
		}

		PrintLog("colName", colName);
		PrintLog("strOperator", strOperator);
		PrintLog("colVal", colVal);

		if (this->dtTableM.Columns.Contains(colName))
		{
			if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
			{
				if (colVal.Trim() != "")
				{
					if (iCount > 0)
					{
						strCondition += " AND ";
					}

					if (this->sJoinType == "I" || this->sJoinType == "L" || this->sJoinType == "R")
					{
						if (rowCondition.get_Table().Columns[i].get_Caption() == sJoinTableName1)
						{
							colName = "t1." + colName;
						}
						else if (rowCondition.get_Table().Columns[i].get_Caption() == sJoinTableName2)
						{
							colName = "t2." + colName;
						}
						else
						{
							colName = "t." + colName;
						}
					}

					if (strOperator == "%")
					{
						strCondition += colName + " LIKE " + "'" + colVal + "%'";
					}
					else if (strOperator == "%*")
					{
						strCondition += colName + " LIKE " + "'%" + colVal + "'";
					}
					else if (strOperator == "%%")
					{
						strCondition += colName + " LIKE " + "'%" + colVal + "%'";
					}
					else if (strOperator == "^")
					{
						strCondition += colName + " IN (" + colVal + ")";
					}
					else
					{
						strCondition += colName + strOperator + "'" + colVal + "'";
					}

					iCount++;
				}
			}
			else
			{
				if (colVal.Trim() != "0")
				{
					if (iCount > 0)
					{
						strCondition += " AND ";
					}

					if (this->sJoinType == "I" || this->sJoinType == "L")
					{
						colName = "t." + colName;
					}
					else if (this->sJoinType == "R")
					{
						colName = "t1." + colName;
					}

					strCondition += colName + strOperator + colVal;
					iCount++;
				}
			}
		}
		else if (this->sJoinType == "I" || this->sJoinType == "L" || this->sJoinType == "R")
		{
			PrintLog("连接查询");
			PrintDataTable(this->dtTable1);
			if (this->dtTable1.Columns.Contains(colName))
			{
				PrintLog("dtTable1 colName", colName);
				if (this->dtTable1.Columns[colName].get_DataType() == DT_STRING)
				{
					if (colVal.Trim() != "")
					{
						if (iCount > 0)
						{
							strCondition += " AND ";
						}

						if (strOperator == " LIKE ")
						{
							strCondition += "t1." + colName + " LIKE '" + colVal + "%'";
						}

						if (strOperator == "%")
						{
							strCondition += "t1." + colName + " LIKE '" + colVal + "%'";
						}
						else if (strOperator == "%*")
						{
							strCondition += "t1." + colName + " LIKE '%" + colVal + "'";
						}
						else if (strOperator == "%%")
						{
							strCondition += "t1." + colName + " LIKE '%" + colVal + "%'";
						}
						else if (strOperator == "^")
						{
							strCondition += "t1." + colName + " IN (" + colVal + ")";
						}
						else
						{
							strCondition += "t1." + colName + strOperator + "'" + colVal + "'";
						}

						iCount++;
					}
				}
				else
				{
					if (colVal.Trim() != "0")
					{
						if (iCount > 0)
						{
							strCondition += " AND ";
						}

						strCondition += "t1." + colName + strOperator + colVal;
						iCount++;
					}
				}
			}
			else if (this->dtTable2.Columns.Contains(colName))
			{
				PrintLog("dtTable2 colName", colName);
				if (this->dtTable2.Columns[colName].get_DataType() == DT_STRING)
				{
					if (colVal.Trim() != "")
					{
						if (iCount > 0)
						{
							strCondition += " AND ";
						}

						if (strOperator == "%")
						{
							strCondition += "t2." + colName + " LIKE '" + colVal + "%'";
						}
						else if (strOperator == "%*")
						{
							strCondition += "t2." + colName + " LIKE '%" + colVal + "'";
						}
						else if (strOperator == "%%")
						{
							strCondition += "t2." + colName + " LIKE '%" + colVal + "%'";
						}
						else if (strOperator == "^")
						{
							strCondition += "t2." + colName + " IN (" + colVal + ")";
						}
						else
						{
							strCondition += "t2." + colName + strOperator + "'" + colVal + "'";
						}

						iCount++;
					}
				}
				else
				{
					if (colVal.Trim() != "0")
					{
						if (iCount > 0)
						{
							strCondition += " AND ";
						}

						strCondition += "t2." + colName + strOperator + colVal;
						iCount++;
					}
				}
			}
		}
	}

	PrintLog("strCondition", strCondition);
	return strCondition;
}

//-----------------------------------------------------------------------
//      概述:
//               拼接GroupBy语句。
//      返回值:
//               拼接的GroupBy语句。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::BuildGroupByString()
{
	if (this->listGroupByColName.size() == 0)
	{
		return "";
	}

	CString strGroupBy = " GROUP BY ";
	for (list<CString>::const_iterator iter = this->listGroupByColName.begin(); iter != this->listGroupByColName.end(); iter++)
	{
		if (strGroupBy == " GROUP BY ")
		{
			strGroupBy += *iter;
		}
		else
		{
			strGroupBy += "," + *iter;
		}
	}

	return strGroupBy;
}

//-----------------------------------------------------------------------
//      概述:
//               拼接OrderBy语句。
//      返回值:
//               拼接的OrderBy语句。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::BuildOrderByString()
{
	if (this->listOrderByAscColName.size() + this->listOrderByDescColName.size() == 0)
	{
		return "";
	}

	CString strOrderBy = " ORDER BY ";
	CString colName = "";
	for (list<CString>::const_iterator iter = this->listOrderByAscColName.begin(); iter != this->listOrderByAscColName.end(); iter++)
	{
		if (this->sJoinType == "I" || this->sJoinType == "L")
		{
			if (this->dtTableM.Columns.Contains(*iter))
			{
				colName = "t." + *iter;
			}
			else if (this->dtTable1.Columns.Contains(*iter))
			{
				colName = "t1." + *iter;
			}
			else if (this->dtTable2.Columns.Contains(*iter))
			{
				colName = "t2." + *iter;
			}
		}
		else if (this->dtTable.Columns.Contains(*iter))
		{
			colName = "t." + *iter;
		}

		if (strOrderBy == " ORDER BY ")
		{
			strOrderBy += colName;
		}
		else
		{
			strOrderBy += "," + colName;
		}
	}

	//PrintLog("strOrderBy", strOrderBy);

	for (list<CString>::const_iterator iter = this->listOrderByDescColName.begin(); iter != this->listOrderByDescColName.end(); iter++)
	{
		if (this->sJoinType == "I" || this->sJoinType == "L")
		{
			if (this->dtTableM.Columns.Contains(*iter))
			{
				colName = "t." + *iter;
			}
			else if (this->dtTable1.Columns.Contains(*iter))
			{
				colName = "t1." + *iter;
			}
			else if (this->dtTable2.Columns.Contains(*iter))
			{
				colName = "t2." + *iter;
			}
		}
		else if (this->dtTable.Columns.Contains(*iter))
		{
			colName = "t." + *iter;
		}

		if (strOrderBy == " ORDER BY ")
		{
			strOrderBy += colName + " DESC";
		}
		else
		{
			strOrderBy += "," + colName + " DESC";
		}
	}

	//PrintLog("strOrderBy", strOrderBy);
	return strOrderBy;
}

//-----------------------------------------------------------------------
//      概述:
//               根据修改列拼接新增语句。
//      参数:
//               rowNum								   - CDataTable 行号(默认0)。
//      返回值:
//               拼接的修改语句。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::BuildInsertString(int rowNum)
{
	CString strIns = " (";
	CString colName = "";
	CString colCaption = "";
	CString dataType = "";
	int dataLength = 0;
	int dataPrecision = 0;
	int dataScale = 0;

	for (int i = 0; i < this->dtTableColumnM.Rows.get_Count(); i++)
	{
		colName = this->dtTableColumnM.Rows[i]["COLUMN_NAME"].ToString();
		if (i > 0)
		{
			strIns += ",";
		}
		strIns += colName;
	}

	strIns += ") VALUES (";
	for (int i = 0; i < this->dtTableColumnM.Rows.get_Count(); i++)
	{
		colName = this->dtTableColumnM.Rows[i]["COLUMN_NAME"].ToString();
		colCaption = this->dtTableColumnM.Rows[i]["COLUMN_CNAME"].ToString();
		dataType = this->dtTableColumnM.Rows[i]["DATA_TYPE"].ToString();
		//dataLength = this->dtTableColumnM.Rows[i]["DATA_PRECISION"].ToDecimal().ToInt32();
		dataScale = this->dtTableColumnM.Rows[i]["DATA_SCALE"].ToDecimal().ToInt32();

		//Log::Trace("", __FUNCTION__, "i[{0}],colName[{1}]", i, colName);
		//Log::Trace("", __FUNCTION__, "i[{0}],dataType[{1}]", i, dataType);

		//拼接Insert的Value语句，以CDataRow的数据为值
		if (dataType == "C")
		{
			//比较长度
			if (this->dtTable.Rows[rowNum][colName].ToString().GetLength() > dataLength)
			{
				//Log::Trace("", __FUNCTION__, "字段[{0}][{1}]的值[{2}]超长,最大长度为[{3}]",
				//	colName, colCaption, this->dtTable.Rows[rowNum][colName].ToString(), dataLength);
			}

			//Log::Trace("", __FUNCTION__, "--1-- i[{0}],col[{1}],value[{2}]", i, colName, this->dtTable.Rows[rowNum][colName].ToString());
			if (i == this->dtTableColumnM.Rows.get_Count() - 1)
			{
				if (this->dtTable.Rows[rowNum][colName].ToString().Trim() == "")
				{
					strIns += "' ')";
				}
				else
				{
					strIns += "'" + this->dtTable.Rows[rowNum][colName].ToString() + "')";
				}
			}
			else
			{
				if (this->dtTable.Rows[rowNum][colName].ToString().Trim() == "")
				{
					//Log::Trace("", __FUNCTION__, "列[{0}]为空", colName);
					strIns += "' ',";
				}
				else
				{
					//Log::Trace("", __FUNCTION__, "列[{0}]非空", colName);
					strIns += "'" + this->dtTable.Rows[rowNum][colName].ToString() + "',";
				}
			}

			//Log::Trace("", __FUNCTION__, "strIns[{0}]", strIns);
		}
		else
		{
			//按精度四舍五入
			CDecimal colValue = 0;

			if (this->dtTable.Rows[rowNum][colName].ToString().Trim() != "")
			{
				if (dataType == "I")
				{
					colValue = this->dtTable.Rows[rowNum][colName].ToDecimal().ToInt32();
				}
				else
				{
					colValue = this->dtTable.Rows[rowNum][colName].ToDecimal().Round(dataScale);
				}
			}

			if (colValue >= pow(10, dataLength - dataScale))
			{
				//Log::Trace("", __FUNCTION__, "字段[{0}]的值[{1}]超大,最大值为[{2}]", colName, colValue, dataLength - dataScale);
			}

			//Log::Trace("", __FUNCTION__, "--2-- i[{0}],col[{1}],value[{2}]", i, colName, this->dtTable.Rows[rowNum][colName].ToString());
			if (i == this->dtTableColumnM.Rows.get_Count() - 1)
			{
				strIns += colValue.ToString() + ")";
			}
			else
			{
				strIns += colValue.ToString() + ",";
			}

			//Log::Trace("", __FUNCTION__, "strIns[{0}]", strIns);
		}
	}

	return strIns;
}

//-----------------------------------------------------------------------
//      概述:
//               根据修改列拼接修改语句。
//      参数:
//               rowNum								   - CDataTable 行号(默认0)。
//      返回值:
//               拼接的修改语句。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::BuildUpdateString(int rowNum)
{
	CString colName = "";
	CString colValue = "";
	CString strUpd = "";
	int iCount = 0;

	for (list<CString>::const_iterator iter = this->listUpdateColName.begin(); iter != this->listUpdateColName.end(); iter++)
	{
		colName = *iter;
		colValue = this->dtTable.Rows[rowNum][colName].ToString();

		if (colValue == "")
		{
			continue;
		}

		if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
		{
			if (iCount > 0)
			{
				strUpd += ",";
			}

			strUpd += colName + " = '" + colValue + "'";
			iCount++;
		}
		else
		{
			if (iCount > 0)
			{
				strUpd += ",";
			}

			strUpd += colName + " = " + colValue + "";
			iCount++;
		}
	}

	return strUpd;
}

//-----------------------------------------------------------------------
//      概述:
//               初始化某行数据。
//      参数:
//               rowNum								   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::InitDataRow(int rowNum)
{
	if (this->dtTable.Rows.get_Count() - 1 < rowNum)
	{
		return;
	}

	for (int i = 0; i < this->dtTable.Columns.get_Count(); i++)
	{
		CString colName = this->dtTable.Columns[i].get_ColumnName();
		if (this->dtTable.Columns[i].get_DataType() == DT_STRING)
		{
			this->dtTable.Rows[rowNum][colName] = " ";
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)0;
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               移除某行数据。
//      参数:
//               rowNum								   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::RemoveDataRow(int rowNum, int rowCount)
{
	this->dtTable.Rows.Remove(rowNum, rowCount);
}

//-----------------------------------------------------------------------
//      概述:
//               移除所有行数据。
//      参数:
//               rowNum								   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::ClearDataRow()
{
	this->dtTable.Rows.Clear();
}

//-----------------------------------------------------------------------
//      概述:
//               根据条件condition查询表中满足条件的记录数。
//      参数:
//               condition                             - 查询条件
//      返回值:
//               满足条件的记录数目。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::QueryCount(const CString& condition)
{
	CTracer log(__FUNCTION__);

	CDbCommand cmd_inq(this->cdbConn);
	CString inqStr = "SELECT COUNT(1) " + this->BuildFromString();
	if (condition.Trim() != "")
	{
		inqStr += " WHERE " + condition;
	}

	PrintLog("inqStr", inqStr);

	try
	{
		cmd_inq.SetCommandText(inqStr);
		return cmd_inq.ExecuteScalar().ToInt32();
		cmd_inq.Close();
	}
	catch (CDbException& ex)
	{
		cmd_inq.Close();
		PrintLog("查询出错, inqStr", inqStr);
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode = [{0}],请联系开发人员", arguments, 1);
		return -1;
	}
}

//-----------------------------------------------------------------------
//      概述:
//               根据主键数据查询表中满足条件的记录数。
//      参数:
//               rowNum								   - CDataTable 行号(默认0)。
//      返回值:
//               满足条件的记录数目。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::QueryCount(int rowNum)
{
	CString condition = this->BuildWhereString(rowNum);
	PrintLog("condition", condition);
	return QueryCount(condition);
}

//-----------------------------------------------------------------------
//      概述:
//               根据条件查询表， 将结果保存在实体对象的CDataTable中。
//      参数:
//               orderBy                               - 排序语句(默认空)
//               rowNum								   - 查询条件字段所在行号(默认0)
//               recordFrom                            - 分页起始记录序号(默认空)
//               pageSize                              - 分页每页记录数(默认空)
//      返回值:
//               查询结果的记录数目， 如果查询出错则返回 -1。
BM2_FUNCTION_EXPORT int CDynaTable::Query(const CString& condition, int rowNum, int recordFrom, int pageSize)
{
	CTracer log(__FUNCTION__);

	//CDataTable dtQuery;
	CDbCommand cmd_inq(this->cdbConn);
	sSql = this->BuildSelectString() + this->BuildFromString();

	CString strCondition = "";
	if (condition.Trim() != "")
	{
		strCondition = condition;
	}
	else
	{
		strCondition = this->BuildWhereString(rowNum);
	}

	if (strCondition.Trim() != "")
	{
		sSql += " WHERE " + strCondition.Trim();
	}

	CString strOrderBy = this->BuildOrderByString();
	if (strOrderBy.Trim() != "")
	{
		sSql += strOrderBy;
	}

	int totalRecord = 0;
	this->dtTable.Rows.Clear();

	try
	{
		cmd_inq.SetCommandText(sSql);
		if (pageSize > 0)
		{
			cmd_inq.ExecuteQuery(this->dtTable, recordFrom, pageSize);
		}
		else
		{
			cmd_inq.ExecuteQuery(this->dtTable);
		}
		cmd_inq.Close();

		PrintLog("sSql", sSql);
		//PrintDataTable(this->dtTable);
	}
	catch (CDbException& ex)
	{
		cmd_inq.Close();
		PrintLog("查询出错,inqStr", sSql);
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode = [{0}],请联系开发人员", arguments, 1);
		return -1;
	}

	PrintLog("pageSize", pageSize);
	//for (int i = 0; i < dtQuery.Rows.get_Count(); i++)
	//{
	//	this->dtTable.Rows.Add();
	//	this->dtTable.Rows[i].Merge(dtQuery.Rows[i]);
	//}

	if (pageSize > 0)
	{
		totalRecord = this->QueryCount(condition);
	}
	else
	{
		totalRecord = this->dtTable.Rows.get_Count();
	}

	PrintLog("totalRecord", totalRecord);
	return totalRecord;
}

//-----------------------------------------------------------------------
//      概述:
//               根据条件condition查询表， 将结果保存在实体对象的CDataTable中。
//      参数:
//               rowCondition                          - 查询条件CDataRow 对象
//               recordFrom                            - 分页起始记录序号(默认空)
//               pageSize                              - 分页每页记录数(默认空)
//      返回值:
//               查询结果的记录数目， 如果查询出错则返回 -1。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::Query(CDataRow& rowCondition, int recordFrom, int pageSize)
{
	CTracer log(__FUNCTION__);

	CDataTable table = rowCondition.get_Table();
	//CDataTable dtQuery;

	CDbCommand cmd_inq(this->cdbConn);
	CString strCondition = this->BuildWhereString(rowCondition);
	//PrintLog("strCondition", strCondition);

	CString condition = this->BuildFromString();
	if (strCondition.Trim() != "")
	{
		condition += " WHERE " + strCondition;
	}

	sSql = this->BuildSelectString() + condition;

	CString strOrderBy = this->BuildOrderByString();
	if (strOrderBy.Trim() != "")
	{
		sSql += strOrderBy;
	}

	int totalRecord = 0;
	this->dtTable.Rows.Clear();

	try
	{
		cmd_inq.SetCommandText(sSql);

		if (pageSize > 0)
		{
			cmd_inq.ExecuteQuery(this->dtTable, recordFrom, pageSize);
		}
		else
		{
			cmd_inq.ExecuteQuery(this->dtTable);
		}
		cmd_inq.Close();

		if (sSql.GetLength() < 10000)
		{
			PrintLog("sSql", sSql);
		}
	}
	catch (CDbException& ex)
	{
		cmd_inq.Close();
		if (sSql.GetLength() < 10000)
		{
			PrintLog("查询出错,inqStr", sSql);
		}
		else
		{
			PrintLog("查询出错");
		}

		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode = [{0}],请联系开发人员", arguments, 1);
		return -1;
	}

	//for (int i = 0; i < dtQuery.Rows.get_Count(); i++)
	//{
	//	this->dtTable.Rows.Add();
	//	this->dtTable.Rows[i].Merge(dtQuery.Rows[i]);
	//}

	if (pageSize > 0)
	{
		totalRecord = this->QueryCount(condition);
	}
	else
	{
		totalRecord = this->dtTable.Rows.get_Count();
	}

	PrintLog("totalRecord", totalRecord);
	return totalRecord;
}

//-----------------------------------------------------------------------
//      概述:
//               将dtTable数据新增到数据库表中。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::Insert(int insertFlag)
{
	CTracer log(__FUNCTION__);
	PrintLog("Insert, RowsCount", this->dtTable.Rows.get_Count());

	int insCount = 0;
	CString insStr = "";
	CDbCommand cmd_ins(this->cdbConn);

	for (int i = 0; i < this->dtTable.Rows.get_Count(); i++)
	{
		if (insertFlag == 0)
		{
			this->SetColVal("REC_CREATOR", (CString)s.userid, i);
			this->SetColVal("REC_CREATE_TIME", CDateTime::Now().ToString("yyyyMMddHHmmss"), i);
			this->SetColVal("REC_REVISOR", " ", i);
			this->SetColVal("REC_REVISE_TIME", " ", i);
			this->SetColVal("REC_ERASOR", " ", i);
			this->SetColVal("REC_ERASE_TIME", " ", i);
			this->SetColVal("ARCHIVE_FLAG", " ", i);
		}
		else if (insertFlag == 1)
		{
			this->SetColVal("REC_ERASOR", (CString)s.userid, i);
			this->SetColVal("REC_ERASE_TIME", CDateTime::Now().ToString("yyyyMMddHHmmss"), i);
		}
		else if (insertFlag == 2)
		{
			this->SetColVal("REC_CREATOR", (CString)s.userid, i);
			this->SetColVal("REC_CREATE_TIME", CDateTime::Now().ToString("yyyyMMddHHmmss"), i);
		}
		else if (insertFlag == 3)
		{
			this->SetColVal("REC_REVISOR", (CString)s.userid, i);
			this->SetColVal("REC_REVISE_TIME", CDateTime::Now().ToString("yyyyMMddHHmmss"), i);
		}

		insStr = "INSERT INTO " + this->sTableName + this->BuildInsertString(i);
		cmd_ins.SetCommandText(insStr);
		PrintLog("insStr", insStr);
		try
		{
			int insCountSub = cmd_ins.ExecuteNonQuery();
			cmd_ins.Close();

			insCount += insCountSub;
		}
		catch (CDbException& ex)  //捕获数据库操作异常
		{
			PrintLog("insStr", insStr);
			//CString errorSql = insStr;
			//while (errorSql.GetLength() > 600)
			//{
			//	CString printSql = errorSql.Substring(0, 600);
			//	PrintLog("printSql", printSql);

			//	errorSql = errorSql.Substring(600);
			//}
			//
			//if (errorSql.GetLength() > 0)
			//{
			//	PrintLog("printSql", errorSql);
			//}

			cmd_ins.Close();
			CFormattable arguments[] = { ex.GetCode() };
			CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}],请联系开发人员", arguments, 1);
			insCount = -1;
			break;
		}
	}

	PrintLog("新增结束,insCount", insCount);
	return insCount;
}

//-----------------------------------------------------------------------
//      概述:
//               根据条件condition，从数据库更新满足条件的记录， 修改列取AddUpdateColName添加的列名。
//      参数:
//               condition                                       - 过滤条件
//      返回值:
//               被更新的记录数， 如果更新出错则返回 -1。              
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::Update(const CString& condition, int rowNum)
{
	CTracer log(__FUNCTION__);

	if (condition.Trim() == "")
	{
		strcpy(s.msg, "没有条件语句");
		return false;
	}

	int updCount = 0;
	CString strSql = "";
	CString strUpd = "";
	CDbCommand cmd_upd(this->cdbConn);

	if (this->listUpdateColName.size() == 0)
	{
		PrintLog("没有更新项");
		return 0;
	}

	this->AddUpdateColName("REC_REVISOR");
	this->AddUpdateColName("REC_REVISE_TIME");

	this->SetColVal("REC_REVISOR", (CString)s.userid, rowNum);
	this->SetColVal("REC_REVISE_TIME", CDateTime::Now().ToString("yyyyMMddHHmmss"), rowNum);

	strUpd = this->BuildUpdateString(rowNum);

	strSql = "UPDATE " + this->sTableName + " SET " + strUpd + " WHERE " + condition;
	PrintLog("strSql", strSql);
	try
	{
		cmd_upd.SetCommandText(strSql);
		updCount = cmd_upd.ExecuteNonQuery();
		cmd_upd.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		cmd_upd.Close();
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}],请联系开发人员", arguments, 1);
		CString str = strSql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);

		updCount = -1;
	}

	PrintLog("修改成功, updCount", updCount);
	return updCount;
}

//-----------------------------------------------------------------------
//      概述:
//               根据主键，从数据库更新满足条件的记录， 修改列取AddlistUpdateColName添加的列名。
//      返回值:
//               被更新的记录数， 如果更新出错则返回 -1。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::Update()
{
	PrintLog("Update");

	int updCount = 0;
	for (int i = 0; i < this->dtTable.Rows.get_Count(); i++)
	{
		CString condition = this->BuildWhereString(i);
		PrintLog("condition", condition);

		int updCountSub = this->Update(condition, i);
		PrintLog("updCountSub", updCountSub);

		if (updCountSub < 0)
		{
			return -1;
		}

		updCount += updCountSub;
	}

	PrintLog("updCount", updCount);
	return updCount;
}

//-----------------------------------------------------------------------
//      概述:
//               根据条件condition，从数据库删除满足条件的记录。
//      参数:
//               condition                                       - 过滤条件
//      返回值:
//               被删除的记录数， 如果删除出错则返回 -1。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::Delete(const CString& condition)
{
	CTracer log(__FUNCTION__);

	if (condition.Trim() == "")
	{
		strcpy(s.msg, "没有条件语句");
		return false;
	}

	int delCount = 0;
	CString strSql = "";
	CString strUpd = "";
	CDbCommand cmd_del(this->cdbConn);

	strSql = "DELETE FROM " + this->sTableName + " WHERE " + condition;
	PrintLog("strSql", strSql);
	try
	{
		cmd_del.SetCommandText(strSql);
		delCount = cmd_del.ExecuteNonQuery();
		cmd_del.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		cmd_del.Close();
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}],请联系开发人员", arguments, 1);
		CString str = strSql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);

		delCount = -1;
	}

	PrintLog("删除成功, delCount", delCount);
	return delCount;
}

//-----------------------------------------------------------------------
//      概述:
//               根据主键，从数据库删除满足条件的记录。
//      返回值:
//               被删除的记录数， 如果删除出错则返回 -1。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::Delete(int rowNum)
{
	CString condition = this->BuildWhereString(rowNum);
	return this->Delete(condition);
}

//-----------------------------------------------------------------------
//      概述:
//               根据主键，从数据库删除满足条件的记录。
//      返回值:
//               被删除的记录数， 如果删除出错则返回 -1。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::DeleteAll()
{
	int delCount = 0;
	for (int i = 0; i < this->dtTable.Rows.get_Count(); i++)
	{
		CString condition = this->BuildWhereString(i);
		int delCountSub = this->Delete(condition);

		if (delCountSub < 0)
		{
			return -1;
		}

		delCount += delCountSub;
	}

	return delCount;
}

//-----------------------------------------------------------------------
//      概述:
//               将CDataTable中的数据复制到实体对象中的CDataTable中。
//      参数:
//               table                                     - CDataTable 对象。
//      说明:
//               
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::CopyFrom(CDataTable& table)
{
	this->dtTable.Rows.Clear();
	for (int i = 0; i < table.Rows.get_Count(); i++)
	{
		this->dtTable.Rows.Add();
		this->dtTable.Rows[i].Merge(table.Rows[i]);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               将实体对象CDataTable中的数据复制到目的CDataTable中。
//      参数:
//               table                                     - CDataTable 对象。
//      说明:
//               
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::CopyTo(CDataTable& table)
{
	table.Clear();
	table.Clone(this->dtTable);
	for (int i = 0; i < this->dtTable.Rows.get_Count(); i++)
	{
		table.Rows.Add();
		table.Rows[i].Merge(this->dtTable.Rows[i]);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               将CDataRow中的数据合并到实体对象CDataTable的指定行中。
//      参数:
//               row                                       - CDataRow 对象。
//				 rowNum									   - 实体对象CDataTable行号(默认0)。	
//      说明:
//               
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::MergeFrom(CDataRow& row, int rowNum)
{
	while (this->dtTable.Rows.get_Count() - 1 < rowNum)
	{
		this->dtTable.Rows.Add();
	}

	CDataTable dtSrc = row.get_Table();
	for (int i = 0; i < this->dtTable.Columns.get_Count(); i++)
	{
		CString colName = this->dtTable.Columns[i].get_ColumnName().ToUpper();
		int colHoldFlag = 0;

		if (!dtSrc.Columns.Contains(colName))
		{
			continue;
		}

		//跳过保留字段
		for (list<CString>::const_iterator iter = this->listHoldColName.begin(); iter != this->listHoldColName.end(); iter++)
		{
			if ((*iter).ToUpper() == colName)
			{
				colHoldFlag = 1;
				break;
			}
		}

		if (colHoldFlag == 1)
		{
			continue;
		}

		if (this->dtTable.Columns[i].get_DataType() == DT_STRING)
		{
			if (row[colName] == CDBNull::Value || row[colName].ToString().Trim() == "")
			{
				this->dtTable.Rows[rowNum][colName] = " ";
			}
			else
			{
				this->dtTable.Rows[rowNum][colName] = (CString)row[colName];
			}
		}
		else
		{
			if (row[colName] == CDBNull::Value)
			{
				this->dtTable.Rows[rowNum][colName] = (CDecimal)0;
			}
			else
			{
				this->dtTable.Rows[rowNum][colName] = (CDecimal)row[colName];
			}
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               将实体对象CDataTable中的数据添加到目的CDataTable中。
//      参数:
//               table                                             - CDataTable 对象。
//				 rowNum											   - 实体对象CDataTable行号(默认0)。	
//               addCol                                            - 如果为true， 则添加CDataTable中不存在的列名。
//               overrideFlag                                      - 如果为true， 则覆盖CDataTable中已有的数据。
//      说明:
//               
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::MergeTo(CDataTable& table, int rowNum, bool addCol, bool overrideFlag)
{
	if (this->dtTable.Rows.get_Count() > rowNum)
	{
		CDataRow& row = table.Rows.Add();
		MergDataRow(this->dtTable.Rows[rowNum], row, addCol, overrideFlag);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               将实体对象CDataTable的行转为列。
//      参数:
//               colEName                                          - 行转列的列名存放字段名。
//				 colCName										   - 行转列的列名中文存放字段名。	
//               keyColName1                                       - 行转列关键字段1。
//               keyColName2                                       - 行转列关键字段2， 默认没有。
//               keyColName4                                       - 行转列关键字段2， 默认没有。
//      返回值:
//               转换完成的CDataTable。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CDataTable CDynaTable::ConvertRowToColumn(CString colEName, CString colCName, CString colType, CString colValue,
	CString keyColName, CString seqColName)
{
	CDataTable dtConvert;
	int iRowNum = 0;

	if (this->dtTable.Columns.Contains(colEName) &&
		this->dtTable.Columns.Contains(colCName) &&
		this->dtTable.Columns.Contains(colValue) &&
		this->dtTable.Columns.Contains(keyColName))
	{
		PrintLog("ConvertRowToColumn");

		list<CString> listKeyColValue;

		for (int i = 0; i < this->dtTable.Columns.get_Count(); i++)
		{
			if (this->dtTable.Columns[i].get_ColumnName() != colEName &&
				this->dtTable.Columns[i].get_ColumnName() != colCName &&
				this->dtTable.Columns[i].get_ColumnName() != colType &&
				this->dtTable.Columns[i].get_ColumnName() != colValue &&
				this->dtTable.Columns[i].get_ColumnName() != seqColName)
			{
				dtConvert.Columns.Add(this->dtTable.Columns[i].get_DataType(), this->dtTable.Columns[i].get_ColumnName());
				dtConvert.Columns[this->dtTable.Columns[i].get_ColumnName()].set_Caption(this->dtTable.Columns[i].get_Caption());
			}
		}

		for (int i = 0; i < this->dtTable.Rows.get_Count(); i++)
		{
			int columnExitsFlag = 0;
			for (list<CString>::const_iterator iter = listKeyColValue.begin(); iter != listKeyColValue.end(); iter++)
			{
				if (*iter == this->dtTable.Rows[i][keyColName].ToString())
				{
					columnExitsFlag = 1;
					break;
				}
			}

			if (columnExitsFlag == 0)
			{
				listKeyColValue.push_back(this->dtTable.Rows[i][keyColName].ToString());
				dtConvert.Rows.Add();
				dtConvert.Rows[iRowNum++].Merge(this->dtTable.Rows[i]);
			}
		}

		//PrintDataTable(dtConvert);

		for (int i = 0; i < dtConvert.Rows.get_Count(); i++)
		{
			//PrintLog("i", i);
			//PrintLog("KEY", dtConvert.Rows[i][keyColName].ToString());

			list<int> listSeqNo;
			for (int j = 0; j < this->dtTable.Rows.get_Count(); j++)
			{
				if (this->dtTable.Rows[j][keyColName].ToString() == dtConvert.Rows[i][keyColName].ToString())
				{
					//PrintLog("dtTable Seq", this->dtTable.Rows[j][seqColName].ToDecimal());
					listSeqNo.push_back(this->dtTable.Rows[j][seqColName].ToDecimal().ToInt32());
				}
			}

			listSeqNo.sort();

			for (list<int>::const_iterator iter2 = listSeqNo.begin(); iter2 != listSeqNo.end(); iter2++)
			{
				//PrintLog("SEQ", *iter2);

				for (int j = 0; j < this->dtTable.Rows.get_Count(); j++)
				{
					if (this->dtTable.Rows[j][keyColName].ToString() == dtConvert.Rows[i][keyColName].ToString() &&
						this->dtTable.Rows[j][seqColName].ToDecimal().ToInt32() == *iter2)
					{
						CString colName = this->dtTable.Rows[j][colEName].ToString();
						CString colCaption = this->dtTable.Rows[j][colCName].ToString();
						PrintLog("colName", colName);
						PrintLog("colCaption", colCaption);

						if (!dtConvert.Columns.Contains(colName))
						{
							if (this->dtTable.Columns.Contains(colType))
							{
								if (this->dtTable.Rows[j][colType].ToString() == "C")
								{
									dtConvert.Columns.Add(DT_STRING, colName);
								}
								else
								{
									dtConvert.Columns.Add(DT_DECIMAL, colName);
								}
							}
							else if (colType == "D")
							{
								dtConvert.Columns.Add(DT_DECIMAL, colName);
							}
							else
							{
								dtConvert.Columns.Add(DT_STRING, colName);
							}

							dtConvert.Columns[this->dtTable.Rows[j][colEName].ToString()].set_Caption(colCaption);
						}

						//PrintLog("i", i);
						//PrintLog("ITEM_ENAME", this->dtTable.Rows[j][colEName].ToString());
						//PrintLog("ITEM_CNAME", this->dtTable.Rows[j][colCName].ToString());
						//PrintLog("ITEM_VALUE", this->dtTable.Rows[j][colValue].ToDecimal());

						dtConvert.Rows[i][colName] = this->dtTable.Rows[j][colValue];
					}
				}
			}
		}
	}

	PrintDataTable(dtConvert);
	return dtConvert;
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CString类型)。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetColVal(const CString& colName, CString colVal, int rowNum)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		while (this->dtTable.Rows.get_Count() - 1 < rowNum)
		{
			this->dtTable.Rows.Add();
		}

		this->dtTable.Rows[rowNum][colName] = colVal;
	}
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CDecimal类型)。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetColVal(const CString& colName, CDecimal colVal, int rowNum)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		while (this->dtTable.Rows.get_Count() - 1 < rowNum)
		{
			this->dtTable.Rows.Add();
		}

		this->dtTable.Rows[rowNum][colName] = colVal;
	}
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable所有行某列数据。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CString类型)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetColValAllRow(const CString& colName, CString colVal)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		for (int i = 0; i < this->dtTable.Rows.get_Count(); i++)
		{
			this->dtTable.Rows[i][colName] = colVal;
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable所有行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CDecimal类型)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetColValAllRow(const CString& colName, CDecimal colVal)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		for (int i = 0; i < this->dtTable.Rows.get_Count(); i++)
		{
			this->dtTable.Rows[i][colName] = colVal;
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               复制实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               rowNum											   - CDataTable 行号。
//               rowNumSrc										   - CDataTable 复制行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::CopyColVal(const CString& colName, int rowNum, int rowNumSrc)
{
	if (!this->dtTable.Columns.Contains(colName))
	{
		return;
	}

	if (this->dtTable.Rows.get_Count() - 1 < rowNum)
	{
		return;
	}

	if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
	{
		if (this->dtTable.Rows[rowNumSrc][colName] == CDBNull::Value || (CString)this->dtTable.Rows[rowNumSrc][colName] == "")
		{
			this->dtTable.Rows[rowNum][colName] = " ";
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CString)this->dtTable.Rows[rowNumSrc][colName];
		}
	}
	else
	{
		if (this->dtTable.Rows[rowNum][colName] == CDBNull::Value)
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)0;
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)this->dtTable.Rows[rowNumSrc][colName];
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               复制实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               colNameSrc                                        - CDataTable 复制列列名。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::CopyColVal(const CString& colName, const CString& colNameSrc, int rowNum)
{
	if (!this->dtTable.Columns.Contains(colName) || !this->dtTable.Columns.Contains(colNameSrc))
	{
		return;
	}

	if (this->dtTable.Rows.get_Count() - 1 < rowNum)
	{
		return;
	}

	if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
	{
		if (this->dtTable.Rows[rowNum][colNameSrc] == CDBNull::Value || (CString)this->dtTable.Rows[rowNum][colNameSrc] == "")
		{
			this->dtTable.Rows[rowNum][colName] = " ";
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CString)this->dtTable.Rows[rowNum][colNameSrc];
		}
	}
	else
	{
		if (this->dtTable.Rows[rowNum][colNameSrc] == CDBNull::Value)
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)0;
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)this->dtTable.Rows[rowNum][colNameSrc];
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               复制CDataRow某列数据到实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               colNameSrc                                        - CDataRow 列名。
//               row											   - CDataRow 对象。
//               rowNum											   - CDataTable 行号(默认0)。
//               addCol                                            - 如果为true， 则添加CDataTable中不存在的列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::CopyColVal(const CString& colName, CDataRow& row, CString colNameSrc, int rowNum, bool addCol)
{
	CDataTable dtSrc = row.get_Table();
	if (!dtSrc.Columns.Contains(colNameSrc))
	{
		return;
	}

	if (!this->dtTable.Columns.Contains(colName))
	{
		if (addCol == true)
		{
			this->dtTable.Columns.Add(dtSrc.Columns[colNameSrc].get_DataType(), colName);
		}
		else
		{
			return;
		}
	}

	while (this->dtTable.Rows.get_Count() - 1 < rowNum)
	{
		this->dtTable.Rows.Add();
	}

	if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
	{
		if (row[colNameSrc] == CDBNull::Value || (CString)row[colNameSrc] == "")
		{
			this->dtTable.Rows[rowNum][colName] = " ";
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CString)row[colNameSrc];
		}
	}
	else
	{
		if (row[colNameSrc] == CDBNull::Value)
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)0;
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)row[colNameSrc];
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               复制CDataRow某列数据到实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               row											   - CDataRow 对象。
//               rowNum											   - CDataTable 行号(默认0)。
//               addCol                                            - 如果为true， 则添加CDataTable中不存在的列名。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::CopyColVal(const CString& colName, CDataRow& row, int rowNum, bool addCol)
{
	CDataTable dtSrc = row.get_Table();
	if (!dtSrc.Columns.Contains(colName))
	{
		return;
	}

	if (!this->dtTable.Columns.Contains(colName))
	{
		if (addCol == true)
		{
			this->dtTable.Columns.Add(dtSrc.Columns[colName].get_DataType(), colName);
		}
		else
		{
			return;
		}
	}

	while (this->dtTable.Rows.get_Count() - 1 < rowNum)
	{
		this->dtTable.Rows.Add();
	}

	if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
	{
		if (row[colName] == CDBNull::Value || (CString)row[colName] == "")
		{
			this->dtTable.Rows[rowNum][colName] = " ";
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CString)row[colName];
		}
	}
	else
	{
		if (row[colName] == CDBNull::Value)
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)0;
		}
		else
		{
			this->dtTable.Rows[rowNum][colName] = (CDecimal)row[colName];
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               实体对象CDataTable添加列名。
//      参数:
//               colName                                           - CDataTable 列名。
//               colType										   - CDataTable 列名类型(默认DT_STRING)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddColName(const CString& colName, DsType colType)
{
	if (!this->dtTable.Columns.Contains(colName))
	{
		if (colType == DT_DECIMAL)
		{
			this->dtTable.Columns.Add(DT_DECIMAL, colName);
		}
		else
		{
			this->dtTable.Columns.Add(DT_STRING, colName);
		}
	}
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable某行某列数据， 默认第一行, 如果没有该列名则添加列名。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CString类型)。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddColVal(const CString& colName, CString colVal, int rowNum)
{
	this->AddColName(colName);

	while (this->dtTable.Rows.get_Count() - 1 < rowNum)
	{
		this->dtTable.Rows.Add();
	}

	this->dtTable.Rows[rowNum][colName] = colVal;
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable某行某列数据， 默认第一行, 如果没有该列名则添加列名。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CDecimal类型)。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::AddColVal(const CString& colName, CDecimal colVal, int rowNum)
{
	if (!this->dtTable.Columns.Contains(colName))
	{
		this->dtTable.Columns.Add(DT_DECIMAL, colName);
	}

	while (this->dtTable.Rows.get_Count() - 1 < rowNum)
	{
		this->dtTable.Rows.Add();
	}

	this->dtTable.Rows[rowNum][colName] = colVal;
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable某行某列数据， 默认第一行, 将概字段设为过滤条件。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CString类型)。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetFilterColVal(const CString& colName, CString colVal, int rowNum)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		while (this->dtTable.Rows.get_Count() - 1 < rowNum)
		{
			this->dtTable.Rows.Add();
		}

		this->dtTable.Rows[rowNum][colName] = colVal;
		this->AddFilterColName(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable某行某列数据， 默认第一行, 将概字段设为过滤条件。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CDecimal类型)。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetFilterColVal(const CString& colName, CDecimal colVal, int rowNum)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		while (this->dtTable.Rows.get_Count() - 1 < rowNum)
		{
			this->dtTable.Rows.Add();
		}

		this->dtTable.Rows[rowNum][colName] = colVal;
		this->AddFilterColName(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable某行某列数据， 默认第一行, 将概字段设为保留列。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CString类型)。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetHoldColVal(const CString& colName, CString colVal, int rowNum)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		while (this->dtTable.Rows.get_Count() - 1 < rowNum)
		{
			this->dtTable.Rows.Add();
		}

		this->dtTable.Rows[rowNum][colName] = colVal;
		this->AddHoldColName(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               写实体对象CDataTable某行某列数据， 默认第一行, 将概字段设为保留列。
//      参数:
//               colName                                           - CDataTable 列名。
//               colVal                                            - CDataTable 列值(CString类型)。
//               rowNum											   - CDataTable 行号(默认0)。
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::SetHoldColVal(const CString& colName, CDecimal colVal, int rowNum)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		while (this->dtTable.Rows.get_Count() - 1 < rowNum)
		{
			this->dtTable.Rows.Add();
		}

		this->dtTable.Rows[rowNum][colName] = colVal;
		this->AddHoldColName(colName);
	}
}

//-----------------------------------------------------------------------
//      概述:
//               取实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               rowNum											   - CDataTable 行号(默认0)。
//      返回值:
//               CString类型数据。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::GetColValString(const CString& colName, int rowNum)
{
	if (this->dtTable.Columns.Contains(colName) && this->dtTable.Rows.get_Count() - 1 >= rowNum)
	{
		return this->dtTable.Rows[rowNum][colName];
	}
	else
	{
		return "";
	}
}

//-----------------------------------------------------------------------
//      概述:
//               取实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName                                           - CDataTable 列名。
//               rowNum											   - CDataTable 行号(默认0)。
//      返回值:
//               CDecimal类型数据。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CDecimal CDynaTable::GetColValDecimal(const CString& colName, int rowNum)
{
	if (this->dtTable.Columns.Contains(colName) && this->dtTable.Rows.get_Count() - 1 >= rowNum)
	{
		return this->dtTable.Rows[rowNum][colName];
	}
	else
	{
		return 0;
	}
}

//-----------------------------------------------------------------------
//      概述:
//               取实体对象CDataTable。
//      返回值:
//               实体对象CDataTable。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CDataTable& CDynaTable::GetDataTable()
{
	CDataTable& retTable = this->dtTable;
	return retTable;
}

//-----------------------------------------------------------------------
//      概述:
//               取实体对象CDataTable的某行数据。
//      参数:
//               rowNum											   - CDataTable 行号(默认0)。
//      返回值:
//               CDataRow。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CDataRow& CDynaTable::GetDataRow(int rowNum)
{
	CDataRow& retRow = this->dtTable.Rows[rowNum];
	return retRow;
}

//-----------------------------------------------------------------------
//      概述:
//				取修改字段,用,拼接
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::GetUpdateColString()
{
	CString strUpdCol = "";
	for (list<CString>::const_iterator iter = this->listUpdateColName.begin(); iter != this->listUpdateColName.end(); iter++)
	{
		if (iter != this->listUpdateColName.begin())
		{
			strUpdCol += ",";
		}

		strUpdCol += *iter;
	}

	return strUpdCol;
}

//-----------------------------------------------------------------------
//      概述:
//               判断实体对象CDataTable是否存在某列。
//      参数:
//               colName                                           - CDataTable 列名。
//      返回值:
//               存在返回ture,不存在返回false。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT bool CDynaTable::Contains(CString colName)
{
	return this->dtTable.Columns.Contains(colName);
}

//-----------------------------------------------------------------------
//      概述:
//               返回对应的表名。
//      说明:
//               
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT CString CDynaTable::GetTableName()
{
	return this->sTableName;
}

//-----------------------------------------------------------------------
//      概述:
//               取实体对象CDataTable行数。
//      返回值:
//               实体对象CDataTable行数。     
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT int CDynaTable::GetRowCount()
{
	return this->dtTable.Rows.get_Count();
}

//-----------------------------------------------------------------------
//      概述:
//               打印实体对象CDataTable数据。    
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::Print()
{
	PrintDataTable(this->dtTable);
}

//-----------------------------------------------------------------------
//      概述:
//               打印实体对象CDataTable某行数据。
//      参数:
//               rowNum											   - CDataTable 行号。    
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::Print(int rowNo)
{
	PrintDataTable(this->dtTable, rowNo);
}

//-----------------------------------------------------------------------
//      概述:
//               打印实体对象CDataTable某行某列数据， 默认第一行。
//      参数:
//               colName										   - CDataTable 列名。
//               rowNum											   - CDataTable 行号(默认0)。    
//-----------------------------------------------------------------------
BM2_FUNCTION_EXPORT void CDynaTable::Print(CString colName, int rowNo)
{
	if (this->dtTable.Columns.Contains(colName))
	{
		if (this->dtTable.Columns[colName].get_DataType() == DT_STRING)
		{
			PrintLog(colName, this->dtTable.Rows[rowNo][colName].ToString());
		}
		else
		{
			PrintLog(colName, this->dtTable.Rows[rowNo][colName].ToDecimal());
		}
	}
}