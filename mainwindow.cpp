/*
 * mainwindow.cpp
 * Реализация интерфейса. UI строится программно (без .ui-файла).
 * Математика вызывается исключительно через calc_core.h.
 */

#include "mainwindow.h"
#include "calc_core.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QString>

/* ------------------------------------------------------------------ */
/* Конструктор                                                          */
/* ------------------------------------------------------------------ */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Калькулятор пропускной способности узлов доступа");
    setMinimumWidth(520);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    /* --- Выбор модели --- */
    QGroupBox *gbModel = new QGroupBox("Модель", central);
    QHBoxLayout *modelLayout = new QHBoxLayout(gbModel);
    modelCombo = new QComboBox(gbModel);
    modelCombo->addItem("1. Эрланга-Б (потери, ∞ источников)");
    modelCombo->addItem("2. Энгсета  (потери, конечные источники)");
    modelCombo->addItem("3. Эрланга-А (ожидание, M/M/V)");
    modelCombo->addItem("4. С резервированием");
    modelCombo->addItem("5. С групповым поступлением");
    modelLayout->addWidget(modelCombo);
    mainLayout->addWidget(gbModel);

    /* --- Тип задачи --- */
    QGroupBox *gbTask = new QGroupBox("Тип задачи", central);
    QHBoxLayout *taskLayout = new QHBoxLayout(gbTask);
    rbDirect   = new QRadioButton("Прямая", gbTask);
    rbInverseV = new QRadioButton("Обратная: найти V", gbTask);
    rbInverseA = new QRadioButton("Обратная: найти A", gbTask);
    rbDirect->setChecked(true);

    QButtonGroup *bg = new QButtonGroup(gbTask);
    bg->addButton(rbDirect);
    bg->addButton(rbInverseV);
    bg->addButton(rbInverseA);

    taskLayout->addWidget(rbDirect);
    taskLayout->addWidget(rbInverseV);
    taskLayout->addWidget(rbInverseA);
    mainLayout->addWidget(gbTask);

    /* --- Страницы параметров --- */
    pagesStack = new QStackedWidget(central);
    pagesStack->addWidget(buildErlangPage());   /* 0 */
    pagesStack->addWidget(buildEngsetPage());   /* 1 */
    pagesStack->addWidget(buildErlangCPage());  /* 2 */
    pagesStack->addWidget(buildReservPage());   /* 3 */
    pagesStack->addWidget(buildBatchPage());    /* 4 */
    mainLayout->addWidget(pagesStack);

    /* --- Кнопки --- */
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnCalc  = new QPushButton("Вычислить", central);
    btnClear = new QPushButton("Очистить", central);
    btnCalc->setDefault(true);
    btnLayout->addWidget(btnCalc);
    btnLayout->addWidget(btnClear);
    mainLayout->addLayout(btnLayout);

    /* --- Результат --- */
    QGroupBox *gbResult = new QGroupBox("Результат", central);
    QVBoxLayout *resLayout = new QVBoxLayout(gbResult);
    resultBox = new QTextEdit(gbResult);
    resultBox->setReadOnly(true);
    resultBox->setMinimumHeight(140);
    resultBox->setFontFamily("Courier New");
    resLayout->addWidget(resultBox);
    mainLayout->addWidget(gbResult);

    /* --- Сигналы --- */
    connect(modelCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onModelChanged(int)));
    connect(rbDirect,   SIGNAL(toggled(bool)), this, SLOT(onTaskTypeChanged()));
    connect(rbInverseV, SIGNAL(toggled(bool)), this, SLOT(onTaskTypeChanged()));
    connect(rbInverseA, SIGNAL(toggled(bool)), this, SLOT(onTaskTypeChanged()));
    connect(btnCalc,    SIGNAL(clicked()), this, SLOT(onCalculate()));
    connect(btnClear,   SIGNAL(clicked()), this, SLOT(onClear()));

    updateFieldVisibility();
}

MainWindow::~MainWindow() {}

/* ------------------------------------------------------------------ */
/* Строители страниц                                                   */
/* ------------------------------------------------------------------ */

