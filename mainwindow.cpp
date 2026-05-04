/*
 * mainwindow.cpp
 * Все строки берутся через AppLocale::instance().s("key").
 * retranslate() обновляет тексты всех виджетов без пересоздания UI.
 */

#include "mainwindow.h"
#include "app_locale.h"
#include "calc_core.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QFont>
#include <QApplication>
#include <QApplication>

#define L(key) AppLocale::instance().s(key)

static QString N(double v, int prec = 6)
{
    return QString::number(v, 'g', prec);
}

/* ================================================================== */
/* Конструктор                                                          */
/* ================================================================== */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), calcCounter(0)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    /* ── Панель управления ── */
    gbControls = new QGroupBox(central);
    QHBoxLayout *ctrlLayout = new QHBoxLayout(gbControls);

    lblLang     = new QLabel(gbControls);
    langCombo   = new QComboBox(gbControls);
    langCombo->addItem("RU");
    langCombo->addItem("EN");

    lblNotation    = new QLabel(gbControls);
    notationCombo  = new QComboBox(gbControls);
    notationCombo->addItem("λ, μ, ρ …");
    notationCombo->addItem("A, V, B …");

    lblFontCaption = new QLabel(gbControls);
    fontSlider     = new QSlider(Qt::Horizontal, gbControls);
    fontSlider->setRange(6, 20);
    fontSlider->setValue(14);
    fontSlider->setFixedWidth(90);
    fontLabel = new QLabel("14pt", gbControls);
    fontLabel->setFixedWidth(30);

    ctrlLayout->addWidget(lblLang);
    ctrlLayout->addWidget(langCombo);
    ctrlLayout->addSpacing(12);
    ctrlLayout->addWidget(lblNotation);
    ctrlLayout->addWidget(notationCombo);
    ctrlLayout->addStretch();
    ctrlLayout->addWidget(lblFontCaption);
    ctrlLayout->addWidget(fontSlider);
    ctrlLayout->addWidget(fontLabel);
    mainLayout->addWidget(gbControls);

    /* ── Выбор модели ── */
    gbModel = new QGroupBox(central);
    QHBoxLayout *modelLayout = new QHBoxLayout(gbModel);
    modelCombo = new QComboBox(gbModel);
    for (int i = 0; i < 5; ++i) modelCombo->addItem("");
    modelLayout->addWidget(modelCombo);
    mainLayout->addWidget(gbModel);

    /* ── Тип задачи ── */
    gbTask = new QGroupBox(central);
    QHBoxLayout *taskLayout = new QHBoxLayout(gbTask);
    rbDirect   = new QRadioButton(gbTask);
    rbInverseV = new QRadioButton(gbTask);
    rbInverseA = new QRadioButton(gbTask);
    rbDirect->setChecked(true);

    QButtonGroup *bg = new QButtonGroup(gbTask);
    bg->addButton(rbDirect);
    bg->addButton(rbInverseV);
    bg->addButton(rbInverseA);

    taskLayout->addWidget(rbDirect);
    taskLayout->addWidget(rbInverseV);
    taskLayout->addWidget(rbInverseA);
    mainLayout->addWidget(gbTask);

    /* ── Страницы параметров ── */
    pagesStack = new QStackedWidget(central);
    pagesStack->addWidget(buildErlangPage());
    pagesStack->addWidget(buildEngsetPage());
    pagesStack->addWidget(buildErlangCPage());
    pagesStack->addWidget(buildReservPage());
    pagesStack->addWidget(buildBatchPage());
    mainLayout->addWidget(pagesStack);

    /* ── Кнопки ── */
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnCalc  = new QPushButton(central);
    btnClear = new QPushButton(central);
    btnCalc->setDefault(true);
    btnLayout->addWidget(btnCalc);
    btnLayout->addWidget(btnClear);
    mainLayout->addLayout(btnLayout);

    /* ── История ── */
    gbResult = new QGroupBox(central);
    QVBoxLayout *resLayout = new QVBoxLayout(gbResult);
    resultBox = new QTextEdit(gbResult);
    resultBox->setReadOnly(true);
    resultBox->setMinimumHeight(200);
    resultBox->setFont(QFont("Courier New", 14));
    resLayout->addWidget(resultBox);
    mainLayout->addWidget(gbResult);

    /* ── Сигналы ── */
    connect(modelCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(onModelChanged(int)));
    connect(rbDirect,      SIGNAL(toggled(bool)),            this, SLOT(onTaskTypeChanged()));
    connect(rbInverseV,    SIGNAL(toggled(bool)),            this, SLOT(onTaskTypeChanged()));
    connect(rbInverseA,    SIGNAL(toggled(bool)),            this, SLOT(onTaskTypeChanged()));
    connect(btnCalc,       SIGNAL(clicked()),                this, SLOT(onCalculate()));
    connect(btnClear,      SIGNAL(clicked()),                this, SLOT(onClear()));
    connect(fontSlider,    SIGNAL(valueChanged(int)),        this, SLOT(onFontSizeChanged(int)));
    connect(langCombo,     SIGNAL(currentIndexChanged(int)), this, SLOT(onLanguageChanged(int)));
    connect(notationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onNotationChanged(int)));

    retranslate();
    setMinimumWidth(560);
}

