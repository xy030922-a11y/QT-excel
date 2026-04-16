#include"spreadsheetmodel.h"

SpreadsheetModel::SpreadsheetModel(int rows,int cols,QObject* parent)
    :QAbstractTableModel(parent),m_rows(rows),m_cols(cols)
{
    m_cells.resize(rows);
    for(auto& row : m_cells) row.resize(cols);
}

int SpreadsheetModel::rowCount(const QModelIndex& )const{ return m_rows;}
int SpreadsheetModel::columnCount(const QModelIndex& )const { return m_cols;}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
//用来返回单元格的数据
QVariant SpreadsheetModel::data(const QModelIndex& index,int role)const {
    if(!index.isValid())    return {};
    const CellData cell=m_cells[index.row()][index.column()];

    switch(role){
    case Qt::DisplayRole:   //显示计算后的
        return cell.value.isValid() ? cell.value : QVariant{};
    case Qt::EditRole:  //显示原始公式
        return cell.formula;
    case Qt::FontRole:
        return cell.format.font;
    case Qt::ForegroundRole:
        return cell.format.fColor;
    case Qt::BackgroundRole:
        return cell.format.bgColor;
    case Qt::TextAlignmentRole:
        return static_cast<int>(cell.format.align);
    default:
        return{};
    }
}


QVariant SpreadsheetModel::headerData(int section,
                                      Qt::Orientation orientation,
                                      int role)const{
    if(role!=Qt::DisplayRole)   return {};
    if(orientation==Qt::Horizontal){
        QString label;
        int n=section;
        do{
            label.prepend(QChar('A'+n%26));
            n=n/26 -1;
        }while(n>=0);
        return label;

    }
    return section+1;
}

bool SpreadsheetModel::setData(const QModelIndex& index, const QVariant& val,
                               int role ) {
    if(!index.isValid() || role!=Qt::EditRole)  return false;

    m_cells[index.row()][index.column()].formula=val.toString();
    m_cells[index.row()][index.column()].value=val;

    emit dataChanged(index, index, {role, Qt::DisplayRole});
    return true;
}


// ── RFC-4180 CSV escape ───────────────────────────────────────────────────────
// 字段含逗号、双引号或换行时，用双引号包裹；内部双引号变成 ""
QString  SpreadsheetModel::escapeCsvField(const QString& field){
    if(field.contains(',') || field.contains('"') || field.contains('\n')){
        return  '"' + QString(field).replace('"',"\"\"") + '"';
    }
    return field;
}

// ── RFC-4180 CSV parse（处理带引号的字段）────────────────────────────────────
QStringList SpreadsheetModel::parseCsvLine(const QString& line) {
    QStringList fields;
    QString cur;
    bool inQuotes = false;
    for (int i = 0; i < line.size(); ++i) {
        QChar ch = line[i];
        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    cur += '"'; ++i;   // "" → "
                } else {
                    inQuotes = false;
                }
            } else {
                cur += ch;
            }
        } else {
            if (ch == '"') {
                inQuotes = true;
            } else if (ch == ',') {
                fields << cur;
                cur.clear();
            } else {
                cur += ch;
            }
        }
    }
    fields << cur;
    return fields;
}
bool SpreadsheetModel::saveCSV(const QString& filePath) const{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text))  return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "\xEF\xBB\xBF";   // BOM — Excel 正确识别中文

    //找到最后一个有数据的行或列，避免大量写space
    int lastRow=0,lastCol=0;
    for(int r=0;r<m_rows;r++){
        for(int c=0;c<m_cols;c++){
            if(!m_cells[r][c].isEmpty()){
                lastRow=qMax(lastRow,r);
                lastCol=qMax(lastCol,c);
            }
        }
    }

    for(int r=0;r<=lastRow;r++){
        QStringList fields;
        for(int c=0;c<=lastCol;c++){
            const QString text=m_cells[r][c].value.isValid() ?
                        m_cells[r][c].value.toString() : m_cells[r][c].formula;
            fields<<escapeCsvField(text);
        }
        out << fields.join(',') << "\r\n";
    }
    return true;
}
bool SpreadsheetModel::loadCSV(const QString& filePath){
    QFile file(filePath);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text))  return false;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    in.setAutoDetectUnicode(true);  // 自动跳过 BOM

    QVector<QStringList> csvData;
    int maxCols=0;
    QStringList fields;
    while(!in.atEnd()){
        QString line=in.readLine();
        fields=parseCsvLine(line);
        maxCols=qMax(maxCols,fields.size());
        csvData<<fields;
    }

    int newRows=qMax(m_rows,csvData.size());
    int newCols=qMax(m_cols,maxCols);

    beginResetModel();
    m_rows=newRows;
    m_cols=newCols;
    m_cells.resize(m_rows);
    for(auto& row : m_cells)    row.resize(m_cols);
//clear all
    for(auto& row : m_cells){
        for(auto& cell : row){
            cell=CellData{};
        }
    }