QGroupBox *MainWindow::buildErlangPage()
{
    pageErlang = new QGroupBox("Параметры (Эрланга-Б)");
    QFormLayout *fl = new QFormLayout(pageErlang);

    spV_erl = new QSpinBox(pageErlang);
    spV_erl->setRange(1, 9999);
    spV_erl->setValue(10);
    spV_erl->setToolTip("Ёмкость пучка (число каналов)");

    spA_erl = new QDoubleSpinBox(pageErlang);
    spA_erl->setRange(0.001, 99999.0);
    spA_erl->setDecimals(4);
    spA_erl->setValue(7.0);
    spA_erl->setToolTip("Предложенная нагрузка (Эрл)");

    spBnorm_erl = new QDoubleSpinBox(pageErlang);
    spBnorm_erl->setRange(0.0001, 0.9999);
    spBnorm_erl->setDecimals(4);
    spBnorm_erl->setValue(0.01);
    spBnorm_erl->setToolTip("Норма потерь (напр. 0.01 = 1%)");

    fl->addRow("V — ёмкость пучка:", spV_erl);
    fl->addRow("A — нагрузка (Эрл):", spA_erl);
    fl->addRow("B_норм — норма потерь:", spBnorm_erl);

    return pageErlang;
}

QGroupBox *MainWindow::buildEngsetPage()
{
    pageEngset = new QGroupBox("Параметры (Энгсет)");
    QFormLayout *fl = new QFormLayout(pageEngset);

    spV_eng = new QSpinBox(pageEngset);
    spV_eng->setRange(1, 9999);
    spV_eng->setValue(10);

    spN_eng = new QSpinBox(pageEngset);
    spN_eng->setRange(2, 99999);
    spN_eng->setValue(20);
    spN_eng->setToolTip("Число источников N (N > V)");

    spA0_eng = new QDoubleSpinBox(pageEngset);
    spA0_eng->setRange(0.0001, 9999.0);
    spA0_eng->setDecimals(4);
    spA0_eng->setValue(0.5);
    spA0_eng->setToolTip("Нагрузка одного свободного источника (Эрл)");

    spBnorm_eng = new QDoubleSpinBox(pageEngset);
    spBnorm_eng->setRange(0.0001, 0.9999);
    spBnorm_eng->setDecimals(4);
    spBnorm_eng->setValue(0.01);

    fl->addRow("V — ёмкость пучка:", spV_eng);
    fl->addRow("N — число источников:", spN_eng);
    fl->addRow("A₀ — нагрузка источника:", spA0_eng);
    fl->addRow("B_норм — норма потерь:", spBnorm_eng);

    return pageEngset;
}

QGroupBox *MainWindow::buildErlangCPage()
{
    pageErlangC = new QGroupBox("Параметры (Эрланга-А, M/M/V)");
    QFormLayout *fl = new QFormLayout(pageErlangC);

    spV_erlc = new QSpinBox(pageErlangC);
    spV_erlc->setRange(1, 9999);
    spV_erlc->setValue(10);
    spV_erlc->setToolTip("Число каналов обслуживания");

    spA_erlc = new QDoubleSpinBox(pageErlangC);
    spA_erlc->setRange(0.001, 99998.0);
    spA_erlc->setDecimals(4);
    spA_erlc->setValue(7.0);
    spA_erlc->setToolTip("Предложенная нагрузка A (Эрл). Условие устойчивости: A < V");

    spCnorm_erlc = new QDoubleSpinBox(pageErlangC);
    spCnorm_erlc->setRange(0.0001, 0.9999);
    spCnorm_erlc->setDecimals(4);
    spCnorm_erlc->setValue(0.05);
    spCnorm_erlc->setToolTip("Норма вероятности ожидания C (напр. 0.05 = 5%)");

    fl->addRow("V — число каналов:", spV_erlc);
    fl->addRow("A — нагрузка (Эрл):", spA_erlc);
    fl->addRow("C_норм — норма ожидания:", spCnorm_erlc);

    return pageErlangC;
}

