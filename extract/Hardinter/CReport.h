#pragma once
#include "CRecord.h"
#include "../resource.h"
#include <QFile>

class CReport
{
public:
    enum
    {
        TYPE_SPE_VALUE,
        FILETYPE_PIC,
        FILETYPE_DOC
    };

private:
    QFile reportFile;
    QFile serialNum;
    QFile SourceFile;
    class CRecord * record;
    unsigned char *pDataBuffer;

    static const unsigned int BUFFERSIZE = 1000;
    static const unsigned long ENDSTATUE = -1;
public:
    CReport();
    ~CReport();
    CReport(const CReport &report);
    /* 新建一个记录文档，保存文件相关信息(待添加)
     * 输入参数    filepath--被分析文件路径； fType--由enum FILETYPE_定义
     * 返回值      成功返回FILE_SUCCEED, 失败返回MEM_RUN_OUT, HD_RUN_OUT
     */
    int NewReportFile(const QString& filepath, int fType);
    /* 写入记录
     * 输入参数    offset--可疑数据起点的地址偏移量； remarks--备注； length--可疑数据块长度
     * 需要从源文件复制数据块至记录文件
     * 返回值      成功返回FUNC_SUCCEED, 失败返回MEM_RUN_OUT, HD_RUN_OUT
     */
    int AddToReport(unsigned long offset, const QString& remarks, int length);
    /* 写入备注----此函数是重载函数
     * 输入参数     remarks--备注
     * 返回值      同上
     */
    int AddToReport(const QString& remarks);
    /*获取当前实例生成的报告
     */
    const class CRecord &GetRecord();
    void  Close();//方便显式调用,同时加文件结束符,并将Record的状态修正为已完成


    /* 打开记录文件，返回记录
     * 失败(文件不存在)返回空记录
     */
    const CRecord &OpenReportFile(const QString& serialNumber);
    const class CRecord &Recover( void );//返回所恢复的偏移量
    /* 在文件或其他数据中查找字符串
     * 输入参数 type--用于确定要搜索的内容类型，capital 与 len--被搜索的内容
     * 在特定数据中搜索相应的内容
     * 返回偏移量，或SEARCH_FAILED
     */
    static unsigned long SearchInReport(int type, const unsigned char* capital, int len);
    static void DelRecord(class CRecord *rec);
private:
    static unsigned long long SearchInReport(int type, const QString extraStr,
                                             const unsigned char *capital, int len);
    //此函数保留

};
