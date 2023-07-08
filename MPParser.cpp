#include "MPParser.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

MPParser::MPParser(QObject *parent) : QObject(parent)
{
    p_data = nullptr;
}

MPParser::~MPParser()
{
}

//解析bound字符串
QByteArray MPParser::getBoundary(QByteArray head)
{
    qint64     aSize     = 0;
    char*      aContent  = head.data();
    char*      aBoundary = this->MPP_GetBoundary(aContent,&aSize);
    return QByteArray(aBoundary,aSize);
}

//解析
void MPParser::proces(QByteArray boundary, QByteArray body)
{
    qint64  aSize     = boundary.length();
    char*   aBoundary = boundary.data();
    qint64  dSize     = body.length();
    char*   aData     = body.data();
    //初始化
    this->MPP_MultiPartInit(aBoundary,aSize);
    //解析
    this->MPP_MultiPartProcess(aData,dSize);
}
//返回头结构
QList<QJsonObject> MPParser::getHead()
{
    return p_data->getHead();
}
//返回数据结构
QList<QByteArray> MPParser::getData()
{
    return p_data->getData();
}

char* MPParser::MPP_GetBoundary(char* theContentType, qint64* theSize)
{
    char* aCurrPtr  = NULL;
    char* aBoundary = strstr(theContentType, "boundary");
    if( aBoundary == NULL )
    {
        return NULL;
    }
    aBoundary += 8;
    while( *aBoundary != 0 )
    {
        if( (*aBoundary != '=') &&  (!isspace(*aBoundary)) )
        {
            aCurrPtr = aBoundary;
            *theSize = 0;
            while( *aCurrPtr != 0 )
            {
                if( isspace(*aCurrPtr) || (*aCurrPtr == ';'))
                {
                    return aBoundary;
                }
                (*theSize)++;
                aCurrPtr++;
            }
            return aBoundary;
        }
        aBoundary++;
    }
    return NULL;
}

int MPParser::mpp_strncmp(char* theBuffer, qint64 theSize, char* theStringToFind)
{
    int aFindStrLen = strlen(theStringToFind);
    if( theSize < aFindStrLen ) return -1;
    return memcmp(theBuffer,theStringToFind,aFindStrLen);
}

int MPParser::mpp_memncmp(char* theBuffer, qint64 theSize, char* theBuffToFind, qint64 theFindSize)
{
    if( theSize < theFindSize ) return -1;
    return memcmp(theBuffer,theBuffToFind,theFindSize);
}

void MPParser::mpp_trim(char* theBuffer,qint64* theStart, qint64* theSize)
{
    char* aPtr;
    while( (*theSize) > 0)
    {
        if( isspace(theBuffer[*theStart]) )
        {
            (*theStart)++;
            (*theSize)--;
        }
        else
        {
            break;
        }
    }

    while( (*theSize) > 0)
    {
        aPtr = theBuffer + (*theSize) - 1;
        if( isspace(*aPtr) )
        {
            (*theSize)--;
        }
        else
        {
            break;
        }
    }
}

void MPParser::mpp_remove_quotas(char* theBuffer,qint64* theStart, qint64* theSize)
{
    if( theBuffer[*theStart] == '"' && theBuffer[*theStart + *theSize -1])
    {
        (*theStart) += 1;
        (*theSize)  -= 2;
    }
}

void MPParser::mpp_shift(char* theBuffer)
{
    if( theBuffer[m_Indx] == '\r' || theBuffer[m_Indx] == '\n')
    {
        m_Indx++;
        m_Remain--;
        if( theBuffer[m_Indx] == '\n')
        {
            m_Indx++;
            m_Remain--;
        }
        return;
    }
    m_Indx++;
    m_Remain--;
}

//初始化过程
void MPParser::MPP_MultiPartInit(char* theBoundStr, qint64 theBoundStrLen)
{
    //初始化指针
    if(p_data != nullptr)
    {
        p_data->init();
    }
    else
    {
        p_data = new MPData(this);
    }
    m_BoundStr = (char*)malloc(theBoundStrLen + 1);
    strncpy(m_BoundStr, theBoundStr, theBoundStrLen);
    m_BoundStr[theBoundStrLen] = 0;
    m_BoundStrLen  = theBoundStrLen;
    m_State        = MPS_FIND_FIRST_DELIMETER;
}

