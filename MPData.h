#ifndef MPDATA_H
#define MPDATA_H

#include <QObject>
#include <QJsonObject>

//对应 http MultiPart 格式数据结构类
//该类 不对外开放

class MPData : public QObject
{
    Q_OBJECT
public:

    explicit MPData(QObject *parent = nullptr);

    void init();
    void start();
    void setHeaderName( const QString& theName);
    void setHeaderValue(const QString& theValue);
    void setParameterName( const QString& theName);
    void setParameterValue(const QString& theValue);
    void setData(const QByteArray& theValue);
    void end();

    QList<QJsonObject> getHead() { return m_head ;}
    QList<QByteArray>  getData() { return m_data ;}

private:

    QString             m_headName;
    QString             m_paraName;
    QByteArray          m_byte;
    QJsonObject         m_unit;

    QList<QJsonObject>  m_head;
    QList<QByteArray>   m_data;
};

#endif // MPDATA_H