QGroupBox *MainWindow::buildReservPage()
{
    pageReserv = new QGroupBox("Параметры (резервирование)");
    QFormLayout *fl = new QFormLayout(pageReserv);

    spV_res = new QSpinBox(pageReserv);
    spV_res->setRange(1, 9999);
    spV_res->setValue(10);

    spC_res = new QSpinBox(pageReserv);
    spC_res->setRange(0, 9998);
    spC_res->setValue(2);
    spC_res->setToolTip("Число зарезервированных каналов (0 ≤ c < V)");

    spA_res = new QDoubleSpinBox(pageReserv);
    spA_res->setRange(0.001, 99999.0);
    spA_res->setDecimals(4);
    spA_res->setValue(6.0);

    spBnorm_res = new QDoubleSpinBox(pageReserv);
    spBnorm_res->setRange(0.0001, 0.9999);
    spBnorm_res->setDecimals(4);
    spBnorm_res->setValue(0.01);

    fl->addRow("V — ёмкость пучка:", spV_res);
    fl->addRow("c — резерв (каналов):", spC_res);
    fl->addRow("A — нагрузка (Эрл):", spA_res);
    fl->addRow("B_норм — норма потерь:", spBnorm_res);

    return pageReserv;
}

QGroupBox *MainWindow::buildBatchPage()
{
    pageBatch = new QGroupBox("Параметры (групповое поступление)");
    QFormLayout *fl = new QFormLayout(pageBatch);

    spV_bat = new QSpinBox(pageBatch);
    spV_bat->setRange(1, 9999);
    spV_bat->setValue(12);

    spG_bat = new QSpinBox(pageBatch);
    spG_bat->setRange(1, 9999);
    spG_bat->setValue(3);
    spG_bat->setToolTip("Размер группы g (каналов на группу)");

    spA_bat = new QDoubleSpinBox(pageBatch);
    spA_bat->setRange(0.001, 99999.0);
    spA_bat->setDecimals(4);
    spA_bat->setValue(6.0);
    spA_bat->setToolTip("Суммарная предложенная нагрузка (Эрл)");

    spBnorm_bat = new QDoubleSpinBox(pageBatch);
    spBnorm_bat->setRange(0.0001, 0.9999);
    spBnorm_bat->setDecimals(4);
    spBnorm_bat->setValue(0.01);

    fl->addRow("V — ёмкость пучка:", spV_bat);
    fl->addRow("g — размер группы:", spG_bat);
    fl->addRow("A — нагрузка (Эрл):", spA_bat);
    fl->addRow("B_норм — норма потерь:", spBnorm_bat);

    return pageBatch;
}

/* ------------------------------------------------------------------ */
/* Слоты                                                               */
/* ------------------------------------------------------------------ */

void MainWindow::onModelChanged(int index)
{
    pagesStack->setCurrentIndex(index);
    updateFieldVisibility();
}

void MainWindow::onTaskTypeChanged()
{
    updateFieldVisibility();
}

void MainWindow::updateFieldVisibility()
{
    int  model  = modelCombo->currentIndex();
    bool direct = rbDirect->isChecked();
    bool inv_V  = rbInverseV->isChecked();
    bool inv_A  = rbInverseA->isChecked();

    switch (model) {
    case 0: /* Эрланга-Б */
        spV_erl->setEnabled(direct || inv_A);
        spA_erl->setEnabled(direct || inv_V);
        spBnorm_erl->setEnabled(inv_V || inv_A);
        rbInverseA->setText("Обратная: найти A");
        break;
    case 1: /* Энгсет */
        spV_eng->setEnabled(direct || inv_A);
        spN_eng->setEnabled(true);
        spA0_eng->setEnabled(direct || inv_V);
        spBnorm_eng->setEnabled(inv_V || inv_A);
        rbInverseA->setText("Обратная: найти A₀");
        break;
    case 2: /* Эрланга-А */
        spV_erlc->setEnabled(direct || inv_A);
        spA_erlc->setEnabled(direct || inv_V);
        spCnorm_erlc->setEnabled(inv_V || inv_A);
        rbInverseA->setText("Обратная: найти A");
        break;
    case 3: /* Резервирование */
        spV_res->setEnabled(direct || inv_A);
        spC_res->setEnabled(true);
        spA_res->setEnabled(direct || inv_V);
        spBnorm_res->setEnabled(inv_V || inv_A);
        rbInverseA->setText("Обратная: найти A");
        break;
    case 4: /* Групповое */
        spV_bat->setEnabled(direct || inv_A);
        spG_bat->setEnabled(true);
        spA_bat->setEnabled(direct || inv_V);
        spBnorm_bat->setEnabled(inv_V || inv_A);
        rbInverseA->setText("Обратная: найти A");
        break;
    }
}

