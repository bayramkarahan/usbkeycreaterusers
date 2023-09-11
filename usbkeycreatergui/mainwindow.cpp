#include "mainwindow.h"
#include<QStorageInfo>
#include<QDebug>
#include<QComboBox>
#include<QPushButton>
#include<QCryptographicHash>
#include <libudev.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include<QApplication>
#include<QDesktopWidget>
//sudo apt-get install libudev-dev

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

{
       /***********************************************************/
     QRect screenGeometry = QApplication::desktop()->screenGeometry();
      boy=screenGeometry.height()/100;

    setFixedWidth(boy*80);
    setFixedHeight(boy*30);
    setWindowTitle("Usb Anahtar Oluşturucu");

    int x = (screenGeometry.width()/2 - this->width()/2);
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
    this->setStyleSheet("background-color: #dfdfdf;");

    /***********************************************************/
    //this->setFixedSize(300,150);
    QWidget *widget=new QWidget(this);
    widget->setFixedSize(this->width(),this->height());

    QComboBox * disk = new QComboBox(widget);
    disk->addItems(diskList());
    disk->setFixedSize(boy*45,boy*3.9);

    username = new QLineEdit(widget);
    username->setFixedSize(boy*33,boy*3.9);
    username->setText("");

    QLabel *labelusername=new QLabel(widget);
    labelusername->setText("Kullanıcı Adı");
    labelusername->setFixedSize(boy*35,boy*3.9);
    QLabel *labelusername1=new QLabel(widget);
    labelusername1->setText("Tüm Tahtalar Tek Kullanıcı Oluşturarak Kullanılabilir.Örn: ebaqr");
    labelusername1->setFixedSize(boy*78,boy*7);

    QPushButton *refreshButton=new QPushButton(widget);
    refreshButton->setText("Yenile");
    refreshButton->setFixedSize(boy*33,boy*3.9);

    connect(refreshButton, &QPushButton::clicked, [=]() {
        disk->clear();
     disk->addItems(diskList());
        //QByteArray hashData = QCryptographicHash::hash("example", QCryptographicHash::Md5);
        //qDebug() << hashData.toHex();
    });
    QLabel *baslik=new QLabel(widget);
    baslik->setText("  Disk Listesi");
    baslik->setFixedSize(boy*78,boy*3.9);

   durum=new QLabel(widget);
   durum->setFixedSize(boy*78,boy*3.9);

    QPushButton *keycreatButton=new QPushButton(widget);
    keycreatButton->setFixedSize(boy*78,boy*3.9);

    keycreatButton->setText("Anahtar Oluştur");
    connect(keycreatButton, &QPushButton::clicked, [=]() {
        if(disk->currentText()=="/"||disk->currentText()=="") {
            durum->setText("Usb Disk Seçiniz!");
            return;
        }
        if(username->text()=="") {
            durum->setText("Kullanıcı Adı Giriniz!");
            return;
        }
        keyCreateToDisk(disk->currentText(),username->text());
        //udevList();
    });



    mainlayout = new QGridLayout(widget);
    mainlayout->setAlignment(Qt::AlignCenter);
    // mainlayout->setRowStretch(2,1);
    // mainlayout->setColumnStretch(2,1);
   // mainlayout->setContentsMargins(0,2,0,2);
    mainlayout->setMargin(3);
    mainlayout->setSpacing(0);

    mainlayout->addWidget(baslik, 4,0,1,2,Qt::AlignCenter);
    mainlayout->addWidget(disk,5,0,1,1,Qt::AlignLeft);
    mainlayout->addWidget(refreshButton, 5,1,1,1,Qt::AlignLeft);
    mainlayout->addWidget(labelusername,6,0,1,1,Qt::AlignLeft);
    mainlayout->addWidget(username, 6,1,1,1,Qt::AlignLeft);

    mainlayout->addWidget(labelusername1, 8,0,1,2,Qt::AlignLeft);
    mainlayout->addWidget(keycreatButton, 9,0,1,2,Qt::AlignCenter);
    mainlayout->addWidget(durum, 10,0,1,2,Qt::AlignCenter);


}
int MainWindow::udevList()
{

    return 0;

}
void MainWindow::keyCreateToDisk(QString diskname,QString username)
{
    QStringList dlist;
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
            if (storage.isValid() && storage.isReady()) {
                if (!storage.isReadOnly()) {
                   if(storage.displayName()==diskname)
                   {
                       ///sbin/blkid -o udev /dev/sdb2|grep ID_FS_PARTUUID=
                       //udevadm info --query=all -n /dev/sdc|grep ID_SERIAL_SHORT
                       QString tempdisk=storage.device();
                       tempdisk.chop(1);
                       QString komut="udevadm info --query=all -n  "+tempdisk+"|grep ID_SERIAL_SHORT=";

                       //QString komut="/sbin/blkid -o udev "+storage.device()+"|grep ID_FS_PARTUUID=";
                      // qDebug()<<"komut"<<komut;

                       QStringList arguments;
                       arguments << "-c" << komut;
                       QString result;
                       QProcess process;
                       process.start("/bin/bash",arguments);
                       if(process.waitForFinished())
                       {
                           result = process.readAll();
                       }
                       result.chop(1);
                       result=result.split("=")[1];

                      // qDebug()<<"Sonuç:"<<result;

                      // qDebug()<<"seçilen disk"<<diskname<<storage.device()<<storage.rootPath();
                    QString pass=result.left(6);
                    QByteArray hashData = QCryptographicHash::hash(result.toStdString().c_str(), QCryptographicHash::Md5);
                    QByteArray hashusername = QCryptographicHash::hash(username.toStdString().c_str(), QCryptographicHash::Md5);
                    QByteArray hashpass = QCryptographicHash::hash(pass.toStdString().c_str(), QCryptographicHash::Md5);

                    ///qDebug() << hashData.toHex();
                        QString dosya=storage.rootPath()+"/.ebaqr";
                        QFile file(dosya.toStdString().c_str());
                          file.open(QIODevice::WriteOnly | QIODevice::Text);
                           QTextStream out(&file);
                           out << hashData.toHex()+"|"+username+"|"+hashusername.toHex()+"\n";

                           // optional, as QFile destructor will already do it:
                           file.close();

                             durum->setText("Disk Hazırlandı. Parolanız: "+hashpass.toHex().left(6));

                   }
                }
            }
        }
}
QStringList MainWindow::diskList()
{
    QStringList dlist;
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
            if (storage.isValid() && storage.isReady()) {
                if (!storage.isReadOnly()) {
                    ///sys/block/sdb/removable
                    QString disk=storage.device();
                    if(disk.contains("/dev/sd"))
                    {
                        disk=disk.right(4);
                        if(removableDisk(disk))
                        {
                           // qDebug()<<disk<<storage.displayName();
                            dlist<<storage.displayName();
                        }
                    }



                }
            }
        }
    return dlist;
}
bool  MainWindow::removableDisk(QString disk)
{
    bool diskState;
    disk.chop(1);
        QString filename="/sys/block/"+disk+"/removable";
        //qDebug()<<"liste:"<<filename;
        if(QFile::exists(filename))
        {
            const int size = 256;
            FILE* fp = fopen(filename.toStdString().c_str(), "r");
            if(fp == NULL)
            {
                perror("Error opening sys/block/sd/removable");
            }

            char line[size];
            //fgets(line, size, fp);    // Skip the first line, which consists of column headers.
            while(fgets(line, size, fp))
            {
                //qDebug()<<"satir: "<<line;
                QString satir=line;
                satir.chop(1);
             //   tempsessionlist<<satir;

                if(satir.contains("1")){
                   // qDebug()<<"satir: "<<disk<<satir;
                diskState=true;
                }
            }

            fclose(fp);
        }

        return diskState;
}
MainWindow::~MainWindow()
{

}

