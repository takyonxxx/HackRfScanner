#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,  m_ptt(false)
    ,  ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("HackRf");

    ui->m_pBPtt->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #FF5733;");
    ui->m_cFreqType->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cDemod->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cFreqStep->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");

    ui->m_pIncFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");
    ui->m_pDecFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");
    ui->labelFreqStep->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #FF5733;");

    ui->m_cFreqType->setMinimumHeight(30);
    ui->m_cDemod->setMinimumHeight(30);
    ui->m_cFreqStep->setMinimumHeight(40);
    ui->m_cFreqStep->setMaximumWidth(80);
    ui->labelFreqStep->setMinimumHeight(40);
    ui->labelFreqStep->setMaximumWidth(60);

    ui->pushToggleSdr->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #097532;");
    ui->pushExit->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #900C3F;");

    ui->sMeter->setMinimumHeight(60);
    ui->sMeter->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #350E06;");

    currentDemod    = DEMOD_WFM;
    currentFreqMod  = MHZ;    
    freq_type_index = 2;
    demod_index = 1;
    d_iqFftData.resize(DEFAULT_FFT_SIZE);

    ui->m_cDemod->setCurrentIndex(demod_index);

    QString homePath = QDir::homePath();
    m_sSettingsFile = homePath + "/settings.ini";

    sdrDevice = new SdrDevice(this);
    connect(sdrDevice, &SdrDevice::infoFrequency, this, &MainWindow::infoFrequency);
    connect(sdrDevice, &SdrDevice::rxBuffer, this, &MainWindow::getRxBuffer);    

    if (QFile(m_sSettingsFile).exists())
        loadSettings();
    else
        saveSettings();

    ui->m_cFreqStep->addItem("1", QVariant(1));
    ui->m_cFreqStep->addItem("5", QVariant(5));
    ui->m_cFreqStep->addItem("10", QVariant(10));
    ui->m_cFreqStep->addItem("50", QVariant(50));
    ui->m_cFreqStep->addItem("100", QVariant(100));

    int m_LowCutFreq = -120e3;
    int m_HiCutFreq = 120e3;
    int flo = -5000;
    int fhi = 5000;
    int click_res = 100;

    ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_MHZ);
    ui->freqCtrl->setDigitColor(QColor("#FFC300"));
    ui->freqCtrl->setFrequency(DEFAULT_FREQUENCY);
    connect(ui->freqCtrl, &CFreqCtrl::newFrequency, this, &MainWindow::onFreqCtrl_setFrequency);

    sdrDevice->setMode(ReceiverMode::RX, currentFrequency);
    ui->m_cFreqType->setCurrentIndex(freq_type_index);
    ui->m_cDemod->setCurrentIndex(demod_index);
    ui->freqCtrl->setFrequency(currentFrequency);
}

MainWindow::~MainWindow()
{   
    if (sdrDevice) {
        delete sdrDevice;
        sdrDevice = nullptr;
    }    
    delete ui;
}


void MainWindow::onFreqCtrl_setFrequency(qint64 freq)
{
    sdrDevice->setFrequency(freq);
    currentFrequency = sdrDevice->getCenterFrequency();
    saveSettings();
}

void MainWindow::infoFrequency(int freq)
{
    ui->freqCtrl->setFrequency(freq);
}


void MainWindow::on_pushToggleSdr_clicked()
{
    if(ui->pushToggleSdr->text() == "Start")
    {
        sdrDevice->start();
        m_stop = false;
        ui->pushToggleSdr->setText("Stop");
    }
    else
    {
        sdrDevice->stop();
        m_stop = true;
        ui->pushToggleSdr->setText("Start");
    }
}


void MainWindow::on_pushExit_clicked()
{
    sdrDevice->stop();
    delete sdrDevice;
    exit(0);
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
    ui->freqCtrl->setFrequency(currentFrequency);
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
    ui->freqCtrl->setFrequency(currentFrequency);
    saveSettings();
}


void MainWindow::on_m_cFreqType_currentIndexChanged(int index)
{
    if (index >= 0 && index < ui->m_cFreqType->count())
    {
        currentFreqMod = static_cast<FreqMod>(index);
        freq_type_index = index;
        switch (index) {
        case 0:
            ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_HZ);
            break;
        case 1:
            ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_KHZ);
            break;
        case 2:
            ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_MHZ);
            break;
        case 3:
            ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_GHZ);
            break;
        default:
            // Handle invalid index
            break;
        }
        ui->freqCtrl->setFrequency(currentFrequency);
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
}

void MainWindow::saveSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.setValue("freq_type_index", QString::number(freq_type_index));
    settings.setValue("demod_index", QString::number(demod_index));
    settings.setValue("current_frequency", QString::number(currentFrequency));
}

void MainWindow::on_m_pBPtt_clicked()
{
    if (ui->m_pBPtt->text() == "RX")
    {
        ui->m_pBPtt->setText("TX");
        m_ptt = false;
        sdrDevice->setMode(ReceiverMode::TX, currentFrequency);
    }
    else
    {
        ui->m_pBPtt->setText("RX");
        m_ptt = true;
        sdrDevice->setMode(ReceiverMode::RX, currentFrequency);
    }
}

void resampleData(const std::vector<float>& in, std::vector<float>& out, int targetSize)
{
    int originalSize = in.size();
    out.resize(targetSize);

    for (int i = 0; i < targetSize; ++i)
    {
        float index = static_cast<float>(i) * (originalSize - 1) / (targetSize - 1);
        int lowIndex = static_cast<int>(std::floor(index));
        int highIndex = static_cast<int>(std::ceil(index));
        float weight = index - lowIndex;

        if (lowIndex == highIndex)
        {
            out[i] = in[lowIndex];
        }
        else
        {
            out[i] = (1 - weight) * in[lowIndex] + weight * in[highIndex];
        }
    }
}

void MainWindow::getRxBuffer(const float *in, int size)
{
    const unsigned int fftsize = static_cast<unsigned int>(size);

    if (fftsize == 0)
    {
        return;
    }

    d_iqFftData.resize(fftsize);

    for (unsigned int i = 0; i < fftsize; ++i)
    {
        d_iqFftData[i] = in[i];
    }

    float pwr;
    float pwr_scale = static_cast<float>(1.0 / fftsize);
    double fullScalePower = 1.0;
    double sum_signal_level = 0;
    int num_iterations = 0;

    for (unsigned int i = 0; i < fftsize; i++)
    {
        pwr = pwr_scale * (d_iqFftData[i] * d_iqFftData[i]);
        double fft_signal = 20 * std::log10(pwr / fftsize / fullScalePower);
        auto level = 10 * std::log10(pwr + 1.0e-20f / fullScalePower);
        sum_signal_level += level;
        num_iterations++;
    }

    auto signal_level = sum_signal_level / num_iterations;
    ui->sMeter->setLevel(signal_level);
}
