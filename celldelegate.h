//单元格编辑
#ifndef CELLDELEGATE_H
#define CELLDELEGATE_H
#include<QStyledItemDelegate>

class CellDelegate: public QStyledItemDelegate{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter*,const QStyleOptionViewItem&,
               const QModelIndex&)const override;

    //显示editrole的载体
    QWidget* createEditor(QWidget* parent,
                              const QStyleOptionViewItem&,
                              const QModelIndex&) const override;
    void setEditorData(QWidget*, const QModelIndex&) const override;
    void setModelData(QWidget*, QAbstractItemModel*,
                      const QModelIndex&) const override;

};
















#endif // CELLDELEGATE_H
