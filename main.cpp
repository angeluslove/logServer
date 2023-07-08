#include <QCoreApplication>
#include <QHttpServer>
#include <QFile>
#include <QDir>
#include <QFileInfoList>

#include "MPParser.h"     //复合数据解析类

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QHttpServer server;
    server.route("/", []()
    {
        return "http log server";
    });

    //处理上传
    server.route("/uplog", [](const QHttpServerRequest &request)
    {
        qDebug() << "开始上传处理";
        //多协议头
        QByteArray cth = request.value("Content-Type");
        if(cth.contains("multipart/form-data"))
        {
            //解析多头部协议
            MPParser   mpp;
            QByteArray bound = mpp.getBoundary(cth);
            QByteArray body  = request.body();
            //解析
            mpp.proces(bound,body);
            //获取解析结果
            QList<QJsonObject> list  = mpp.getHead();
            QList<QByteArray>  datas = mpp.getData();
            //循环处理
            for(int i = 0; i < datas.count(); ++i)
            {
                QJsonObject head     = list.at(i);
                QByteArray  fileData = datas.at(i);
                QString     fileName = head.value("filename").toVariant().toString();
                qDebug() << "保存文件: " << fileName << fileData.length();
                //文件存储路径
                QString path = "logs/" + fileName;
                QFile file(QDir::toNativeSeparators(path));
                //写入文件
                if(file.open(QIODevice::WriteOnly))
                {
                    file.write(fileData);
                    file.close();
                }
                else
                {
                    qDebug() << fileName << "写入文件失败!";
                }
            }
        }
        qDebug() << "上传处理完成";
        return QHttpServerResponse("upload log ok");
    });

    //列表显示
    server.route("/log/", [](const QHttpServerRequest &request)
    {
        qDebug() << "列表显示处理";
        QDir dir("logs");
        QFileInfoList flist = dir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot);
        QString url = request.url().toString();
        QString uri = "<a href='%1%2'> %2 </a> <br>";
        uri = uri.arg(url);
        QByteArray back;
        back.append("<html> <head> <meta http-equiv='Content-Type' content='text/html'; charset=utf-8 /> </head>");
        back.append("log文件列表:<br>");
        foreach(QFileInfo var , flist)
        {
            if(var.isFile())
            {
                QString temp = uri.arg(var.fileName());
                back.append(temp.toUtf8());
            }
        }
        back.append("</html>");
        return QHttpServerResponse(back);
    });

    //显示文件
    server.route("/log/<arg>", [](QString name)
    {
        qDebug() << "显示文件处理";
        if(name.isEmpty())
        {
            return QHttpServerResponse(name + " file not found");
        }
        QString path = "logs/" + name;
        QFile file(path);
        if(!file.open(QFile::ReadOnly))
        {
            return QHttpServerResponse(name + " file not found");
        }
        QByteArray data = file.readAll();
        file.close();
        return QHttpServerResponse(data);
    });

    //开始监听端口16666
    const auto port = server.listen(QHostAddress::Any, 16666);
    if(!port)
    {
        qDebug() << "Server failed to listen on a port.";
        return 0;
    }

    qDebug() << "Running on http://127.0.0.1:/" + QString::number(port);

    return a.exec();
}
