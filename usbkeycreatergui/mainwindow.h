#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QGridLayout>
#include<QProcess>
#include<QLabel>
#include <libudev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <cctype>
#include <unistd.h>
#include<QLineEdit>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    bool  removableDisk(QString disk);
    QStringList diskList();
    void keyCreateToDisk(QString diskname, QString username);
    int udevList();
private:
       QGridLayout *mainlayout;
        QLabel *durum;
int boy;
QLineEdit * username;
};
#endif // MAINWINDOW_H
