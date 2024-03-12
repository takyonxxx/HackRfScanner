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
    void on_pushToggleSdr_clicked();
    void on_pushExit_clicked();

private:
    SdrDevice *sdrDevice{};
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
