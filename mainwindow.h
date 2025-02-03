#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QListWidgetItem>
#include <QTableWidgetItem>

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

public slots:
    void showMainMenu();
    void loadWorkersTable();
    void TableWidgetRowClicked(int row);
    void loadWorkerDetails(int workerId);
    void selectDiplom();
    void selectPasport();
    void selectPhoto();
    void showAddNewWorkerMenu();
    void addNewWorker();
    void loadPosts();
    void loadDepartaments();
    void loadLanguages();
    void loadGenders();
    void loadEduTypes();
    void addLanguage();
    void editLanguage(QListWidgetItem *item);
    void editEducation(QTableWidgetItem *item);
    void loadEducation(int row);
    void addEducation();
    void clear();
    void fillDepartamentsTable();
    void fillEduTypesTable();
    void fillGendersTable();
    void clearRowsInTable(QTableWidget*);
    void toChange(int workerId);
    void loadWorkerData(int workerId);
    void loadWorkerEducations(int workerId);
    void loadWorkerLanguages(int workerId);
    void updateWorkerData(int workerId);
    void updatePassportData(int workerId);
    void updateWorkerEducations(int workerId);
    void updateWorkerLanguages(int workerId);
    void deleteWorker(int workerId);

private:
    Ui::MainWindow *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_kalugina");
    QPixmap getImageFromDB(QByteArray);
    QList<QString> getSelectedIDDepartaments();
    QList<QString> getSelectedIDEduTypes();
    QList<QString> getSelectedIDGenders();
    void filterTable();
};
#endif // MAINWINDOW_H