//解析过程
void MPParser::MPP_MultiPartProcess(char* theBuffer, qint64 theBufferSize)
{
    m_Indx   = 0;
    m_Remain = theBufferSize;
    qint64   aSectionStart = 0;
    qint64   aSize;

    while(m_Indx < theBufferSize)
    {
        switch(m_State)
        {
            case MPS_FIND_FIRST_DELIMETER:
            {
                if(mpp_memncmp(theBuffer + m_Indx,m_Remain,m_BoundStr,m_BoundStrLen) == 0)
                {
                    m_Indx   += m_BoundStrLen;
                    m_Remain -= m_BoundStrLen;
                    mpp_shift(theBuffer);
                    aSectionStart = m_Indx;
                    m_State = MPS_HEADER_NAME;
                    //------------------------
                    p_data->start();
                    //------------------------
                    continue;
                }
            }
            break;
            case MPS_HEADER_NAME:
            {
                if( theBuffer[m_Indx] == '\r' || theBuffer[m_Indx] == '\n')
                {
                    mpp_shift(theBuffer);
                    m_State = MPS_DATA_PART;
                    aSectionStart = m_Indx;
                    continue;
                }
                if( theBuffer[m_Indx] == ':')
                {

                        aSize = m_Indx-aSectionStart;
                        mpp_trim(theBuffer,&aSectionStart,&aSize);
                        //------------------------
                        QByteArray byte = QByteArray(theBuffer + aSectionStart,aSize);
                        p_data->setHeaderName(byte);
                        //------------------------
                        m_State = MPS_HEADER_VALUE;
                        aSectionStart = m_Indx+1;

                }
            }
            break;
            case MPS_HEADER_VALUE:
            {
                if( theBuffer[m_Indx] == ';' || theBuffer[m_Indx] == '\r' || theBuffer[m_Indx] == '\n')
                {
                    aSize = m_Indx-aSectionStart;
                    mpp_trim(theBuffer,&aSectionStart,&aSize);
                    //------------------------
                    QByteArray byte = QByteArray(theBuffer + aSectionStart,aSize);
                    p_data->setHeaderValue(byte);
                    //------------------------

                    if( theBuffer[m_Indx] == '\r' || theBuffer[m_Indx] == '\n')
                    {
                        m_State = MPS_HEADER_NAME;
                    }
                    else
                    {
                        m_State = MPS_HEADER_PARAMETER_NAME;
                        aSectionStart = m_Indx + 1;
                    }
                }
            }
            break;
            case MPS_HEADER_PARAMETER_NAME:
            {
                /*Parse parameter name*/
                if( theBuffer[m_Indx] == '\r' || theBuffer[m_Indx] == '\n')
                {
                    m_Indx++;
                    m_Remain--;
                    if( theBuffer[m_Indx] == '\n')
                    {
                        m_Indx++;
                        m_Remain--;
                    }
                    aSectionStart = m_Indx + 1;
                    m_State = MPS_HEADER_NAME;
                    continue;
                }
                if( theBuffer[m_Indx] == '=' )
                {
                    m_State = MPS_HEADER_PARAMETER_VALUE;

                    aSize = m_Indx-aSectionStart;
                    mpp_trim(theBuffer,&aSectionStart,&aSize);
                    //------------------------
                    QByteArray byte = QByteArray(theBuffer + aSectionStart,aSize);
                    p_data->setParameterName(byte);
                    //------------------------
                    aSectionStart = m_Indx + 1;
                }
            }
            break;
            case MPS_HEADER_PARAMETER_VALUE:
            {
                /*Parse parameter value*/
                if( theBuffer[m_Indx] == ';' || theBuffer[m_Indx]=='\r' || theBuffer[m_Indx]=='\r' )
                {
                    aSize = m_Indx-aSectionStart;
                    mpp_trim(theBuffer,&aSectionStart,&aSize);
                    mpp_remove_quotas(theBuffer,&aSectionStart,&aSize);
                    //------------------------
                    QByteArray byte = QByteArray(theBuffer + aSectionStart,aSize);
                    p_data->setParameterValue(byte);
                    //------------------------
                    if(theBuffer[m_Indx] == ';')
                    {
                        m_State = MPS_HEADER_PARAMETER_NAME;
                        aSectionStart = m_Indx + 1;
                    }
                    else
                    {
                        mpp_shift(theBuffer);
                        aSectionStart = m_Indx;
                        m_State = MPS_HEADER_NAME;
                        continue;
                    }
                }
            }
            break;
            case MPS_DATA_PART:
            {
                /* Process data */
                if(mpp_memncmp(theBuffer+m_Indx,m_Remain,m_BoundStr,m_BoundStrLen) == 0)
                {
                    //------------------------
                    QByteArray byte = QByteArray(theBuffer + aSectionStart,m_Indx-aSectionStart-4);
                    p_data->setData(byte);
                    //------------------------
                    m_State  =  MPS_SKIP_CRLF;
                    m_Indx   += m_BoundStrLen;
                    m_Remain -= m_BoundStrLen;
                    continue;
                }
            }
            break;
            case MPS_SKIP_CRLF:
            {
                /*Skip the data after delimeter string '--\r\n'*/
                if( theBuffer[m_Indx]=='\r' || theBuffer[m_Indx]=='\n' )
                {
                    m_State = MPS_HEADER_NAME;
                    mpp_shift(theBuffer);
                    aSectionStart = m_Indx;
                    //------------------------
                    p_data->end();
                    //------------------------
                    continue;
                }
            }
            break;
        }
        mpp_shift(theBuffer);
    }
    /*We reach the end of buffer but delimeter have not been found yet */
    if( m_State == MPS_DATA_PART )
    {
        //------------------------
        QByteArray byte = QByteArray(theBuffer + aSectionStart,m_Indx-aSectionStart);
        p_data->setData(byte);
        //------------------------
    }
}