MainWindow::~MainWindow() {}

/* ================================================================== */
/* Строители страниц                                                    */
/* ================================================================== */

QGroupBox *MainWindow::buildErlangPage()
{
    pageErlang = new QGroupBox;
    QFormLayout *fl = new QFormLayout(pageErlang);

    spV_erl     = new QSpinBox(pageErlang);
    spV_erl->setRange(1, 9999); spV_erl->setValue(10);
    spA_erl     = new QDoubleSpinBox(pageErlang);
    spA_erl->setRange(0.001, 99999.0); spA_erl->setDecimals(4); spA_erl->setValue(7.0);
    spBnorm_erl = new QDoubleSpinBox(pageErlang);
    spBnorm_erl->setRange(0.0001, 0.9999); spBnorm_erl->setDecimals(4); spBnorm_erl->setValue(0.01);

    lblV_erl  = new QLabel(pageErlang);
    lblA_erl  = new QLabel(pageErlang);
    lblBn_erl = new QLabel(pageErlang);

    fl->addRow(lblV_erl,  spV_erl);
    fl->addRow(lblA_erl,  spA_erl);
    fl->addRow(lblBn_erl, spBnorm_erl);
    return pageErlang;
}

QGroupBox *MainWindow::buildEngsetPage()
{
    pageEngset = new QGroupBox;
    QFormLayout *fl = new QFormLayout(pageEngset);

    spV_eng     = new QSpinBox(pageEngset);
    spV_eng->setRange(1, 9999); spV_eng->setValue(10);
    spN_eng     = new QSpinBox(pageEngset);
    spN_eng->setRange(2, 99999); spN_eng->setValue(20);
    spA0_eng    = new QDoubleSpinBox(pageEngset);
    spA0_eng->setRange(0.0001, 9999.0); spA0_eng->setDecimals(4); spA0_eng->setValue(0.5);
    spBnorm_eng = new QDoubleSpinBox(pageEngset);
    spBnorm_eng->setRange(0.0001, 0.9999); spBnorm_eng->setDecimals(4); spBnorm_eng->setValue(0.01);

    lblV_eng  = new QLabel(pageEngset);
    lblN_eng  = new QLabel(pageEngset);
    lblA0_eng = new QLabel(pageEngset);
    lblBn_eng = new QLabel(pageEngset);

    fl->addRow(lblV_eng,  spV_eng);
    fl->addRow(lblN_eng,  spN_eng);
    fl->addRow(lblA0_eng, spA0_eng);
    fl->addRow(lblBn_eng, spBnorm_eng);
    return pageEngset;
}

