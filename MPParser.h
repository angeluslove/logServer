#ifndef MPPARSER_H
#define MPPARSER_H

#include <QObject>
#include "MPData.h"

/*
 * 对应 http MultiPart 解析类
 * 该类核心解析逻辑 来自 Maygli 的C语言项目
 * https://github.com/maygli/MultiPartParser
*/

class MPParser: public QObject
{
    Q_OBJECT

public:

    explicit MPParser(QObject *parent = nullptr);
    ~MPParser();

    //获取边界值
    QByteArray getBoundary(QByteArray head);
    //解析
    void       proces(QByteArray boundary,QByteArray body);

    //获取返回结构
    QList<QJsonObject> getHead();
    QList<QByteArray>  getData();

private:

    enum MultipartState
    {
        MPS_FIND_FIRST_DELIMETER,
        MPS_HEADER_NAME,
        MPS_HEADER_VALUE,
        MPS_HEADER_PARAMETER_NAME,
        MPS_HEADER_PARAMETER_VALUE,
        MPS_DATA_PART,
        MPS_SKIP_CRLF
    };

    char*           m_BoundStr;
    qint64          m_BoundStrLen;
    qint64          m_Indx;
    qint64          m_Remain;
    MultipartState  m_State;

    MPData*         p_data;
    //----------------------------
    //解析边界字符
    char* MPP_GetBoundary(char* theContentType, qint64* theSize);
    //初始化
    void  MPP_MultiPartInit(char* theBoundStr, qint64 theBoundStrLen);
    //解析处理器
    void  MPP_MultiPartProcess(char* theBuffer, qint64 theBufferSize);

    int   mpp_strncmp(char* theBuffer, qint64 theSize, char* theStringToFind);
    int   mpp_memncmp(char* theBuffer, qint64 theSize, char* theBuffToFind, qint64 theFindSize);
    void  mpp_trim(char* theBuffer,qint64* theStart, qint64* theSize);
    void  mpp_remove_quotas(char* theBuffer,qint64* theStart, qint64* theSize);
    void  mpp_shift(char* theBuffer);
};

#endif //MPPARSER_H
