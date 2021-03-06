#pragma once
#include "CReport.h"

CReport::CReport()
{
    record = new CRecord;
}

CReport::~CReport()
{

}

CReport::CReport(const CReport &report)
{
    (void) report;
}

int CReport::NewReportFile(const QString& filepath, int fType)
{
    return FUNC_SUCCEED;
}

int CReport::AddToReport(unsigned long offset, const QString& remarks, int length)
{
    return FUNC_SUCCEED;
}

int CReport::AddToReport(const QString& remarks)
{
    return FUNC_SUCCEED;
}

const class CRecord &CReport::GetRecord()
{
    return *record;
}

void CReport::DelRecord( class CRecord *rec)
{
    delete rec;
}

void  CReport::Close()
{
    return;
}

const class CRecord& CReport::OpenReportFile(const QString& serialNumber)
{
    return *record;
}

const CRecord &CReport::Recover( void )
{
    return *record;
}
unsigned long CReport::SearchInReport(int type, const unsigned char* capital, int len)
{
    return FUNC_SUCCEED;
}