QGroupBox *MainWindow::buildErlangCPage()
{
    pageErlangC = new QGroupBox;
    QFormLayout *fl = new QFormLayout(pageErlangC);

    spV_erlc     = new QSpinBox(pageErlangC);
    spV_erlc->setRange(1, 9999); spV_erlc->setValue(10);
    spA_erlc     = new QDoubleSpinBox(pageErlangC);
    spA_erlc->setRange(0.001, 99998.0); spA_erlc->setDecimals(4); spA_erlc->setValue(7.0);
    spCnorm_erlc = new QDoubleSpinBox(pageErlangC);
    spCnorm_erlc->setRange(0.0001, 0.9999); spCnorm_erlc->setDecimals(4); spCnorm_erlc->setValue(0.05);

    lblV_erlc  = new QLabel(pageErlangC);
    lblA_erlc  = new QLabel(pageErlangC);
    lblCn_erlc = new QLabel(pageErlangC);

    fl->addRow(lblV_erlc,  spV_erlc);
    fl->addRow(lblA_erlc,  spA_erlc);
    fl->addRow(lblCn_erlc, spCnorm_erlc);
    return pageErlangC;
}

QGroupBox *MainWindow::buildReservPage()
{
    pageReserv = new QGroupBox;
    QFormLayout *fl = new QFormLayout(pageReserv);

    spV_res     = new QSpinBox(pageReserv);
    spV_res->setRange(1, 9999); spV_res->setValue(10);
    spC_res     = new QSpinBox(pageReserv);
    spC_res->setRange(0, 9998); spC_res->setValue(2);
    spA_res     = new QDoubleSpinBox(pageReserv);
    spA_res->setRange(0.001, 99999.0); spA_res->setDecimals(4); spA_res->setValue(6.0);
    spBnorm_res = new QDoubleSpinBox(pageReserv);
    spBnorm_res->setRange(0.0001, 0.9999); spBnorm_res->setDecimals(4); spBnorm_res->setValue(0.01);

    lblV_res  = new QLabel(pageReserv);
    lblC_res  = new QLabel(pageReserv);
    lblA_res  = new QLabel(pageReserv);
    lblBn_res = new QLabel(pageReserv);

    fl->addRow(lblV_res,  spV_res);
    fl->addRow(lblC_res,  spC_res);
    fl->addRow(lblA_res,  spA_res);
    fl->addRow(lblBn_res, spBnorm_res);
    return pageReserv;
}

QGroupBox *MainWindow::buildBatchPage()
{
    pageBatch = new QGroupBox;
    QFormLayout *fl = new QFormLayout(pageBatch);

    spV_bat     = new QSpinBox(pageBatch);
    spV_bat->setRange(1, 9999); spV_bat->setValue(12);
    spG_bat     = new QSpinBox(pageBatch);
    spG_bat->setRange(1, 9999); spG_bat->setValue(3);
    spA_bat     = new QDoubleSpinBox(pageBatch);
    spA_bat->setRange(0.001, 99999.0); spA_bat->setDecimals(4); spA_bat->setValue(6.0);
    spBnorm_bat = new QDoubleSpinBox(pageBatch);
    spBnorm_bat->setRange(0.0001, 0.9999); spBnorm_bat->setDecimals(4); spBnorm_bat->setValue(0.01);

    lblV_bat  = new QLabel(pageBatch);
    lblG_bat  = new QLabel(pageBatch);
    lblA_bat  = new QLabel(pageBatch);
    lblBn_bat = new QLabel(pageBatch);

    fl->addRow(lblV_bat,  spV_bat);
    fl->addRow(lblG_bat,  spG_bat);
    fl->addRow(lblA_bat,  spA_bat);
    fl->addRow(lblBn_bat, spBnorm_bat);
    return pageBatch;
}

/* ================================================================== */
/* retranslate                                                         */
/* ================================================================== */

