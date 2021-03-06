#pragma once
#include "CRecord.h"

CRecord::CRecord()
{
    this->m_Nstatue = CRecord::BLANK;
    this->m_sSerialNum = "";
}

CRecord::CRecord(QString serNum)
{
    this->m_sSerialNum = serNum;
}

CRecord::~CRecord()
{

}

const unsigned long CRecord::getOffsetAt( int index) const
{
    return 0xFFFFFFFFFFFFFFFF;
}
const QString& CRecord::getRemarkAt( int index) const
{
    return this->m_sSerialNum;
}

const QString& CRecord::getSerialNumber( void ) const
{
    return this->m_sSerialNum;
}
const QString& CRecord::getInfo( void ) const
{
    return this->m_sSerialNum;
}

const int CRecord::getStatue( void ) const
{
    return this->m_Nstatue;
}


int CRecord::size( void ) const
{
    return 1;
}

unsigned long CRecord::getLastOffset() const
{
	return 0;
}