void MainWindow::onCalculate()
{
    switch (modelCombo->currentIndex()) {
    case 0: calcErlang();  break;
    case 1: calcEngset();  break;
    case 2: calcErlangC(); break;
    case 3: calcReserv();  break;
    case 4: calcBatch();   break;
    }
}

void MainWindow::onClear()
{
    resultBox->clear();
}

/* ------------------------------------------------------------------ */
/* Вычисления                                                           */
/* ------------------------------------------------------------------ */

static QString fmtLoss(DirectResult r)
{
    return QString(
        "  B (потери):           %1\n"
        "  A_обсл (обслужено):   %2 Эрл\n"
        "  A_пот  (потеряно):    %3 Эрл\n"
        "  Использование пучка:  %4 %%"
    )
    .arg(r.B,           0, 'f', 6)
    .arg(r.A_served,    0, 'f', 4)
    .arg(r.A_lost,      0, 'f', 4)
    .arg(r.utilization * 100.0, 0, 'f', 2);
}

void MainWindow::calcErlang()
{
    QString out = "=== Модель Эрланга-Б ===\n";

    if (rbDirect->isChecked()) {
        int    V = spV_erl->value();
        double A = spA_erl->value();
        out += QString("Прямая задача: V=%1, A=%2 Эрл\n").arg(V).arg(A);
        out += fmtLoss(erlang_direct(V, A));
    }
    else if (rbInverseV->isChecked()) {
        double A = spA_erl->value(), Bn = spBnorm_erl->value();
        out += QString("Обратная (найти V): A=%1, B_норм=%2\n").arg(A).arg(Bn);
        int V = erlang_inverse_V(A, Bn);
        if (V < 0) { out += "  Ошибка: решение не найдено"; }
        else { out += QString("  Минимальное V = %1\n").arg(V); out += fmtLoss(erlang_direct(V, A)); }
    }
    else {
        int    V  = spV_erl->value();
        double Bn = spBnorm_erl->value();
        out += QString("Обратная (найти A): V=%1, B_норм=%2\n").arg(V).arg(Bn);
        double A = erlang_inverse_A(V, Bn);
        if (A < 0) { out += "  Ошибка: решение не найдено"; }
        else { out += QString("  Максимальная A = %1 Эрл\n").arg(A, 0, 'f', 4); out += fmtLoss(erlang_direct(V, A)); }
    }

    resultBox->setPlainText(out);
}

