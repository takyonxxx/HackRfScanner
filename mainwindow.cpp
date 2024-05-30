#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,  m_ptt(false)
    ,  ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("HackRf");

    ui->m_pBSpeak->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_pBSetFreq->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cFreqType->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cDemod->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cFreqStep->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_lEditFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #900C3F;");

    ui->m_pIncFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");
    ui->m_pDecFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");


    ui->freqCtrl->setG_constant(2.5);
    ui->freqCtrl->Setup(11, 0, 2200e6, 1, UNITS_MHZ);
    ui->freqCtrl->SetDigitColor(QColor("#FFC300"));
    ui->freqCtrl->SetFrequency(DEFAULT_FREQUENCY);

    ui->pushToggleSdr->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #097532;");
    ui->pushExit->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #900C3F;");

    ui->m_cFreqStep->addItem("1", QVariant(1));
    ui->m_cFreqStep->addItem("5", QVariant(5));
    ui->m_cFreqStep->addItem("10", QVariant(10));
    ui->m_cFreqStep->addItem("50", QVariant(50));
    ui->m_cFreqStep->addItem("100", QVariant(100));

    currentDemod    = DEMOD_WFM;
    currentFreqMod  = MHZ;
    currentFrequency = 100 * 1000 * 1000;
    freq_type_index = 2;
    demod_index = 1;

    ui->m_cFreqType->setCurrentIndex(freq_type_index);
    ui->m_cDemod->setCurrentIndex(demod_index);

//    hackRfDevice = new HackRfDevice(this);

    sdrDevice = new SdrDevice(this);
    connect(sdrDevice, &SdrDevice::infoFrequency, this, &MainWindow::infoFrequency);

    QString homePath = QDir::homePath();
    m_sSettingsFile = homePath + "/settings.ini";

    if (QFile(m_sSettingsFile).exists())
        loadSettings();
    else
        saveSettings();

}

MainWindow::~MainWindow()
{
    if (hackRfDevice) {
        delete hackRfDevice;
        hackRfDevice = nullptr;
    }
    if (sdrDevice) {
        delete sdrDevice;
        sdrDevice = nullptr;
    }
    delete ui;
}

void MainWindow::infoFrequency(int freq)
{
    ui->freqCtrl->SetFrequency(freq);
}


void MainWindow::on_pushToggleSdr_clicked()
{
    if(ui->pushToggleSdr->text() == "Start")
    {
        sdrDevice->start();
        ui->pushToggleSdr->setText("Stop");
    }
    else
    {
        sdrDevice->stop();
        ui->pushToggleSdr->setText("Start");
    }
}


void MainWindow::on_pushExit_clicked()
{
    exit(0);
}


void MainWindow::on_m_pBSpeak_clicked()
{
    if (ui->m_pBSpeak->text() == "Ptt Off")
    {
        ui->m_pBSpeak->setText("Ptt On");
        m_ptt = false;
    }
    else
    {
        ui->m_pBSpeak->setText("Ptt Off");
        m_ptt = true;
    }
}


void MainWindow::on_m_pBSetFreq_clicked()
{
    QString freq_text = ui->m_lEditFreq->text();
    bool conversionOk;

    auto freq = freq_text.toDouble(&conversionOk);
    if (currentFreqMod == FreqMod::KHZ) {
        freq = 1000 * freq;
    } else if (currentFreqMod == FreqMod::MHZ) {
        freq = 1000 * 1000 * freq;
    }else if (currentFreqMod == FreqMod::GHZ) {
        freq = 1000 * 1000 * 1000 * freq;
    }

    sdrDevice->setFrequency(freq);
    ui->freqCtrl->SetFrequency(freq);
    currentFrequency = sdrDevice->getCenterFrequency();
    saveSettings();
}


void MainWindow::on_m_pIncFreq_clicked()
{
    int selectedIndex = ui->m_cFreqStep->currentIndex();

    QVariant selectedValue = ui->m_cFreqStep->itemData(selectedIndex);
    currentFrequency = sdrDevice->getCenterFrequency();

    int selectedIntValue = selectedValue.toInt();
    if (currentFreqMod == FreqMod::KHZ) {
        selectedIntValue = 1000 * selectedIntValue;
    } else if (currentFreqMod == FreqMod::MHZ) {
        selectedIntValue = 1000 * 1000 * selectedIntValue;
    }else if (currentFreqMod == FreqMod::GHZ) {
        selectedIntValue = 1000 * 1000 * 1000 * selectedIntValue;
    }

    currentFrequency = currentFrequency + selectedIntValue;  
    sdrDevice->setFrequency(currentFrequency);
    ui->freqCtrl->SetFrequency(currentFrequency);
    saveSettings();
}


void MainWindow::on_m_pDecFreq_clicked()
{
    int selectedIndex = ui->m_cFreqStep->currentIndex();

    QVariant selectedValue = ui->m_cFreqStep->itemData(selectedIndex);
    currentFrequency = sdrDevice->getCenterFrequency();

    int selectedIntValue = selectedValue.toInt();
    if (currentFreqMod == FreqMod::KHZ) {
        selectedIntValue = 1000 * selectedIntValue;
    } else if (currentFreqMod == FreqMod::MHZ) {
        selectedIntValue = 1000 * 1000 * selectedIntValue;
    }else if (currentFreqMod == FreqMod::GHZ) {
        selectedIntValue = 1000 * 1000 * 1000 * selectedIntValue;
    }

    currentFrequency = currentFrequency - selectedIntValue;
    sdrDevice->setFrequency(currentFrequency);
    ui->freqCtrl->SetFrequency(currentFrequency);
    saveSettings();
}


void MainWindow::on_m_cFreqType_currentIndexChanged(int index)
{
    if (index >= 0 && index < ui->m_cFreqType->count())
    {
        currentFreqMod = static_cast<FreqMod>(index);
        freq_type_index = index;
        saveSettings();
    }
}


void MainWindow::on_m_cDemod_currentIndexChanged(int index)
{
    if (index >= 0 && index < ui->m_cDemod->count())
    {
        currentDemod = static_cast<Demod>(index);
        demod_index = index;
        saveSettings();
    }
}

void MainWindow::loadSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    freq_type_index = settings.value("freq_type_index", "").toString().toInt();
    demod_index = settings.value("demod_index", "").toString().toInt();
    currentFrequency = settings.value("current_frequency", "").toString().toDouble();
    ui->m_cFreqType->setCurrentIndex(freq_type_index);
    ui->m_cDemod->setCurrentIndex(demod_index);

    auto freq = 0.0;
    if (currentFreqMod == FreqMod::KHZ) {
        freq = currentFrequency / 1000.0;
    } else if (currentFreqMod == FreqMod::MHZ) {
        freq = currentFrequency / 1000.0 / 1000.0;
    }else if (currentFreqMod == FreqMod::GHZ) {
        freq = currentFrequency / 1000.0 / 1000.0 / 1000.0;
    }
    ui->m_lEditFreq->setText(QString::number(freq, 'f', 2));

    sdrDevice->setFrequency(currentFrequency);
    ui->freqCtrl->SetFrequency(currentFrequency);
}

void MainWindow::saveSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.setValue("freq_type_index", QString::number(freq_type_index));
    settings.setValue("demod_index", QString::number(demod_index));
    settings.setValue("current_frequency", QString::number(currentFrequency));
}
