#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QQueue>
#include <QTimer>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel *statusText;
    QString basePath;
    QQueue<QString> *folderQueue;
    QTimer *testTimer;
    QStandardItemModel *foundFolders;

signals:
    void searchNextFolder();

public slots:
    void getBaseDir();
    void searchFolder();
};
#endif // MAINWINDOW_H
