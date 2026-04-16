#include"toolbarManger.h"
#include <QActionGroup>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
#include <QToolBar>
#include <QWidget>



//create icon--bg color
static QIcon colorIcon(const QColor& color,int w=16,int h=16){
    QPixmap px(w,h);
    px.fill(color);
    return QIcon(px);
}
//create icon--font color
static QIcon letterWithStrip(const QString& letter,
                            const QColor& stripColor,
                             int w=18,int h=18){
    QPixmap px(w,h);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    QFont f;
    f.setBold(true);
    f.setPointSize(12);
    p.setFont(f);
    p.setPen(Qt::black);
    p.drawText(QRect(0,0,w,h-3),Qt::AlignCenter,letter);

    p.fillRect(QRect(0,h-2,w,2),stripColor);

    return QIcon(px);
}
//--------------------------------------------------------
ToolbarManger::ToolbarManger(QMainWindow* mainWindow,
                             QObject* parent):QObject(parent),m_mainWindow(mainWindow)
{}
//create toolbar
void ToolbarManger::setupToolbar(){
    QToolBar* tb=m_mainWindow->addToolBar(tr("Tool Bar"));
    tb->setMovable(false);
    tb->setIconSize(QSize(18,18));
    tb->setToolButtonStyle(Qt::ToolButtonIconOnly);

    //copy  cut paste
    actCopy =new QAction(tr("Copy"),this);
    actCopy->setShortcut(QKeySequence::Copy);
    actCopy->setToolTip(tr("Copy Ctrl+C"));
    tb->addAction(actCopy);
    connect(actCopy,&QAction::triggered,this,&ToolbarManger::copyRequested);

    actCut =new QAction(tr("Cut"),this);
    actCut->setShortcut(QKeySequence::Cut);
    actCut->setToolTip(tr("Cut Ctrl+X"));
    tb->addAction(actCut);
    connect(actCut,&QAction::triggered,this,&ToolbarManger::cutRequested);

    actPaste =new QAction(tr("Paste"),this);
    actPaste->setShortcut(QKeySequence::Paste);
    actPaste->setToolTip(tr("Paste Ctrl+V"));
    tb->addAction(actPaste);
    connect(actPaste,&QAction::triggered,this,&ToolbarManger::pasteRequested);
    tb->addSeparator();

//font family   font size
    fontCombo =new QFontComboBox(tb);
    fontCombo->setFixedWidth(350);
    fontCombo->setFixedHeight(50);
    fontCombo->setToolTip(tr("Font Family"));
    fontCombo->setCurrentFont(QFont("Microsoft YaHei",11));
    tb->addWidget(fontCombo);
    connect(fontCombo,&QFontComboBox::currentFontChanged,
            [this](const QFont& f){
        emit fontFamilyChanged(f.family());
    });
//font size
    sizeCombo =new QComboBox(tb);
    sizeCombo->setFixedWidth(80);
    sizeCombo->setFixedHeight(40);
    sizeCombo->setToolTip(tr("Font Size"));
    sizeCombo->setEditable(true);

    const QList<int> presetSize{
        6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20,
        22, 24, 26, 28, 36, 48, 72
    };
    for(auto& pt : presetSize){
        sizeCombo->addItem(QString::number(pt),pt);
    }
    sizeCombo->setCurrentText("11");
    tb->addWidget(sizeCombo);


    auto emitSizeChanged=[this](){
        bool ok=false;
        int pt=sizeCombo->currentText().toInt(&ok);
        if(ok && pt>0 && pt<=400)   emit fontSizeChanged(pt);
    };
    connect(sizeCombo,&QComboBox::currentTextChanged,
            [emitSizeChanged](const QString&){emitSizeChanged();});
    tb->addSeparator();

//bold ita  underline
    actBold =new QAction(tr("B"),this);
    actBold->setCheckable(true);
    actBold->setShortcut(QKeySequence::Bold);
    actBold->setToolTip(tr("Bold Ctrl+B"));
    {
        QFont bf;
        bf.setBold(true);
        bf.setPointSize(11);
        actBold->setFont(bf);
    }
    tb->addAction(actBold);
    connect(actBold,&QAction::toggled,this,&ToolbarManger::boldToggled);

    actItalic=new QAction(tr("I"),this);
    actItalic->setCheckable(true);
    actItalic->setShortcut(QKeySequence::Italic);
    actItalic->setToolTip(tr("Italic Ctrl+I"));
    {
        QFont itf;
        itf.setItalic(true);
        itf.setPointSize(11);
        actItalic->setFont(itf);
    }
    tb->addAction(actItalic);
    connect(actItalic,&QAction::toggled,this,&ToolbarManger::italicToggled);

    actUnderline=new QAction(tr("U"),this);
    actUnderline->setCheckable(true);
    actUnderline->setShortcut(QKeySequence::Underline);
    actUnderline->setToolTip(tr("Underline Crtl+U"));
    {
        QFont uf;
        uf.setUnderline(true);
        uf.setPointSize(11);
        actUnderline->setFont(uf);
    }
    tb->addAction(actUnderline);
    connect(actUnderline,&QAction::toggled,this,&ToolbarManger::underlineToggled);
    tb->addSeparator();

// font color      bg color
    //font color
    fontColorBtn=makeColorToolBtn("A",m_fontColor,tr("Font Color"));
    tb->addWidget(fontColorBtn);
    connect(fontColorBtn,&QToolButton::clicked,this,&ToolbarManger::onPickFontColor);
    //bg color
    bgColotBtn=makeColorToolBtn("H",m_bgColor,tr("Cell Background Color"));
    tb->addWidget(bgColotBtn);
    connect(bgColotBtn,&QToolButton::clicked,this,&ToolbarManger::onPickBgColor);
    tb->addSeparator();

//alignment
    auto* alignGroup=new QActionGroup(this);
    alignGroup->setExclusive(true);

    actAlignleft=new QAction(tr("L"),this);
    actAlignleft->setCheckable(true);
    actAlignleft->setToolTip(tr("AlignLeft"));
    alignGroup->addAction(actAlignleft);
    tb->addAction(actAlignleft);

    actAlignCenter=new QAction(tr("C"),this);
    actAlignCenter->setCheckable(true);
    actAlignCenter->setToolTip(tr("AlignCenter"));
    alignGroup->addAction(actAlignCenter);
    tb->addAction(actAlignCenter);

    actAlignright=new QAction(tr("R"),this);
    actAlignright->setCheckable(true);
    actAlignright->setToolTip(tr("AlignRight"));
    alignGroup->addAction(actAlignright);
    tb->addAction(actAlignright);

    connect(actAlignleft,&QAction::triggered,[this]{
        emit alignChanged(Qt::AlignLeft | Qt::AlignVCenter);});
    connect(actAlignCenter,&QAction::triggered,[this]{
        emit alignChanged(Qt::AlignHCenter | Qt::AlignVCenter);});
    connect(actAlignright,&QAction::triggered,[this]{
        emit alignChanged(Qt::AlignRight | Qt::AlignVCenter);});
}




