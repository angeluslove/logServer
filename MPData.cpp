#include "mpdata.h"

MPData::MPData(QObject *parent) : QObject{parent}
{
    this->init();
}

//初始化
void MPData::init()
{
    m_headName.clear();
    m_paraName.clear();
    m_byte.clear();
    m_unit = {};
    m_head.clear();
    m_data.clear();
}

void MPData::setHeaderName(const QString &theName)
{
    m_headName = theName;
}

void MPData::setHeaderValue(const QString &theValue)
{
    m_unit.insert(m_headName,theValue);
}

void MPData::setParameterName(const QString &theName)
{
    m_paraName = theName;
}

void MPData::setParameterValue(const QString &theValue)
{
    m_unit.insert(m_paraName,theValue);
}

void MPData::setData(const QByteArray &theValue)
{
    m_byte = theValue;
}

void MPData::start()
{
    m_headName.clear();
    m_paraName.clear();
    m_byte.clear();
    m_unit = {};
}

void MPData::end()
{
    m_head.append(m_unit);
    m_data.append(m_byte);
}
