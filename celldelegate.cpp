//渲染cell

#include"celldelegate.h"
#include<QPainter>
#include <QLineEdit>
#include <QApplication>



void CellDelegate::paint(QPainter* painter,const QStyleOptionViewItem& option,
           const QModelIndex& index)const {
    //background color
    QColor  bg=index.data(Qt::BackgroundRole).value<QColor>();
    if(bg.isValid())    painter->fillRect(option.rect,bg);
    //when selected  frame color
    if(option.state & QStyle::State_Selected){
        painter->save();
        painter->setPen(QPen(QColor(0,120,215),2));
        painter->drawRect(option.rect.adjusted(1,1,-1,-1));
        painter->restore();
    }
    //draw text
    QString text=index.data(Qt::DisplayRole).toString();
    if(text.isEmpty())  return;

    QFont font=index.data(Qt::FontRole).value<QFont>();
    QColor fg=index.data(Qt::ForegroundRole).value<QColor>();
    auto align=static_cast<Qt::Alignment>(index.data(Qt::TextAlignmentRole).toInt());

    painter->save();
    painter->setFont(font);
    painter->setPen(fg.isValid()? fg : Qt::black);
    painter->drawText(option.rect.adjusted(3,0,-3,0),align,text);
}


QWidget* CellDelegate::createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
    auto* editor=new QLineEdit(parent);
    //editor->setFrame(false);
    return editor;
}
void CellDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* lineedit=qobject_cast<QLineEdit*>(editor);
    lineedit->setText(index.data(Qt::EditRole).toString());
}
void CellDelegate::setModelData(QWidget* editor,
                                QAbstractItemModel* model,
                                const QModelIndex& index) const {
    auto* le=qobject_cast<QLineEdit*>(editor);
    model->setData(index,le->text(),Qt::EditRole);
}
