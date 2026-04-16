#ifndef TOOLBARMANGER_H
#define TOOLBARMANGER_H
#include<QColor>
#include<QMainWindow>
#include<QToolButton>
#include<QAction>
#include<QFontComboBox>
#include<QComboBox>
#include<QToolBar>


class ToolbarManger : public QObject{
    Q_OBJECT
public:
    explicit ToolbarManger(QMainWindow* mainWindow, QObject* parent = nullptr);
    void setupToolbar();

    QAction* actBold        =nullptr;
    QAction* actItalic      =nullptr;
    QAction* actUnderline   =nullptr;
    QAction* actAlignleft   =nullptr;
    QAction* actAlignright  =nullptr;
    QAction* actAlignCenter =nullptr;
    QAction* actCopy        =nullptr;
    QAction* actCut         =nullptr;
    QAction* actPaste       =nullptr;

    QFontComboBox* fontCombo=nullptr;
    QComboBox*      sizeCombo=nullptr;
    QToolButton*    fontColorBtn=nullptr;
    QToolButton*    bgColotBtn=nullptr;

    QColor currentFontColor()const  {return m_fontColor;}
    QColor currentBgColor() const   {return m_bgColor;}
signals:
    void boldToggled(bool checkded);
    void italicToggled(bool checked);
    void underlineToggled(bool checked);

    void alignChanged(Qt::Alignment align);

    void copyRequested();
    void cutRequested();
    void pasteRequested();

    void fontFamilyChanged(const QString& family);
    void fontSizeChanged(const int size);
    void fontColorChanged(const QColor& fontColor);
    void bgColorChanged(const QColor& bgColor);

private slots:
    void onPickFontColor();
    void onPickBgColor();


private:
    QToolButton* makeColorToolBtn(const QString& letter,
                                  const QColor& initColor,
                                  const QString& tooltip);
    void updateColorStrip(QToolButton* btn,const QColor& color);

    QMainWindow* m_mainWindow=nullptr;
    QColor m_fontColor{Qt::black};
    QColor m_bgColor{Qt::white};

};















#endif // TOOLBARMANGER_H
