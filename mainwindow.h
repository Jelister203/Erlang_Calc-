#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QStackedWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QRadioButton>

/*
 * mainwindow.h
 * Главное окно Qt5-калькулятора.
 * Вся логика отображения — здесь. Математика — только в calc_core.
 */

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

private:
    /* Верхняя панель выбора */
    QComboBox      *modelCombo;
    QRadioButton   *rbDirect;
    QRadioButton   *rbInverseV;
    QRadioButton   *rbInverseA;

    /* Контейнер страниц параметров */
    QStackedWidget *pagesStack;

    /* --- Страница 0: Эрланга-Б --- */
    QGroupBox      *pageErlang;
    QSpinBox       *spV_erl;
    QDoubleSpinBox *spA_erl;
    QDoubleSpinBox *spBnorm_erl;

    /* --- Страница 1: Энгсет --- */
    QGroupBox      *pageEngset;
    QSpinBox       *spV_eng;
    QSpinBox       *spN_eng;
    QDoubleSpinBox *spA0_eng;
    QDoubleSpinBox *spBnorm_eng;

    /* --- Страница 2: Эрланга-А (Erlang-C) --- */
    QGroupBox      *pageErlangC;
    QSpinBox       *spV_erlc;
    QDoubleSpinBox *spA_erlc;
    QDoubleSpinBox *spCnorm_erlc;

    /* --- Страница 3: Резервирование --- */
    QGroupBox      *pageReserv;
    QSpinBox       *spV_res;
    QSpinBox       *spC_res;
    QDoubleSpinBox *spA_res;
    QDoubleSpinBox *spBnorm_res;

    /* --- Страница 4: Групповое поступление --- */
    QGroupBox      *pageBatch;
    QSpinBox       *spV_bat;
    QSpinBox       *spG_bat;
    QDoubleSpinBox *spA_bat;
    QDoubleSpinBox *spBnorm_bat;

    /* Кнопки и вывод */
    QPushButton    *btnCalc;
    QPushButton    *btnClear;
    QTextEdit      *resultBox;

    /* Строители страниц */
    QGroupBox *buildErlangPage();
    QGroupBox *buildEngsetPage();
    QGroupBox *buildErlangCPage();
    QGroupBox *buildReservPage();
    QGroupBox *buildBatchPage();

    void updateFieldVisibility();

    void calcErlang();
    void calcEngset();
    void calcErlangC();
    void calcReserv();
    void calcBatch();
};

#endif /* MAINWINDOW_H */
