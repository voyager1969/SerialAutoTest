#pragma once
#include <QString>
#include <QList>
#include <QFile>
#include <qtextstream.h>
#include <QTextCodec>
namespace LX
{
	class Sheet
	{
		//  enum FieldType { STRING, INT, DOUBLE, BOOL };
	public:
		Sheet() {}
		Sheet(Sheet&& rhs);
		Sheet& operator =(Sheet&& rhs);
	public:
		// QString name;
		QList<QString> head;
		QList<QList<QString>> data;
		// QList<FieldType> fieldTypes;

		QList<QString> splitCSVLine(const QString& lineStr, QString sep)
		{
			QList<QString> strList;
			strList = lineStr.split(sep);
			strList[strList.size()-1]= strList.last().remove("\n");
			return qMove(strList);
		}

		int ReadCsv(QString filepath)
		{
			QTextCodec* codec = QTextCodec::codecForName("GBK");
			QFile file(filepath);
			if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				return 0;
			}

			QTextStream inStream(&file);

			bool flagHead = 1;  // Is FirstLine
			while (!file.atEnd())
			{
				QString lineStr = codec->toUnicode(file.readLine());
				if (lineStr.isEmpty())
				{
					continue;
				}
				if (flagHead)
				{
					head = splitCSVLine(lineStr, ",");
					flagHead = 0;
				}
				else
				{
					data.append(splitCSVLine(lineStr, ","));
				}
			}
			file.close();
			return 1;
		}
		QList<QString> ShowItemList(QString itemName)
		{
			QList<QString> qstrItemList;
			int iNameIndex = -1;
			// 查找item编号
			for (int i = 0; i < head.size(); ++i)
			{
				if (itemName == head.at(i))
				{
					iNameIndex = i;
				}
			}
			if (iNameIndex != -1)
			{
				if (data.size())
				{
					// 列出该编号的所有的数据
					for (int i = 0; i < data.size(); ++i)
					{
						if (data[i].size() == head.size())
						{
							qstrItemList.append(data[i][iNameIndex]);
						}

					}
				}

			}
			return qstrItemList;
		}
	};
}