//load data
    for(int r=0;r<csvData.size();r++){
        const QStringList& fields=csvData[r];
        for(int c=0;c<fields.size();c++){
            m_cells[r][c].formula=fields[c];
            m_cells[r][c].value=fields[c];
        }
    }
    endResetModel();
    return true;
}

void SpreadsheetModel::clearAll(){
    beginResetModel();
    for(auto& row : m_cells){
        for(auto& cell : row){
            cell=CellData{};
        }
    }
    endResetModel();
}
void SpreadsheetModel::setBold(const QModelIndexList& ids, bool v){
    for(const auto& idx : ids){
        auto& fmt=m_cells[idx.row()][idx.column()].format;
        fmt.bold=v;
        fmt.font.setBold(v);
    }
    notifyChanged(ids);
}
void SpreadsheetModel::setItalic    (const QModelIndexList& ids, bool v){
    for(const auto& idx : ids){
        auto& fmt=m_cells[idx.row()][idx.column()].format;
        fmt.italic=v;
        fmt.font.setItalic(v);
    }
    notifyChanged(ids);
}
void SpreadsheetModel::setUnderline (const QModelIndexList& ids, bool v){
    for(const auto& idx : ids){
        auto& fmt=m_cells[idx.row()][idx.column()].format;
        fmt.underline=v;
        fmt.font.setUnderline(v);
    }
    notifyChanged(ids);
}
void SpreadsheetModel::setFontFamily(const QModelIndexList& ids, const QString& family){
    for(const auto& idx : ids){
        m_cells[idx.row()][idx.column()].format.font.setFamily(family);
    }
    notifyChanged(ids);
}
void SpreadsheetModel::setFontSize  (const QModelIndexList& ids, int pt){
    for (const auto& idx : ids){
        m_cells[idx.row()][idx.column()].format.font.setPointSize(pt);
    }
    notifyChanged(ids);
}
void SpreadsheetModel::setFontColor (const QModelIndexList& ids, const QColor& color){
    for (const auto& idx : ids){
        m_cells[idx.row()][idx.column()].format.fColor = color;
    }
    notifyChanged(ids);
}
void SpreadsheetModel::setBgColor   (const QModelIndexList& ids, const QColor& color){
    for (const auto& idx : ids){
        m_cells[idx.row()][idx.column()].format.bgColor = color;
    }
    notifyChanged(ids);
}
void SpreadsheetModel::setAlignment (const QModelIndexList& ids, Qt::Alignment align){
    for (const auto& idx : ids){
        m_cells[idx.row()][idx.column()].format.align = align;
    }
    notifyChanged(ids);
}


CellFormat SpreadsheetModel::formatAt(const QModelIndex& index) const{
    if(!index.isValid())    return {};
    return  m_cells[index.row()][index.column()].format;
}
void SpreadsheetModel::notifyChanged(const QModelIndexList& ids){
    if(ids.isEmpty())   return;

    int r0=ids[0].row(),r1=r0,c0=ids[0].column(),c1=c0;
    for(const auto& idx : ids){
        r0=qMin(r0,idx.row());
        r1=qMax(r1,idx.row());
        c0=qMin(c0,idx.column());
        c1=qMax(c1,idx.column());
    }
    emit dataChanged(index(r0,c0),index(r1,c1));
}



//----------------------插入  删除  行列----------------------------------
//插入行
void SpreadsheetModel::insertRows(int beforeRow, int count){
    if(count<=0)    return;
    beforeRow=qBound(0,beforeRow,m_rows);
    beginInsertRows(QModelIndex(),beforeRow,beforeRow+count);
    for(int i=0;i<count;i++){
        QVector<CellData> newRow(m_cols);
        m_cells.insert(beforeRow,newRow);
    }
    m_rows+=count;
    endInsertRows();
}
//插入列
void SpreadsheetModel::insertCols(int beforeCol, int count){
    if(count<=0)    return;
    beforeCol=qBound(0,beforeCol,m_cols);
    beginInsertColumns(QModelIndex(),beforeCol,beforeCol+count);
    for(auto& row : m_cells){
        for(int i=0;i<count;i++){
            row.insert(beforeCol,CellData{});
        }
    }
    m_cols+=count;
    endInsertColumns();
}
//删除行
void SpreadsheetModel::removeRows(int row,       int count){
    if(count<=0 || row<0)   return;
    beginRemoveRows(QModelIndex(),row,row+count);
    m_cells.remove(row,count);
    m_rows-=count;
    endRemoveRows();
}
//删除列
void SpreadsheetModel::removeCols(int col,       int count){
    if(count<=0 || col<0)   return;
    beginRemoveColumns(QModelIndex(),col,col+count);
    for(auto& row : m_cells){
        row.remove(col, count);
    }
    m_cols-=count;
    endRemoveColumns();
}//--------------------------------------------------------------------------------------
