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
    , foundFolders(new QStandardItemModel)
{
    // Status bar setup
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(statusText);
    ui->listView->setModel(foundFolders);

    basePath = QDir::homePath();

    // Signals and slots for this class
    QObject::connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::getBaseDir);
    QObject::connect(this, &MainWindow::searchNextFolder, this, &MainWindow::searchFolder);
    QObject::connect(testTimer, &QTimer::timeout, this, &MainWindow::searchFolder);

    testTimer->setSingleShot(false);
    testTimer->setInterval(1);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::getBaseDir()
{
    testTimer->stop();
    QString result = QFileDialog::getExistingDirectory(
        nullptr,
        "Choose a starting directory for git project search",
        basePath
        );

    if (result.isEmpty()) return;

    basePath = result;
    folderQueue->enqueue(result);
    foundFolders->clear();
    testTimer->start();
}


void MainWindow::searchFolder()
{
    if (folderQueue->isEmpty()) {
        testTimer->stop();
        return;
    }

    QString currentFolder = folderQueue->dequeue();
    qDebug() << "Scan folder: " << currentFolder;

    foundFolders->appendRow(new QStandardItem(currentFolder));

    QDir workingFolder(currentFolder);
    workingFolder.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);

    foreach (const QFileInfo &folder, workingFolder.entryInfoList()){
        folderQueue->enqueue(folder.absoluteFilePath());
    }
}
