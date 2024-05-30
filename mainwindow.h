#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCoreApplication>
#include <QApplication>
#include <QDebug>
#include "sdrdevice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void infoFrequency(int f);
    void loadSettings();
    void saveSettings();

    void on_pushToggleSdr_clicked();
    void on_pushExit_clicked();
    void on_m_pBSetFreq_clicked();
    void on_m_pIncFreq_clicked();
    void on_m_pDecFreq_clicked();
    void on_m_cFreqType_currentIndexChanged(int index);
    void on_m_cDemod_currentIndexChanged(int index);    
    void on_m_pBPtt_clicked();

private:
    SdrDevice *sdrDevice{};
    FreqMod currentFreqMod;
    Demod currentDemod;
    double currentFrequency;
    bool m_ptt;

    QString m_sSettingsFile;
    int freq_type_index;
    int demod_index;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
