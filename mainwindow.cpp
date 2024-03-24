#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QString>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , statusText(new QLabel)
    , folderQueue(new QQueue<QString>)
    , testTimer(new QTimer)
{
    // Status bar setup
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(statusText);

    basePath = QDir::homePath();

    // Signals and slots for this class
    QObject::connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::getBaseDir);
    QObject::connect(this, &MainWindow::searchNextFolder, this, &MainWindow::searchFolder);
    QObject::connect(testTimer, &QTimer::timeout, this, &MainWindow::searchFolder);

    testTimer->setSingleShot(true);
    testTimer->setInterval(1);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::getBaseDir()
{
    QString result = QFileDialog::getExistingDirectory(
        nullptr,
        "Choose a starting directory for git project search",
        basePath
        );

    if (result.isEmpty()) return;

    basePath = result;
    folderQueue->enqueue(result);
    testTimer->start();
}


void MainWindow::searchFolder()
{
    if (folderQueue->isEmpty()) return;

    QString currentFolder = folderQueue->dequeue();
    qDebug() << "Scan folder: " << currentFolder;

    QDir workingFolder(currentFolder);
    workingFolder.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);

    foreach (const QFileInfo &folder, workingFolder.entryInfoList()){
        folderQueue->enqueue(folder.absoluteFilePath());
    }

    testTimer->start();
}