void MainWindow::retranslate()
{
    setWindowTitle(L("app_title"));

    gbControls->setTitle(L("group_controls"));
    gbModel->setTitle(L("group_model"));
    gbTask->setTitle(L("group_task"));
    gbResult->setTitle(L("group_result"));

    lblLang->setText(L("lbl_lang"));
    lblNotation->setText(L("lbl_notation"));
    lblFontCaption->setText(L("lbl_font"));
    fontSlider->setToolTip(L("tip_font"));

    modelCombo->setItemText(0, L("model_erlang"));
    modelCombo->setItemText(1, L("model_engset"));
    modelCombo->setItemText(2, L("model_erlangc"));
    modelCombo->setItemText(3, L("model_reserv"));
    modelCombo->setItemText(4, L("model_batch"));

    rbDirect->setText(L("rb_direct"));
    rbInverseV->setText(L("rb_inv_v"));
    updateInverseALabel();

    btnCalc->setText(L("btn_calc"));
    btnClear->setText(L("btn_clear"));

    pageErlang->setTitle(L("model_erlang"));
    lblV_erl->setText(L("param_V"));
    lblA_erl->setText(L("param_A_erlang"));
    lblBn_erl->setText(L("param_Bnorm"));
    spV_erl->setToolTip(L("tip_V"));
    spA_erl->setToolTip(L("tip_A_erlang"));
    spBnorm_erl->setToolTip(L("tip_Bnorm"));

    pageEngset->setTitle(L("model_engset"));
    lblV_eng->setText(L("param_V"));
    lblN_eng->setText(L("param_N"));
    lblA0_eng->setText(L("param_A0"));
    lblBn_eng->setText(L("param_Bnorm"));
    spV_eng->setToolTip(L("tip_V"));
    spN_eng->setToolTip(L("tip_N"));
    spA0_eng->setToolTip(L("tip_A0"));
    spBnorm_eng->setToolTip(L("tip_Bnorm"));

    pageErlangC->setTitle(L("model_erlangc"));
    lblV_erlc->setText(L("param_V"));
    lblA_erlc->setText(L("param_A_erlangc"));
    lblCn_erlc->setText(L("param_Cnorm"));
    spV_erlc->setToolTip(L("tip_V"));
    spA_erlc->setToolTip(L("tip_A_erlangc"));
    spCnorm_erlc->setToolTip(L("tip_Cnorm"));

    pageReserv->setTitle(L("model_reserv"));
    lblV_res->setText(L("param_V"));
    lblC_res->setText(L("param_c"));
    lblA_res->setText(L("param_A_reserv"));
    lblBn_res->setText(L("param_Bnorm_res"));
    spV_res->setToolTip(L("tip_V"));
    spC_res->setToolTip(L("tip_c"));
    spA_res->setToolTip(L("tip_A_erlang"));
    spBnorm_res->setToolTip(L("tip_Bnorm"));

    pageBatch->setTitle(L("model_batch"));
    lblV_bat->setText(L("param_V"));
    lblG_bat->setText(L("param_g"));
    lblA_bat->setText(L("param_A_batch"));
    lblBn_bat->setText(L("param_Bnorm_bat"));
    spV_bat->setToolTip(L("tip_V"));
    spG_bat->setToolTip(L("tip_g"));
    spA_bat->setToolTip(L("tip_A_batch"));
    spBnorm_bat->setToolTip(L("tip_Bnorm"));
}

void MainWindow::updateInverseALabel()
{
    int model = modelCombo->currentIndex();
    switch (model) {
    case 1:  rbInverseA->setText(L("rb_inv_a_engset"));  break;
    case 2:  rbInverseA->setText(L("rb_inv_a_erlangc")); break;
    case 3:  rbInverseA->setText(L("rb_inv_a_reserv"));  break;
    case 4:  rbInverseA->setText(L("rb_inv_a_batch"));   break;
    default: rbInverseA->setText(L("rb_inv_a_erlang"));  break;
    }
}

/* ================================================================== */
/* Слоты                                                               */
/* ================================================================== */

void MainWindow::onLanguageChanged(int index)
{
    AppLocale::instance().setLanguage(index == 0 ? AppLocale::RU : AppLocale::EN);
    retranslate();
}

void MainWindow::onNotationChanged(int index)
{
    AppLocale::instance().setNotation(index == 0 ? AppLocale::CLASSIC : AppLocale::LATIN);
    retranslate();
}

