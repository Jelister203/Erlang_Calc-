#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QStackedWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QSlider>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onModelChanged(int index);
    void onTaskTypeChanged();
    void onCalculate();
    void onClear();
    void onFontSizeChanged(int value);
    void onLanguageChanged(int index);
    void onNotationChanged(int index);

private:
    QComboBox *langCombo;
    QComboBox *notationCombo;

    QComboBox    *modelCombo;
    QRadioButton *rbDirect;
    QRadioButton *rbInverseV;
    QRadioButton *rbInverseA;

    QStackedWidget *pagesStack;

    /* Страница 0: Эрланга-Б */
    QGroupBox      *pageErlang;
    QLabel         *lblV_erl,  *lblA_erl,  *lblBn_erl;
    QSpinBox       *spV_erl;
    QDoubleSpinBox *spA_erl;
    QDoubleSpinBox *spBnorm_erl;

    /* Страница 1: Энгсет */
    QGroupBox      *pageEngset;
    QLabel         *lblV_eng, *lblN_eng, *lblA0_eng, *lblBn_eng;
    QSpinBox       *spV_eng;
    QSpinBox       *spN_eng;
    QDoubleSpinBox *spA0_eng;
    QDoubleSpinBox *spBnorm_eng;

    /* Страница 2: Эрланга-А */
    QGroupBox      *pageErlangC;
    QLabel         *lblV_erlc, *lblA_erlc, *lblCn_erlc;
    QSpinBox       *spV_erlc;
    QDoubleSpinBox *spA_erlc;
    QDoubleSpinBox *spCnorm_erlc;

    /* Страница 3: Резервирование */
    QGroupBox      *pageReserv;
    QLabel         *lblV_res, *lblC_res, *lblA_res, *lblBn_res;
    QSpinBox       *spV_res;
    QSpinBox       *spC_res;
    QDoubleSpinBox *spA_res;
    QDoubleSpinBox *spBnorm_res;

    /* Страница 4: Групповое поступление */
    QGroupBox      *pageBatch;
    QLabel         *lblV_bat, *lblG_bat, *lblA_bat, *lblBn_bat;
    QSpinBox       *spV_bat;
    QSpinBox       *spG_bat;
    QDoubleSpinBox *spA_bat;
    QDoubleSpinBox *spBnorm_bat;

    QPushButton *btnCalc;
    QPushButton *btnClear;
    QSlider     *fontSlider;
    QLabel      *fontLabel;
    QLabel      *lblFontCaption;
    QLabel      *lblLang;
    QLabel      *lblNotation;

    QGroupBox *gbModel;
    QGroupBox *gbTask;
    QGroupBox *gbResult;
    QGroupBox *gbControls;

    QTextEdit *resultBox;
    int        calcCounter;

    QGroupBox *buildErlangPage();
    QGroupBox *buildEngsetPage();
    QGroupBox *buildErlangCPage();
    QGroupBox *buildReservPage();
    QGroupBox *buildBatchPage();

    void retranslate();
    void updateFieldVisibility();
    void updateInverseALabel();

    void appendResult(const QString &text);
    void calcErlang();
    void calcEngset();
    void calcErlangC();
    void calcReserv();
    void calcBatch();
};

#endif /* MAINWINDOW_H */