//dialog: select font color
void ToolbarManger::onPickFontColor(){
    QColor picked=QColorDialog::getColor(
                m_fontColor,
                m_mainWindow,
                tr("Select Font Color"),
                QColorDialog::ShowAlphaChannel);
    if(!picked.isValid())   return;

    m_fontColor=picked;
    updateColorStrip(fontColorBtn,m_fontColor);
    emit fontColorChanged(m_fontColor);
}
//dialog: select bg color
void ToolbarManger::onPickBgColor(){
    QColor picked=QColorDialog::getColor(
                m_bgColor,
                m_mainWindow,
                tr("Select Background Color"),
                QColorDialog::ShowAlphaChannel);
    if(!picked.isValid())   return;

    m_bgColor=picked;
    updateColorStrip(bgColotBtn,m_bgColor);
    emit bgColorChanged(m_bgColor);
}



QToolButton* ToolbarManger::makeColorToolBtn(const QString& letter,
                              const QColor& initColor,
                              const QString& tooltip){
    auto* btn=new QToolButton(m_mainWindow);
    btn->setToolTip(tooltip);
    btn->setAutoRaise(true);
    btn->setFixedSize(50,50);
    btn->setIcon(letterWithStrip(letter,initColor));
    btn->setIconSize(QSize(50,50));

    return btn;
}
void ToolbarManger::updateColorStrip(QToolButton* btn,const QColor& color){
    btn->setIcon(letterWithStrip(btn == fontColorBtn ? "A" : "H",color));
}