void MainWindow::onModelChanged(int index)
{
    pagesStack->setCurrentIndex(index);
    updateInverseALabel();
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
    case 0:
        spV_erl->setEnabled(direct || inv_A);
        spA_erl->setEnabled(direct || inv_V);
        spBnorm_erl->setEnabled(inv_V || inv_A);
        break;
    case 1:
        spV_eng->setEnabled(direct || inv_A);
        spN_eng->setEnabled(true);
        spA0_eng->setEnabled(direct || inv_V);
        spBnorm_eng->setEnabled(inv_V || inv_A);
        break;
    case 2:
        spV_erlc->setEnabled(direct || inv_A);
        spA_erlc->setEnabled(direct || inv_V);
        spCnorm_erlc->setEnabled(inv_V || inv_A);
        break;
    case 3:
        spV_res->setEnabled(direct || inv_A);
        spC_res->setEnabled(true);
        spA_res->setEnabled(direct || inv_V);
        spBnorm_res->setEnabled(inv_V || inv_A);
        break;
    case 4:
        spV_bat->setEnabled(direct || inv_A);
        spG_bat->setEnabled(true);
        spA_bat->setEnabled(direct || inv_V);
        spBnorm_bat->setEnabled(inv_V || inv_A);
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
    calcCounter = 0;
}

void MainWindow::onFontSizeChanged(int value)
{
    fontLabel->setText(QString("%1pt").arg(value));

    /* Меняем шрифт всего приложения */
    QFont appFont = QApplication::font();
    appFont.setPointSize(value);
    QApplication::setFont(appFont);

    /* Для поля вывода отдельно сохраняем моноширинный шрифт */
    QFont monoFont("Courier New", value);
    resultBox->setFont(monoFont);
}

/* ================================================================== */
/* История                                                             */
/* ================================================================== */

void MainWindow::appendResult(const QString &text)
{
    ++calcCounter;
    QString sep = QString("─── %1 #%2 ───────────────────────\n")
                  .arg(L("calc_label")).arg(calcCounter);
    resultBox->append(sep + text + "\n");
    QTextCursor c = resultBox->textCursor();
    c.movePosition(QTextCursor::End);
    resultBox->setTextCursor(c);
}

/* ================================================================== */
/* Форматирование результатов                                          */
/* ================================================================== */

static QString fmtLoss(DirectResult r, double A, const QString &unit)
{
    auto pad = [](const QString &s) { return s.leftJustified(22, ' '); };
    return QString("  %1 %2\n  %3 %4 %5\n  %6 %7 %5\n  %8 %9 %5\n  %10 %11%")
        .arg(pad(L("res_B")))   .arg(N(r.B))
        .arg(pad(L("res_A")))   .arg(N(A))           .arg(unit)
        .arg(pad(L("res_A1")))  .arg(N(r.A_served))
        .arg(pad(L("res_A0")))  .arg(N(r.A_lost))
        .arg(pad(L("res_eta"))) .arg(N(r.utilization * 100.0, 4));
}

static QString fmtDelay(ErlangCResult r)
{
    auto pad = [](const QString &s) { return s.leftJustified(22, ' '); };
    return QString("  %1 %2\n  %3 %4\n  %5 %6\n  %7 %8\n  %9 %10")
        .arg(pad(L("res_C")))   .arg(N(r.C))
        .arg(pad(L("res_rho"))) .arg(N(r.utilization))
        .arg(pad(L("res_Lq")))  .arg(N(r.Lq))
        .arg(pad(L("res_Wq")))  .arg(N(r.Wq))
        .arg(pad(L("res_W")))   .arg(N(r.W));
}

/* ================================================================== */
/* Вычисления                                                          */
/* ================================================================== */

void MainWindow::calcErlang()
{
    QString out  = L("hdr_erlang") + "\n";
    QString unit = L("unit_erl");

    if (rbDirect->isChecked()) {
        int V = spV_erl->value(); double A = spA_erl->value();
        out += QString("%1: V=%2, A=%3 %4\n").arg(L("task_direct")).arg(V).arg(N(A)).arg(unit);
        out += fmtLoss(erlang_direct(V, A), A, unit);
    }
    else if (rbInverseV->isChecked()) {
        double A = spA_erl->value(), Bn = spBnorm_erl->value();
        out += QString("%1: A=%2 %3, B*=%4\n").arg(L("task_inv_v")).arg(N(A)).arg(unit).arg(N(Bn));
        int V = erlang_inverse_V(A, Bn);
        if (V < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2\n").arg(L("res_v_min")).arg(V);
            out += fmtLoss(erlang_direct(V, A), A, unit);
        }
    }
    else {
        int V = spV_erl->value(); double Bn = spBnorm_erl->value();
        out += QString("%1: V=%2, B*=%3\n").arg(L("task_inv_a")).arg(V).arg(N(Bn));
        double A = erlang_inverse_A(V, Bn);
        if (A < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2 %3\n").arg(L("res_a_max")).arg(N(A)).arg(unit);
            out += fmtLoss(erlang_direct(V, A), A, unit);
        }
    }
    appendResult(out);
}

