#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QQueue>
#include <QTimer>
#include <QStandardItemModel>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, QString baseDir = "");
    ~MainWindow();
    void openFolder(QString);
    bool isGitDir(QString);

private:
    Ui::MainWindow *ui;
    QLabel *statusText;
    QString basePath;
    QQueue<QString> *folderQueue;
    QTimer *testTimer;
    QStandardItemModel *foundFolders;
    QVector<QString> gitDirSignature;

signals:
    void searchNextFolder();

public slots:
    void getBaseDir();
    void searchFolder();
    void folderOpenHandler(const QModelIndex&);
};
#endif // MAINWINDOW_H
