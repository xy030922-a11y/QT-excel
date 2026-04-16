//单元格数据结构
#ifndef CELLDATA_H
#define CELLDATA_H
#include<QFont>
#include<QColor>
#include<QVariant>
#include<QString>


struct CellFormat{
    QFont font;
    QColor fColor =Qt::black;
    QColor bgColor=Qt::white;
    Qt::Alignment align=Qt::AlignLeft | Qt::AlignVCenter;

    bool bold = false;
    bool italic=false;
    bool underline=false;
};

struct CellData{
    QString formula;//显示原始输入，1+2
    QVariant value;//显示计算后的值
    CellFormat format;

    bool isEmpty()const{return formula.isEmpty();}
};



#endif // CELLDATA_H