void MainWindow::calcEngset()
{
    QString out  = L("hdr_engset") + "\n";
    QString unit = L("unit_erl");
    int Nsrc = spN_eng->value();

    if (rbDirect->isChecked()) {
        int V = spV_eng->value(); double A0 = spA0_eng->value();
        if (Nsrc <= V) { appendResult(L("err_n_gt_v")); return; }
        double Atot = Nsrc * A0;
        out += QString("%1: V=%2, N=%3, A0=%4 %5\n")
               .arg(L("task_direct")).arg(V).arg(Nsrc).arg(N(A0)).arg(unit);
        out += QString("  %1 = %2 %3\n").arg(L("res_A_total")).arg(N(Atot)).arg(unit);
        out += fmtLoss(engset_direct(V, Nsrc, A0), Atot, unit);
    }
    else if (rbInverseV->isChecked()) {
        double A0 = spA0_eng->value(), Bn = spBnorm_eng->value();
        out += QString("%1: N=%2, A0=%3 %4, B*=%5\n")
               .arg(L("task_inv_v")).arg(Nsrc).arg(N(A0)).arg(unit).arg(N(Bn));
        int V = engset_inverse_V(Nsrc, A0, Bn);
        if (V < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2\n").arg(L("res_v_min")).arg(V);
            out += fmtLoss(engset_direct(V, Nsrc, A0), Nsrc * A0, unit);
        }
    }
    else {
        int V = spV_eng->value(); double Bn = spBnorm_eng->value();
        if (Nsrc <= V) { appendResult(L("err_n_gt_v")); return; }
        out += QString("%1: V=%2, N=%3, B*=%4\n")
               .arg(L("task_inv_a0")).arg(V).arg(Nsrc).arg(N(Bn));
        double A0 = engset_inverse_A0(V, Nsrc, Bn);
        if (A0 < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2 %3\n").arg(L("res_a0_max")).arg(N(A0)).arg(unit);
            out += fmtLoss(engset_direct(V, Nsrc, A0), Nsrc * A0, unit);
        }
    }
    appendResult(out);
}

void MainWindow::calcErlangC()
{
    QString out  = L("hdr_erlangc") + "\n";
    QString unit = L("unit_erl");

    if (rbDirect->isChecked()) {
        int V = spV_erlc->value(); double A = spA_erlc->value();
        if (A >= V) { appendResult(L("err_unstable")); return; }
        out += QString("%1: V=%2, A=%3 %4, %5=%6\n")
               .arg(L("task_direct")).arg(V).arg(N(A)).arg(unit)
               .arg(L("res_rho_val")).arg(N(A / V));
        out += fmtDelay(erlang_c_direct(V, A));
    }
    else if (rbInverseV->isChecked()) {
        double A = spA_erlc->value(), Cn = spCnorm_erlc->value();
        out += QString("%1: A=%2 %3, C*=%4\n")
               .arg(L("task_inv_v")).arg(N(A)).arg(unit).arg(N(Cn));
        int V = erlang_c_inverse_V(A, Cn);
        if (V < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2  (%3=%4)\n")
                   .arg(L("res_v_min")).arg(V).arg(L("res_rho_val")).arg(N(A / V));
            out += fmtDelay(erlang_c_direct(V, A));
        }
    }
    else {
        int V = spV_erlc->value(); double Cn = spCnorm_erlc->value();
        out += QString("%1: V=%2, C*=%3\n").arg(L("task_inv_a")).arg(V).arg(N(Cn));
        double A = erlang_c_inverse_A(V, Cn);
        if (A < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2 %3  (%4=%5)\n")
                   .arg(L("res_a_max")).arg(N(A)).arg(unit)
                   .arg(L("res_rho_val")).arg(N(A / V));
            out += fmtDelay(erlang_c_direct(V, A));
        }
    }
    appendResult(out);
}

