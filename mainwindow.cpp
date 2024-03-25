#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QString>
#include <QDesktopServices>
#include <QUrl>


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
    setWindowTitle("Find local git repositories");

    basePath = QDir::homePath();
    gitDirSignature = {"HEAD", "config", "description", "hooks", "objects", "refs"};

    // Signals and slots for this class
    QObject::connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::getBaseDir);
    QObject::connect(this, &MainWindow::searchNextFolder, this, &MainWindow::searchFolder);
    QObject::connect(testTimer, &QTimer::timeout, this, &MainWindow::searchFolder);
    QObject::connect(ui->listView, &QListView::doubleClicked, this, &MainWindow::folderOpenHandler);

    testTimer->setSingleShot(false);
    testTimer->setInterval(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::getBaseDir()
{
    testTimer->stop();
    statusText->setText("");
    folderQueue->clear();
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
        statusText->setText("Search complete");
        testTimer->stop();
        return;
    }

    QString currentFolder = folderQueue->dequeue();
    qDebug() << "Scan folder: " << currentFolder;
    ui->statusbar->showMessage(currentFolder, 1000);

    QDir workingFolder(currentFolder);
    workingFolder.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);

    foreach (const QFileInfo &folder, workingFolder.entryInfoList()){
        folderQueue->enqueue(folder.absoluteFilePath());
    }

    if (!isGitDir(currentFolder)) return;

    foundFolders->appendRow(new QStandardItem(currentFolder));
}

void MainWindow::folderOpenHandler(const QModelIndex &index)
{
    QString clickedItemText = index.data().toString();
    qDebug() << "Double-clicked item:" << clickedItemText;
    openFolder(clickedItemText);
}

void MainWindow::openFolder(QString folder)
{
    QUrl folderUrl = QUrl::fromLocalFile(folder);
    QDesktopServices::openUrl(folderUrl);
}

bool MainWindow::isGitDir(QString folder)
{
    bool headless = true;
    bool standard = true;

    QDir currentDir(folder);
    if (!currentDir.exists()) return false;

    foreach (const QString &piece, gitDirSignature) {
        if (!currentDir.exists(piece)) standard = false;
    }

    currentDir.cd(".git");
    if (currentDir.exists()) {
        foreach (const QString &piece, gitDirSignature) {
            if (!currentDir.exists(piece)) headless = false;
        }
    } else {
        standard = false;
    }

    return headless | standard;
}
