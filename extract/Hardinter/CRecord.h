#pragma once
#include "../resource.h"
#include "CReport.h"
#include <QString>
#include <QList>


class CRecord
{
    friend class CReport;
public:
    enum
    {
        BLANK,
        ONGOING,
        DONE
    };
private:
    QString m_sSerialNum;
    QStringList m_lsInfo;

    int m_Nstatue;

    QList<unsigned long long> m_lnOffset;
    QStringList m_lsRemark;
    int iterator;
    CRecord();
    CRecord(QString serNum);
    ~CRecord();
public: 

    /* 函数getOffsetAt 与 getRemarkAt : 用于文件内地址偏移量与备注的随机读取
     * 输入参数     索引下标
     * 功能        随机读取Offset和Remark
     * 返回值      成功返回对应值，失败返回LIST_OVER_BOUND或STRLIST_OVER_BOUND
     */
    const unsigned long getOffsetAt(int index) const;
    const QString& getRemarkAt(int index) const;


    const QString& getSerialNumber( void ) const;//获取分析报告流水号
    const QString& getInfo( void ) const;//以单一字符串形式返回被分析文件基本信息
    //将实现针对文件某特殊信息输出的接口

    const int getStatue( void ) const;

    int size( void ) const;//获取记录的条数
    unsigned long getLastOffset() const;

};