void MainWindow::calcEngset()
{
    QString out = "=== Модель Энгсета ===\n";
    int N = spN_eng->value();

    if (rbDirect->isChecked()) {
        int    V  = spV_eng->value();
        double A0 = spA0_eng->value();
        if (N <= V) { resultBox->setPlainText("Ошибка: N должно быть больше V"); return; }
        out += QString("Прямая задача: V=%1, N=%2, A0=%3 Эрл\n").arg(V).arg(N).arg(A0);
        DirectResult r = engset_direct(V, N, A0);
        out += fmtLoss(r);
        out += QString("\n  (Суммарная нагрузка A = N·A0 = %1 Эрл)").arg((double)N * A0, 0, 'f', 4);
    }
    else if (rbInverseV->isChecked()) {
        double A0 = spA0_eng->value(), Bn = spBnorm_eng->value();
        out += QString("Обратная (найти V): N=%1, A0=%2, B_норм=%3\n").arg(N).arg(A0).arg(Bn);
        int V = engset_inverse_V(N, A0, Bn);
        if (V < 0) { out += "  Ошибка: решение не найдено"; }
        else { out += QString("  Минимальное V = %1\n").arg(V); out += fmtLoss(engset_direct(V, N, A0)); }
    }
    else {
        int    V  = spV_eng->value();
        double Bn = spBnorm_eng->value();
        if (N <= V) { resultBox->setPlainText("Ошибка: N должно быть больше V"); return; }
        out += QString("Обратная (найти A0): V=%1, N=%2, B_норм=%3\n").arg(V).arg(N).arg(Bn);
        double A0 = engset_inverse_A0(V, N, Bn);
        if (A0 < 0) { out += "  Ошибка: решение не найдено"; }
        else { out += QString("  Максимальная A0 = %1 Эрл\n").arg(A0, 0, 'f', 6); out += fmtLoss(engset_direct(V, N, A0)); }
    }

    resultBox->setPlainText(out);
}

void MainWindow::calcErlangC()
{
    QString out = "=== Модель Эрланга-А (M/M/V, ожидание) ===\n";

    if (rbDirect->isChecked()) {
        int    V = spV_erlc->value();
        double A = spA_erlc->value();
        if (A >= (double)V) {
            resultBox->setPlainText("Ошибка: A должна быть меньше V (условие устойчивости rho < 1)");
            return;
        }
        out += QString("Прямая задача: V=%1, A=%2 Эрл\n").arg(V).arg(A);
        ErlangCResult r = erlang_c_direct(V, A);
        out += QString(
            "  C (вер. ожидания):    %1\n"
            "  Загрузка rho = A/V:   %2\n"
            "  Lq (в очереди):       %3\n"
            "  Wq (ожид., в ед. h):  %4\n"
            "  W  (в системе, ед.h): %5"
        )
        .arg(r.C,           0, 'f', 6)
        .arg(r.utilization, 0, 'f', 4)
        .arg(r.Lq,          0, 'f', 4)
        .arg(r.Wq,          0, 'f', 4)
        .arg(r.W,           0, 'f', 4);
    }
    else if (rbInverseV->isChecked()) {
        double A = spA_erlc->value(), Cn = spCnorm_erlc->value();
        out += QString("Обратная (найти V): A=%1, C_норм=%2\n").arg(A).arg(Cn);
        int V = erlang_c_inverse_V(A, Cn);
        if (V < 0) { out += "  Ошибка: решение не найдено"; }
        else {
            out += QString("  Минимальное V = %1\n").arg(V);
            ErlangCResult r = erlang_c_direct(V, A);
            out += QString(
                "  C (вер. ожидания):    %1\n"
                "  Загрузка rho = A/V:   %2\n"
                "  Lq (в очереди):       %3\n"
                "  Wq (ожид., в ед. h):  %4"
            )
            .arg(r.C,           0, 'f', 6)
            .arg(r.utilization, 0, 'f', 4)
            .arg(r.Lq,          0, 'f', 4)
            .arg(r.Wq,          0, 'f', 4);
        }
    }
    else {
        int    V  = spV_erlc->value();
        double Cn = spCnorm_erlc->value();
        out += QString("Обратная (найти A): V=%1, C_норм=%2\n").arg(V).arg(Cn);
        double A = erlang_c_inverse_A(V, Cn);
        if (A < 0) { out += "  Ошибка: решение не найдено"; }
        else {
            out += QString("  Максимальная A = %1 Эрл\n").arg(A, 0, 'f', 4);
            ErlangCResult r = erlang_c_direct(V, A);
            out += QString(
                "  C (вер. ожидания):    %1\n"
                "  Загрузка rho = A/V:   %2\n"
                "  Lq (в очереди):       %3\n"
                "  Wq (ожид., в ед. h):  %4"
            )
            .arg(r.C,           0, 'f', 6)
            .arg(r.utilization, 0, 'f', 4)
            .arg(r.Lq,          0, 'f', 4)
            .arg(r.Wq,          0, 'f', 4);
        }
    }

    resultBox->setPlainText(out);
}

