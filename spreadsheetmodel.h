//model
#ifndef SPREADSHEETMODEL_H
#define SPREADSHEETMODEL_H
#include<QAbstractTableModel>
#include<QFile>
#include<QTextStream>

#include"celldata.h"

class SpreadsheetModel : public QAbstractTableModel{
    Q_OBJECT
public:
    explicit SpreadsheetModel(int rows=50,int cols=50,
                              QObject* parent=nullptr);

    int rowCount(const QModelIndex& ={})const override;
    int columnCount(const QModelIndex& ={})const override;
    QVariant data(const QModelIndex& index,int role)const override;

    //列头显示 A B C ...，行头显示 1 2 3 ...
    QVariant headerData(int section,Qt::Orientation orientation,int role)const override;

    bool setData(const QModelIndex& idx, const QVariant& val,
                     int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;


    void setBold      (const QModelIndexList& ids, bool v);
    void setItalic    (const QModelIndexList& ids, bool v);
    void setUnderline (const QModelIndexList& ids, bool v);
    void setFontFamily(const QModelIndexList& ids, const QString& family);
    void setFontSize  (const QModelIndexList& ids, int pt);
    void setFontColor (const QModelIndexList& ids, const QColor& color);
    void setBgColor   (const QModelIndexList& ids, const QColor& color);
    void setAlignment (const QModelIndexList& ids, Qt::Alignment align);

    CellFormat formatAt(const QModelIndex& index) const;

    bool saveCSV(const QString& filePath) const;
    bool loadCSV(const QString& filePath);
    void clearAll();

    void insertRows(int beforeRow, int count);
    void insertCols(int beforeCol, int count);
    void removeRows(int row,       int count);
    void removeCols(int col,       int count);

private:
    int m_rows,m_cols;
    QVector<QVector<CellData>> m_cells;

    // RFC-4180 helpers
    static QString  escapeCsvField(const QString& field);
    static QStringList parseCsvLine(const QString& line);

    void notifyChanged(const QModelIndexList& ids);
};





















#endif // SPREADSHEETMODEL_H
