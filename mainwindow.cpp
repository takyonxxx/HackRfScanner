#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>
#include <vector>
#include <complex>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)    
    ,  sampleRate(DEFAULT_SAMPLE_RATE)
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
    d_fftAvg = static_cast<float>(1.0 - 1.0e-2 * 75);

    ui->m_cDemod->setCurrentIndex(demod_index);

    QString homePath = QDir::homePath();
    m_sSettingsFile = homePath + "/settings.ini";

    sdrDevice = new SdrDevice(this);
    connect(sdrDevice, &SdrDevice::infoFrequency, this, &MainWindow::infoFrequency);

    if (QFile(m_sSettingsFile).exists())
        loadSettings();
    else
        saveSettings();

    ui->m_cFreqStep->addItem("1", QVariant(1));
    ui->m_cFreqStep->addItem("5", QVariant(5));
    ui->m_cFreqStep->addItem("10", QVariant(10));
    ui->m_cFreqStep->addItem("50", QVariant(50));
    ui->m_cFreqStep->addItem("100", QVariant(100));


    cPlotter = new CPlotter(this);

    cPlotter->setTooltipsEnabled(true);

    cPlotter->setSampleRate(DEFAULT_SAMPLE_RATE);
    cPlotter->setSpanFreq(static_cast<quint32>(DEFAULT_SAMPLE_RATE));
    cPlotter->setCenterFreq(static_cast<quint64>(currentFrequency));

    cPlotter->setFftRange(-140.0f, 20.0f);
    cPlotter->setFftRate(fftrate);
    cPlotter->setPandapterRange(-140.f, 20.f);
    cPlotter->setHiLowCutFrequencies(m_LowCutFreq, m_HiCutFreq);
    cPlotter->setDemodRanges(m_LowCutFreq, -_KHZ(5), _KHZ(5),m_HiCutFreq, true);

    cPlotter->setFreqUnits(1000);
    cPlotter->setPercent2DScreen(50);
    cPlotter->setFilterBoxEnabled(true);
    cPlotter->setCenterLineEnabled(true);
    cPlotter->setClickResolution(1);

    cPlotter->setFftPlotColor(QColor("#CEECF5"));
    cPlotter->setFreqStep(_KHZ(5));

    //cPlotter->setPeakDetection(true ,2);
    cPlotter->setFftFill(true);

    connect(cPlotter, &CPlotter::newDemodFreq, this, &MainWindow::on_plotter_newDemodFreq);
    connect(cPlotter, &CPlotter::newFilterFreq, this, &MainWindow::on_plotter_newFilterFreq);

//    fftPlotter = new FFTPlotter(this);
//    fftPlotter->setSampleRate(DEFAULT_SAMPLE_RATE);
//    fftPlotter->setCenterFrequency(currentFrequency);

    ui->plotterLayout->addWidget(cPlotter);

    ui->freqCtrl->setup(0, 0, 6000e6, 1, FCTL_UNIT_MHZ);
    ui->freqCtrl->setDigitColor(QColor("#FFC300"));
    ui->freqCtrl->setFrequency(DEFAULT_FREQUENCY);
    connect(ui->freqCtrl, &CFreqCtrl::newFrequency, this, &MainWindow::onFreqCtrl_setFrequency);

    sdrDevice->setMode(ReceiverMode::RX, currentFrequency);
    ui->m_cFreqType->setCurrentIndex(freq_type_index);
    ui->m_cDemod->setCurrentIndex(demod_index);
    ui->freqCtrl->setFrequency(currentFrequency);

//    QTimer* timer = new QTimer(this);
//    connect(timer, &QTimer::timeout, this, &MainWindow::fetchFFtData);
//    int interval = 100;
//    timer->setInterval(interval);
//    timer->start();
}

MainWindow::~MainWindow()
{   
    if (sdrDevice) {
        delete sdrDevice;
        sdrDevice = nullptr;
    }    
    delete ui;
}


/* CPlotter::NewfilterFreq() is emitted or bookmark activated */
void MainWindow::on_plotter_newFilterFreq(int low, int high)
{
    m_LowCutFreq = low;
    m_HiCutFreq = high;
}

void MainWindow::on_plotter_newDemodFreq(qint64 freq, qint64 delta)
{
    Q_UNUSED(delta)
    sdrDevice->setFrequency(freq);
    currentFrequency = freq;
    saveSettings();
}

void MainWindow::onFreqCtrl_setFrequency(qint64 freq)
{
    sdrDevice->setFrequency(freq);
    currentFrequency = sdrDevice->getCenterFrequency();
    cPlotter->setCenterFreq(static_cast<quint64>(currentFrequency));
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
    sampleRate = settings.value("sample_rate", "").toString().toInt();
}

void MainWindow::saveSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.setValue("freq_type_index", QString::number(freq_type_index));
    settings.setValue("demod_index", QString::number(demod_index));
    settings.setValue("current_frequency", QString::number(currentFrequency));
    settings.setValue("sample_rate", QString::number(sampleRate));
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
    if (size == 0) {
        return;
    }

    int fft_size = size / 2;  // Size for FFT
    std::vector<float> fft_output(fft_size);
    float sum_signal_power = 0.0f;
    int num_iterations = 0;

    int chunk_size = sampleRate;

    if (size < chunk_size) {
        chunk_size = size;
    }

    // Calculate the number of chunks in the input data
    int num_chunks = size / chunk_size;

    // Process each chunk separately
    for (int i = 0; i < num_chunks; ++i) {
        const float *chunk_start = in + i * chunk_size;
        // Process the buffer to compute FFT and power spectrum
        processRxBuffer(chunk_start, chunk_size, fft_output.data(), sum_signal_power, num_iterations);
        // Pass the data arrays to the plotter
        cPlotter->setNewFttData(fft_output.data(), fft_output.data(), chunk_size / 2); // Assuming FFT size is half of the chunk size
        auto signal_level_dB = 10 * log10(sum_signal_power / num_iterations);
        ui->sMeter->setLevel(-1 * (100 - signal_level_dB));
    }

    // Process the remaining data (if any)
    int remaining_size = size % chunk_size;
    if (remaining_size > 0) {
        const float *remaining_chunk_start = in + num_chunks * chunk_size;
        processRxBuffer(remaining_chunk_start, remaining_size, fft_output.data(), sum_signal_power, num_iterations);
        cPlotter->setNewFttData(fft_output.data(), fft_output.data(), remaining_size / 2);
        auto signal_level_dB = 10 * log10(sum_signal_power / num_iterations);
        ui->sMeter->setLevel(-1 * (100 - signal_level_dB));
    }
}


void MainWindow::fetchFFtData()
{
    auto length = DEFAULT_SAMPLE_RATE;
    std::vector<float> data(length);
}