void MainWindow::calcReserv()
{
    QString out  = L("hdr_reserv") + "\n";
    QString unit = L("unit_erl");

    if (rbDirect->isChecked()) {
        int V = spV_res->value(), c = spC_res->value(); double A = spA_res->value();
        if (c >= V) { appendResult(L("err_c_lt_v")); return; }
        out += QString("%1: V=%2, c=%3, %4=%5, A=%6 %7\n")
               .arg(L("task_direct")).arg(V).arg(c)
               .arg(L("res_v_eff")).arg(V - c).arg(N(A)).arg(unit);
        out += fmtLoss(reservation_direct(V, c, A), A, unit);
    }
    else if (rbInverseV->isChecked()) {
        int c = spC_res->value(); double A = spA_res->value(), Bn = spBnorm_res->value();
        out += QString("%1: c=%2, A=%3 %4, B*=%5\n")
               .arg(L("task_inv_v")).arg(c).arg(N(A)).arg(unit).arg(N(Bn));
        int V = reservation_inverse_V(c, A, Bn);
        if (V < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2  (%3=%4)\n")
                   .arg(L("res_v_min")).arg(V).arg(L("res_v_eff")).arg(V - c);
            out += fmtLoss(reservation_direct(V, c, A), A, unit);
        }
    }
    else {
        int V = spV_res->value(), c = spC_res->value(); double Bn = spBnorm_res->value();
        if (c >= V) { appendResult(L("err_c_lt_v")); return; }
        out += QString("%1: V=%2, c=%3, B*=%4\n")
               .arg(L("task_inv_a")).arg(V).arg(c).arg(N(Bn));
        double A = reservation_inverse_A(V, c, Bn);
        if (A < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2 %3\n").arg(L("res_a_max")).arg(N(A)).arg(unit);
            out += fmtLoss(reservation_direct(V, c, A), A, unit);
        }
    }
    appendResult(out);
}

void MainWindow::calcBatch()
{
    QString out  = L("hdr_batch") + "\n";
    QString unit = L("unit_erl");

    if (rbDirect->isChecked()) {
        int V = spV_bat->value(), g = spG_bat->value(); double A = spA_bat->value();
        if (g > V) { appendResult(L("err_g_le_v")); return; }
        out += QString("%1: V=%2, g=%3, A=%4 %5\n")
               .arg(L("task_direct")).arg(V).arg(g).arg(N(A)).arg(unit);
        out += fmtLoss(batch_direct(V, g, A), A, unit);
        out += QString("\n  %1 = %2 %3").arg(L("res_Ag")).arg(N(A / g)).arg(unit);
    }
    else if (rbInverseV->isChecked()) {
        int g = spG_bat->value(); double A = spA_bat->value(), Bn = spBnorm_bat->value();
        out += QString("%1: g=%2, A=%3 %4, B*=%5\n")
               .arg(L("task_inv_v")).arg(g).arg(N(A)).arg(unit).arg(N(Bn));
        int V = batch_inverse_V(g, A, Bn);
        if (V < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2\n").arg(L("res_v_min")).arg(V);
            out += fmtLoss(batch_direct(V, g, A), A, unit);
        }
    }
    else {
        int V = spV_bat->value(), g = spG_bat->value(); double Bn = spBnorm_bat->value();
        if (g > V) { appendResult(L("err_g_le_v")); return; }
        out += QString("%1: V=%2, g=%3, B*=%4\n")
               .arg(L("task_inv_a")).arg(V).arg(g).arg(N(Bn));
        double A = batch_inverse_A(V, g, Bn);
        if (A < 0) out += L("err_no_solution");
        else {
            out += QString("  %1 = %2 %3\n").arg(L("res_a_max")).arg(N(A)).arg(unit);
            out += fmtLoss(batch_direct(V, g, A), A, unit);
        }
    }
    appendResult(out);
}