void MainWindow::calcReserv()
{
    QString out = "=== Модель с резервированием ===\n";

    if (rbDirect->isChecked()) {
        int V = spV_res->value(), c = spC_res->value();
        double A = spA_res->value();
        if (c >= V) { resultBox->setPlainText("Ошибка: c должно быть меньше V"); return; }
        out += QString("Прямая задача: V=%1, c=%2, A=%3 Эрл\n").arg(V).arg(c).arg(A);
        out += QString("  (V_эфф = V - c = %1)\n").arg(V - c);
        out += fmtLoss(reservation_direct(V, c, A));
    }
    else if (rbInverseV->isChecked()) {
        int c = spC_res->value();
        double A = spA_res->value(), Bn = spBnorm_res->value();
        out += QString("Обратная (найти V): c=%1, A=%2, B_норм=%3\n").arg(c).arg(A).arg(Bn);
        int V = reservation_inverse_V(c, A, Bn);
        if (V < 0) { out += "  Ошибка: решение не найдено"; }
        else { out += QString("  Минимальное V = %1  (V_эфф = %2)\n").arg(V).arg(V-c); out += fmtLoss(reservation_direct(V, c, A)); }
    }
    else {
        int V = spV_res->value(), c = spC_res->value();
        double Bn = spBnorm_res->value();
        if (c >= V) { resultBox->setPlainText("Ошибка: c должно быть меньше V"); return; }
        out += QString("Обратная (найти A): V=%1, c=%2, B_норм=%3\n").arg(V).arg(c).arg(Bn);
        double A = reservation_inverse_A(V, c, Bn);
        if (A < 0) { out += "  Ошибка: решение не найдено"; }
        else { out += QString("  Максимальная A = %1 Эрл\n").arg(A, 0, 'f', 4); out += fmtLoss(reservation_direct(V, c, A)); }
    }

    resultBox->setPlainText(out);
}

void MainWindow::calcBatch()
{
    QString out = "=== Модель с групповым поступлением ===\n";

    if (rbDirect->isChecked()) {
        int V = spV_bat->value(), g = spG_bat->value();
        double A = spA_bat->value();
        if (g > V) { resultBox->setPlainText("Ошибка: g не может превышать V"); return; }
        out += QString("Прямая задача: V=%1, g=%2, A=%3 Эрл\n").arg(V).arg(g).arg(A);
        out += fmtLoss(batch_direct(V, g, A));
        out += QString("\n  (A_g = A/g = %1 Эрл на группу)").arg(A / (double)g, 0, 'f', 4);
    }
    else if (rbInverseV->isChecked()) {
        int g = spG_bat->value();
        double A = spA_bat->value(), Bn = spBnorm_bat->value();
        out += QString("Обратная (найти V): g=%1, A=%2, B_норм=%3\n").arg(g).arg(A).arg(Bn);
        int V = batch_inverse_V(g, A, Bn);
        if (V < 0) { out += "  Ошибка: решение не найдено"; }
        else { out += QString("  Минимальное V = %1\n").arg(V); out += fmtLoss(batch_direct(V, g, A)); }
    }
    else {
        int V = spV_bat->value(), g = spG_bat->value();
        double Bn = spBnorm_bat->value();
        if (g > V) { resultBox->setPlainText("Ошибка: g > V"); return; }
        out += QString("Обратная (найти A): V=%1, g=%2, B_норм=%3\n").arg(V).arg(g).arg(Bn);
        double A = batch_inverse_A(V, g, Bn);
        if (A < 0) { out += "  Ошибка: решение не найдено"; }
        else { out += QString("  Максимальная A = %1 Эрл\n").arg(A, 0, 'f', 4); out += fmtLoss(batch_direct(V, g, A)); }
    }

    resultBox->setPlainText(out);
}
