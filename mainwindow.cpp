#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QString>
#include <QDesktopServices>
#include <QUrl>


MainWindow::MainWindow(QWidget *parent, QString baseDir)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , statusText(new QLabel)
    , folderQueue(new QQueue<QString>)
    , testTimer(new QTimer)
    , foundFolders(new QStandardItemModel)
{
    // UI Setup
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(statusText);
    ui->listView->setModel(foundFolders);
    setWindowTitle("Find local git repositories");

    // The list of files/folders I expect to see in a git project
    gitDirSignature = {"HEAD", "config", "description", "hooks", "objects", "refs"};

    // Signals and slots for this class
    QObject::connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::getBaseDir);
    QObject::connect(this, &MainWindow::searchNextFolder, this, &MainWindow::searchFolder);
    QObject::connect(testTimer, &QTimer::timeout, this, &MainWindow::searchFolder);
    QObject::connect(ui->listView, &QListView::doubleClicked, this, &MainWindow::folderOpenHandler);

    // Setup looping timer that calls the crawler as fast as possible (cludge; should be worker thread)
    testTimer->setSingleShot(false);
    testTimer->setInterval(0);

    if (!baseDir.isEmpty()) {
        QDir testBaseDir(baseDir);

        if (testBaseDir.exists()){
            basePath = baseDir;
            folderQueue->enqueue(baseDir);
            testTimer->start();
            return;
        }
    }

    // Default to home dir for the file dialog
    basePath = QDir::homePath();

    // Opening the folder selection dialog box at startup to save the user (me) time
    ui->pushButton->click();
}

MainWindow::~MainWindow()
{
    delete ui;
}


// Presents the user with a folder selector dialog box and if a folder is
// chosen, adds the folder path the the queue and starts the looping timer
// that calls the searchFolder crawler.
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


// Crawler that recurses through folder tree, adding subfolders to it's queue
// and adding folders matching git projects to the output list.
// This slot is currently called on a loop by a timer to get around crashing
// issue; When the queue this function reads from is empty it stops the timer.
void MainWindow::searchFolder()
{
    if (folderQueue->isEmpty()) {
        statusText->setText("Search complete");
        testTimer->stop();
        return;
    }

    QString currentFolder = folderQueue->dequeue();
    ui->statusbar->showMessage(currentFolder, 1000);

    QDir workingFolder(currentFolder);
    workingFolder.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);

    foreach (const QFileInfo &folder, workingFolder.entryInfoList()){
        folderQueue->enqueue(folder.absoluteFilePath());
    }

    if (!isGitDir(currentFolder)) return;

    foundFolders->appendRow(new QStandardItem(currentFolder));
    foundFolders->sort(0);
}

// Slot for passing selected item to folder opening function
void MainWindow::folderOpenHandler(const QModelIndex &index)
{
    QString clickedItemText = index.data().toString();
    openFolder(clickedItemText);
}

// Open the folder URL in the system default file navigator
void MainWindow::openFolder(QString folder)
{
    QUrl folderUrl = QUrl::fromLocalFile(folder);
    QDesktopServices::openUrl(folderUrl);
}

// Checks if the folder passed in is a git project or headless git project
bool MainWindow::isGitDir(QString folder)
{
    bool headless = true;
    bool standard = true;

    // Is folder a headless git project
    QDir currentDir(folder);
    if (!currentDir.exists()) return false;

    foreach (const QString &piece, gitDirSignature) {
        if (!currentDir.exists(piece)) headless = false;
    }

    // Is folder a regular git project
    currentDir.cd(".git");
    if (currentDir.exists()) {
        foreach (const QString &piece, gitDirSignature) {
            if (!currentDir.exists(piece)) standard = false;
        }
    } else {
        standard = false;
    }

    return headless | standard;
}
