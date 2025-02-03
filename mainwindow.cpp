#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include <QPixmap>
#include <QImage>
#include <QBuffer>
#include <QFileDialog>
#include <QSqlError>
#include <QDebug>
#include <QCheckBox>
#include <QRegularExpressionValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->tw_workers, &QTableWidget::cellClicked, this, &MainWindow::TableWidgetRowClicked);
    connect(ui->qpb_toMain,&QPushButton::clicked,this,&MainWindow::showMainMenu);
    connect(ui->qpb_addCancel,&QPushButton::clicked,this,&MainWindow::showMainMenu);
    connect(ui->qpb_addNewWorker,&QPushButton::clicked,this,&MainWindow::showAddNewWorkerMenu);
    connect(ui->qpb_addLanguage,&QPushButton::clicked,this,&MainWindow::addLanguage);
    connect(ui->lw_language, &QListWidget::itemClicked, this, &MainWindow::editLanguage);
    connect(ui->tw_addEducation, &QTableWidget::itemClicked, this, &MainWindow::editEducation);
    connect(ui->tw_education, &QTableWidget::cellClicked, this, &MainWindow::loadEducation);
    connect(ui->qpb_addEdu,&QPushButton::clicked,this,&MainWindow::addEducation);
    connect(ui->lb_AddEduDoc,&QPushLabel::clicked,this,&MainWindow::selectDiplom);
    connect(ui->lb_addPasDoc,&QPushLabel::clicked,this,&MainWindow::selectPasport);
    connect(ui->lb_addPhoto,&QPushLabel::clicked,this,&MainWindow::selectPhoto);
    connect(ui->qpb_clear,&QPushButton::clicked,this,&MainWindow::clear);
    connect(ui->qpb_addWorker,&QPushButton::clicked,this,&MainWindow::addNewWorker);

    ui->tw_workers->setColumnWidth(0,98);
    ui->tw_workers->setColumnWidth(1,300);
    ui->tw_workers->setColumnWidth(2,300);
    ui->tw_workers->setColumnWidth(3,300);

    ui->tw_education->setColumnWidth(0,844);
    ui->tw_education->setColumnWidth(1,394);

    ui->tw_addEducation->setColumnWidth(0,844);
    ui->tw_addEducation->setColumnWidth(1,394);

    QRegularExpression phoneRegex("^\\d{11}$");
    QRegularExpressionValidator *phoneValidator = new QRegularExpressionValidator(phoneRegex, this);
    ui->le_addPhone->setValidator(phoneValidator);

    QRegularExpression textRegex("^[А-Яа-яЁё\\s]+$");
    QRegularExpressionValidator *textValidator = new QRegularExpressionValidator(textRegex, this);
    ui->le_addName->setValidator(textValidator);
    ui->le_addSurname->setValidator(textValidator);
    ui->le_addPatronymic->setValidator(textValidator);
    ui->le_addProfession->setValidator(textValidator);
    ui->le_addQualification->setValidator(textValidator);

    QRegularExpression seriaRegex("^\\d{4}$");
    QRegularExpressionValidator *seriaValidator = new QRegularExpressionValidator(seriaRegex, this);
    ui->le_addSeria->setValidator(seriaValidator);

    QRegularExpression numberRegex("^\\d{6}$");
    QRegularExpressionValidator *numberValidator = new QRegularExpressionValidator(numberRegex, this);
    ui->le_addNumber->setValidator(numberValidator);

    showMainMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Показать главное меню
void MainWindow::showMainMenu()
{
    ui->qpb_update->setVisible(true);
    ui->qpb_addWorker->setVisible(true);
    ui->frame_main->setGeometry(ui->frame_main->x(), 0, ui->frame_main->width(), ui->frame_main->height());
    ui->frame_worker->setGeometry(ui->frame_worker->x(), 1000, ui->frame_worker->width(), ui->frame_worker->height());
    ui->frame_addNewWorker->setGeometry(ui->frame_addNewWorker->x(), 1000, ui->frame_addNewWorker->width(), ui->frame_addNewWorker->height());
    ui->frame_main->setEnabled(true);
    ui->frame_worker->setEnabled(false);
    ui->frame_addNewWorker->setEnabled(false);
    ui->lb_profession->clear();
    ui->lb_qualification->clear();
    ui->lb_eduPlace->clear();
    ui->lb_eduDate->clear();
    ui->lb_eduDoc->clear();
    ui->tabWidget->setCurrentIndex(0);
    loadWorkersTable();

    fillDepartamentsTable();
    fillEduTypesTable();
    fillGendersTable();
}

//Загрузить таблицу сотрудников
void MainWindow::loadWorkersTable()
{
    db = QSqlDatabase::database("db_kalugina");
    if (getDBConnection(db))
    {
        int row = 0;
        QSqlQuery query(db);

        query.prepare(
            "SELECT w.id, p.name, p.surname, p.patronymic, d.name AS department_name, po.name AS post_name "
            "FROM workers w "
            "INNER JOIN pasports p ON w.id = p.worker_id "
            "INNER JOIN departaments d ON w.departament_id = d.id "
            "INNER JOIN posts po ON w.post_id = po.id"
            );

        if (query.exec())
        {
            ui->tw_workers->setRowCount(0);

            while (query.next())
            {
                ui->tw_workers->insertRow(row);

                int workerId = query.value("id").toInt();
                QString name = query.value("name").toString();
                QString surname = query.value("surname").toString();
                QString patronymic = query.value("patronymic").toString();
                QString fullName = surname + " " + name + " " + patronymic;
                QString departmentName = query.value("department_name").toString();
                QString postName = query.value("post_name").toString();

                QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(workerId));
                QTableWidgetItem *fullNameItem = new QTableWidgetItem(fullName);
                QTableWidgetItem *departmentItem = new QTableWidgetItem(departmentName);
                QTableWidgetItem *postItem = new QTableWidgetItem(postName);

                ui->tw_workers->setItem(row, 0, idItem);
                ui->tw_workers->setItem(row, 1, fullNameItem);
                ui->tw_workers->setItem(row, 2, postItem);
                ui->tw_workers->setItem(row, 3, departmentItem);

                row++;
            }
        }
    }
}

//Клик на таблицу
void MainWindow::TableWidgetRowClicked(int row)
{
    QTableWidgetItem *item = ui->tw_workers->item(row, 0);
    if (item)
    {
        int orderId = item->text().toInt();
        loadWorkerDetails(orderId);
    }
}

//Загрузка данных сотрудника
void MainWindow::loadWorkerDetails(int workerId)
{
    ui->frame_main->setGeometry(ui->frame_main->x(), 1000, ui->frame_main->width(), ui->frame_main->height());
    ui->frame_worker->setGeometry(ui->frame_worker->x(), 0, ui->frame_worker->width(), ui->frame_worker->height());
    ui->frame_main->setEnabled(false);
    ui->frame_worker->setEnabled(true);

    db = QSqlDatabase::database("db_kalugina");
    if (getDBConnection(db))
    {
        QSqlQuery query(db);

        query.prepare(R"(
            SELECT name, surname, patronymic, gender_id, birth_date, birth_place,
                   registration_place, seria, number, issue_place, issue_date, pasport_photo
            FROM pasports
            WHERE worker_id = :workerId
        )");
        query.bindValue(":workerId", workerId);
        if (query.exec() && query.next())
        {
            QString name = query.value("name").toString();
            QString surname = query.value("surname").toString();
            QString patronymic = query.value("patronymic").toString();
            QString fullName = surname + " " + name + " " + patronymic;
            ui->lb_fio->setText(fullName);
            ui->lb_fioPas->setText(fullName);

            int genderId = query.value("gender_id").toInt();
            QSqlQuery genderQuery(db);
            genderQuery.prepare("SELECT name FROM genders WHERE id = :genderId");
            genderQuery.bindValue(":genderId", genderId);
            if (genderQuery.exec() && genderQuery.next())
            {
                QString genderName = genderQuery.value("name").toString();
                ui->lb_gender->setText(genderName);
                ui->lb_genderPas->setText(genderName);
            }

            QDate birthDate = query.value("birth_date").toDate();
            ui->lb_birthDate->setText(birthDate.toString("dd.MM.yyyy"));
            ui->lb_birthDatePas->setText(birthDate.toString("dd.MM.yyyy"));

            QString birthPlace = query.value("birth_place").toString();
            ui->lb_birthPlacePas->setText(birthPlace);

            QString registrationPlace = query.value("registration_place").toString();
            ui->lb_regPas->setText(registrationPlace);

            QString seria = query.value("seria").toString();
            ui->lb_seriaPas->setText(seria);

            QString number = query.value("number").toString();
            ui->lb_nemberPas->setText(number);

            QString issuePlace = query.value("issue_place").toString();
            ui->lb_issuePlacePas->setText(issuePlace);

            QDate issueDate = query.value("issue_date").toDate();
            ui->lb_issueDatePas->setText(issueDate.toString("dd.MM.yyyy"));

            QByteArray passportPhotoData = query.value("pasport_photo").toByteArray();
            QPixmap passportPixmap;
            if (passportPixmap.loadFromData(passportPhotoData))
            {
                ui->lb_pasDoc->setPixmap(passportPixmap.scaled(ui->lb_pasDoc->size(), Qt::KeepAspectRatio));
            }
        }

        query.prepare("SELECT phone, post_id, departament_id, photo FROM workers WHERE id = :workerId");
        query.bindValue(":workerId", workerId);
        if (query.exec() && query.next())
        {
            QString phone = query.value("phone").toString();
            ui->lb_phone->setText(phone);

            int postId = query.value("post_id").toInt();
            int departmentId = query.value("departament_id").toInt();
            QByteArray photoData = query.value("photo").toByteArray();

            QPixmap photoPixmap;
            if (photoPixmap.loadFromData(photoData))
            {
                ui->lb_photo->setPixmap(photoPixmap);
            }

            QSqlQuery postQuery(db);
            postQuery.prepare("SELECT name, salary FROM posts WHERE id = :postId");
            postQuery.bindValue(":postId", postId);
            if (postQuery.exec() && postQuery.next())
            {
                QString postName = postQuery.value("name").toString();
                double salary = postQuery.value("salary").toDouble();
                ui->lb_post->setText(postName);
                ui->lb_salary->setText(QString::number(salary, 'f', 2));
            }

            QSqlQuery departmentQuery(db);
            departmentQuery.prepare("SELECT name FROM departaments WHERE id = :departmentId");
            departmentQuery.bindValue(":departmentId", departmentId);
            if (departmentQuery.exec() && departmentQuery.next())
            {
                QString departmentName = departmentQuery.value("name").toString();
                ui->lb_departament->setText(departmentName);
            }
        }

        query.prepare("SELECT l.name FROM workers_languages wl INNER JOIN languages l ON wl.language_id = l.id WHERE wl.worker_id = :workerId");
        query.bindValue(":workerId", workerId);
        if (query.exec())
        {
            QStringList languages;
            while (query.next())
            {
                languages << query.value("name").toString();
            }
            ui->lb_languages->setText(languages.join(", "));
        }

        query.prepare(R"(
        SELECT e.profession, e.qualification, e.education_place, e.graduation_date,
        e.education_document, e.type_id, et.name AS education_type_name
        FROM workers_educations we
        INNER JOIN educations e ON we.education_id = e.id
        INNER JOIN education_types et ON e.type_id = et.id
        WHERE we.worker_id = :workerId
        )");
        query.bindValue(":workerId", workerId);

        if (query.exec())
        {
            ui->tw_education->clearContents();
            ui->tw_education->setRowCount(0);

            while (query.next())
            {
                QString profession = query.value("profession").toString();
                QString qualification = query.value("qualification").toString();
                QString educationPlace = query.value("education_place").toString();
                QDate graduationDate = query.value("graduation_date").toDate();
                QByteArray educationDocument = query.value("education_document").toByteArray();
                QString eduTypeName = query.value("education_type_name").toString();

                int row = ui->tw_education->rowCount();
                ui->tw_education->insertRow(row);

                QTableWidgetItem *professionItem = new QTableWidgetItem(profession);
                ui->tw_education->setItem(row, 0, professionItem);

                QTableWidgetItem *eduTypeItem = new QTableWidgetItem(eduTypeName);
                ui->tw_education->setItem(row, 1, eduTypeItem);

                QVariantMap additionalData;
                additionalData["qualification"] = qualification;
                additionalData["educationPlace"] = educationPlace;
                additionalData["graduationDate"] = graduationDate.toString("yyyy-MM-dd");
                additionalData["educationDocument"] = educationDocument;

                professionItem->setData(Qt::UserRole, additionalData);
            }
        }
    }
    connect(ui->qpb_change, &QPushButton::clicked, this, [this, workerId]() { toChange(workerId); });
    connect(ui->qpb_delete, &QPushButton::clicked, this, [this, workerId]() { deleteWorker(workerId); });
}

void MainWindow::loadEducation(int row)
{
    QTableWidgetItem *professionItem = ui->tw_education->item(row, 0);

    if (professionItem)
    {
        QVariantMap additionalData = professionItem->data(Qt::UserRole).toMap();

        ui->lb_profession->setText(professionItem->text());
        ui->lb_qualification->setText(additionalData["qualification"].toString());
        ui->lb_eduPlace->setText(additionalData["educationPlace"].toString());
        ui->lb_eduDate->setText(additionalData["graduationDate"].toString());

        QByteArray eduDocData = additionalData["educationDocument"].toByteArray();
        QPixmap eduDocPixmap;
        eduDocPixmap.loadFromData(eduDocData);
        if (!eduDocPixmap.isNull())
        {
            ui->lb_eduDoc->setPixmap(eduDocPixmap);
        }
    }
}

//Загрузить фото диплома
void MainWindow::selectDiplom()
{
    QStringList imageName;
    QFileDialog fd;
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::Detail);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle("Выберите документ");
    fd.setNameFilter("Image files (*.png *.jpeg *.jpg)");
    if(fd.exec())
    {
        imageName = fd.selectedFiles();
    }
    if(!imageName.isEmpty())
    {
        QPixmap image;
        image.load(imageName.at(0));
        ui->lb_AddEduDoc->setPixmap(image);
    }
}

//Загрузить фото паспорта
void MainWindow::selectPasport()
{
    QStringList imageName;
    QFileDialog fd;
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::Detail);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle("Выберите документ");
    fd.setNameFilter("Image files (*.png *.jpeg *.jpg)");
    if(fd.exec())
    {
        imageName = fd.selectedFiles();
    }
    if(!imageName.isEmpty())
    {
        QPixmap image;
        image.load(imageName.at(0));
        ui->lb_addPasDoc->setPixmap(image);
    }
}

//Загрузить фото сотрудника
void MainWindow::selectPhoto()
{
    QStringList imageName;
    QFileDialog fd;
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::Detail);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle("Выберите документ");
    fd.setNameFilter("Image files (*.png *.jpeg *.jpg)");
    if(fd.exec())
    {
        imageName = fd.selectedFiles();
    }
    if(!imageName.isEmpty())
    {
        QPixmap image;
        image.load(imageName.at(0));
        ui->lb_addPhoto->setPixmap(image);
    }
}

//Показать меню добавления нового сотрудника
void MainWindow::showAddNewWorkerMenu()
{
    ui->qpb_update->setVisible(false);
    ui->frame_main->setGeometry(ui->frame_main->x(), 1000, ui->frame_main->width(), ui->frame_main->height());
    ui->frame_addNewWorker->setGeometry(ui->frame_addNewWorker->x(), 0, ui->frame_addNewWorker->width(), ui->frame_addNewWorker->height());
    ui->frame_main->setEnabled(false);
    ui->frame_addNewWorker->setEnabled(true);
    ui->tabWidget_2->setCurrentIndex(0);
    loadPosts();
    loadDepartaments();
    loadLanguages();
    loadGenders();
    loadEduTypes();
}

//Добавление нового сотрудника
void MainWindow::addNewWorker()
{
    db = QSqlDatabase::database("db_kalugina");

    if (!getDBConnection(db))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных!");
        qDebug() << "Ошибка: соединение с базой данных не установлено.";
        return;
    }

    if (!db.isOpen())
    {
        QMessageBox::critical(this, "Ошибка", "База данных не открыта.");
        qDebug() << "Ошибка: база данных не открыта.";
        return;
    }

    if (db.driverName() != "QMYSQL")
    {
        QMessageBox::critical(this, "Ошибка", "Неверный драйвер базы данных! Используйте QMYSQL.");
        qDebug() << "Ошибка: драйвер базы данных" << db.driverName();
        return;
    }

    QString phone = ui->le_addPhone->text().trimmed();
    if (phone.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Введите номер телефона!");
        qDebug() << "Ошибка: номер телефона не указан.";
        return;
    }

    QPixmap photo = ui->lb_addPhoto->pixmap(Qt::ReturnByValue);
    if (photo.isNull())
    {
        QMessageBox::warning(this, "Ошибка", "Загрузите фото работника!");
        qDebug() << "Ошибка: фото работника не загружено.";
        return;
    }

    int postId = ui->cb_post->currentData().toInt();
    if (postId == 0)
    {
        QMessageBox::warning(this, "Ошибка", "Выберите должность!");
        qDebug() << "Ошибка: должность не выбрана.";
        return;
    }

    int departmentId = ui->cb_departament->currentData().toInt();
    if (departmentId == 0)
    {
        QMessageBox::warning(this, "Ошибка", "Выберите отделение!");
        qDebug() << "Ошибка: отделение не выбрано.";
        return;
    }

    QByteArray photoData;
    QBuffer photoBuffer(&photoData);
    if (!photoBuffer.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить фото работника в буфер.");
        qDebug() << "Ошибка: фото буфер не открылся.";
        return;
    }
    photo.save(&photoBuffer, "PNG");

    QDate hireDate = QDate::currentDate();

    QSqlQuery query(db);
    query.prepare("INSERT INTO workers (phone, photo, post_id, departament_id, hire_date) "
                  "VALUES (:phone, :photo, :post_id, :departament_id, :hire_date)");
    query.bindValue(":phone", phone);
    query.bindValue(":photo", photoData);
    query.bindValue(":post_id", postId);
    query.bindValue(":departament_id", departmentId);
    query.bindValue(":hire_date", hireDate);

    if (!query.exec())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить работника: " + query.lastError().text());
        qDebug() << "Ошибка при добавлении работника: " << query.lastError().text();
        return;
    }
    int workerId = query.lastInsertId().toInt();
    if (workerId == 0)
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось получить ID добавленного работника.");
        qDebug() << "Ошибка: ID добавленного работника равен 0.";
        return;
    }

    for (int i = 0; i < ui->lw_language->count(); ++i)
    {
        QListWidgetItem *item = ui->lw_language->item(i);
        int languageId = item->data(Qt::UserRole).toInt();

        QSqlQuery langQuery(db);
        langQuery.prepare("INSERT INTO workers_languages (worker_id, language_id) VALUES (:worker_id, :language_id)");
        langQuery.bindValue(":worker_id", workerId);
        langQuery.bindValue(":language_id", languageId);

        if (!langQuery.exec())
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось сохранить язык работника: " + langQuery.lastError().text());
            qDebug() << "Ошибка при сохранении языка: " << langQuery.lastError().text();
            return;
        }
    }

    QString name = ui->le_addName->text().trimmed();
    QString surname = ui->le_addSurname->text().trimmed();
    QString patronymic = ui->le_addPatronymic->text().trimmed();
    int genderId = ui->cb_gender->currentData().toInt();
    QDate birthDate = ui->de_addBirthDate->date();
    QString birthPlace = ui->le_addBirthPlace->text().trimmed();
    QString registrationPlace = ui->le_addPlace->text().trimmed();
    int seria = ui->le_addSeria->text().toInt();
    int number = ui->le_addNumber->text().toInt();
    QString issuePlace = ui->le_addIssuePlace->text().trimmed();
    QDate issueDate = ui->de_addIssueDate->date();

    QPixmap passportPhoto = ui->lb_addPasDoc->pixmap(Qt::ReturnByValue);
    if (passportPhoto.isNull())
    {
        QMessageBox::warning(this, "Ошибка", "Загрузите фото паспорта!");
        qDebug() << "Ошибка: фото паспорта не загружено.";
        return;
    }

    QByteArray passportPhotoData;
    QBuffer passportBuffer(&passportPhotoData);
    if (!passportBuffer.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить фото паспорта в буфер.");
        qDebug() << "Ошибка: буфер фото паспорта не открылся.";
        return;
    }
    passportPhoto.save(&passportBuffer, "PNG");

    QSqlQuery passportQuery(db);
    passportQuery.prepare(
        "INSERT INTO pasports (worker_id, name, surname, patronymic, gender_id, birth_date, birth_place, registration_place, seria, number, issue_place, issue_date, pasport_photo) "
        "VALUES (:worker_id, :name, :surname, :patronymic, :gender_id, :birth_date, :birth_place, :registration_place, :seria, :number, :issue_place, :issue_date, :pasport_photo)");
    passportQuery.bindValue(":worker_id", workerId);
    passportQuery.bindValue(":name", name);
    passportQuery.bindValue(":surname", surname);
    passportQuery.bindValue(":patronymic", patronymic);
    passportQuery.bindValue(":gender_id", genderId);
    passportQuery.bindValue(":birth_date", birthDate);
    passportQuery.bindValue(":birth_place", birthPlace);
    passportQuery.bindValue(":registration_place", registrationPlace);
    passportQuery.bindValue(":seria", seria);
    passportQuery.bindValue(":number", number);
    passportQuery.bindValue(":issue_place", issuePlace);
    passportQuery.bindValue(":issue_date", issueDate);
    passportQuery.bindValue(":pasport_photo", passportPhotoData);

    if (!passportQuery.exec())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить паспортные данные: " + passportQuery.lastError().text());
        qDebug() << "Ошибка при сохранении паспортных данных: " << passportQuery.lastError().text();
        return;
    }

    for (int i = 0; i < ui->tw_addEducation->rowCount(); ++i)
    {
        QTableWidgetItem *professionItem = ui->tw_addEducation->item(i, 0);

        QVariantMap eduData = professionItem->data(Qt::UserRole).toMap();

        int typeId = eduData["typeId"].toInt();

        QByteArray eduDocImageData = eduData["eduDocImage"].toByteArray();
        if (eduDocImageData.isEmpty())
        {
            QMessageBox::warning(this, "Ошибка", "Изображение документа не найдено.");
            return;
        }

        QSqlQuery eduQuery(db);
        eduQuery.prepare(
            "INSERT INTO educations (type_id, profession, graduation_date, education_place, education_document, qualification) "
            "VALUES (:type_id, :profession, :graduation_date, :education_place, :education_document, :qualification)");

        eduQuery.bindValue(":type_id", typeId);
        eduQuery.bindValue(":profession", professionItem->text());
        eduQuery.bindValue(":graduation_date", eduData["eduDate"].toString());
        eduQuery.bindValue(":education_place", eduData["eduPlace"].toString());
        eduQuery.bindValue(":education_document", eduDocImageData);
        eduQuery.bindValue(":qualification", eduData["qualification"].toString());

        if (!eduQuery.exec())
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось сохранить данные об образовании: " + eduQuery.lastError().text());
            return;
        }

        QSqlQuery selectQuery(db);
        selectQuery.prepare(
            "SELECT id FROM educations WHERE type_id = :type_id AND profession = :profession AND graduation_date = :graduation_date");
        selectQuery.bindValue(":type_id", typeId);
        selectQuery.bindValue(":profession", professionItem->text());
        selectQuery.bindValue(":graduation_date", eduData["eduDate"].toString());

        if (!selectQuery.exec())
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось получить ID образования: " + selectQuery.lastError().text());
            return;
        }

        if (selectQuery.next())
        {
            int educationId = selectQuery.value("id").toInt();

            QSqlQuery workersEduQuery(db);
            workersEduQuery.prepare(
                "INSERT INTO workers_educations (worker_id, education_id) "
                "VALUES (:worker_id, :education_id)");

            workersEduQuery.bindValue(":worker_id", workerId);
            workersEduQuery.bindValue(":education_id", educationId);

            if (!workersEduQuery.exec())
            {
                QMessageBox::critical(this, "Ошибка", "Не удалось связать образование с работником: " + workersEduQuery.lastError().text());
                return;
            }
        }
    }
    ui->le_addPhone->clear();
    ui->cb_post->clear();
    ui->cb_departament->clear();
    ui->cb_language->clear();
    ui->lw_language->clear();
    ui->lb_addPhoto->clear();
    ui->le_addName->clear();
    ui->le_addSurname->clear();
    ui->le_addPatronymic->clear();
    ui->cb_gender->clear();
    ui->de_addBirthDate->clear();
    ui->le_addBirthPlace->clear();
    ui->le_addPlace->clear();
    ui->le_addSeria->clear();
    ui->le_addNumber->clear();
    ui->le_addIssuePlace->clear();
    ui->de_addIssueDate->clear();
    ui->lb_addPasDoc->clear();
    ui->le_addProfession->clear();
    ui->cb_eduType->clear();
    ui->le_addQualification->clear();
    ui->le_addEduPlace->clear();
    ui->de_addEduDate->clear();
    ui->lb_AddEduDoc->clear();
    clearRowsInTable(ui->tw_addEducation);

    QMessageBox::information(this, "Успех", "Новый работник успешно добавлен в базу данных!");
}

//Добавить язык
void MainWindow::addLanguage()
{
    int languageId = ui->cb_language->currentData().toInt();
    QString languageName = ui->cb_language->currentText();

    if (languageName.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Выберите язык из списка");
        return;
    }

    for (int i = 0; i < ui->lw_language->count(); ++i)
    {
        QListWidgetItem *item = ui->lw_language->item(i);
        if (item->data(Qt::UserRole).toInt() == languageId)
        {
            QMessageBox::information(this, "Информация", "Этот язык уже добавлен");
            return;
        }
    }

    QListWidgetItem *newItem = new QListWidgetItem(languageName, ui->lw_language);
    newItem->setData(Qt::UserRole, languageId);
    ui->lw_language->addItem(newItem);
}

//Изменить язык
void MainWindow::editLanguage(QListWidgetItem *item)
{
    if (!item) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Удаление языка",
        QString("Вы уверены, что хотите удалить язык \"%1\" из списка?").arg(item->text()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes)
    {
        delete item;
    }
}

//Добавить запись образования
void MainWindow::addEducation()
{
    QString profession = ui->le_addProfession->text().trimmed();
    QString qualification = ui->le_addQualification->text().trimmed();
    QString eduType = ui->cb_eduType->currentText();
    QString eduPlace = ui->le_addEduPlace->text().trimmed();
    QDate eduDate = ui->de_addEduDate->date();

    QPixmap eduDocPixmap = ui->lb_AddEduDoc->pixmap(Qt::ReturnByValue);
    if (eduDocPixmap.isNull())
    {
        QMessageBox::warning(this, "Ошибка", "Изображение документа не выбрано!");
        return;
    }

    QByteArray eduDocData;
    QBuffer buffer(&eduDocData);
    buffer.open(QIODevice::WriteOnly);
    eduDocPixmap.save(&buffer, "PNG");

    if (profession.isEmpty() || eduType.isEmpty() || qualification.isEmpty() || eduPlace.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля!");
        return;
    }

    int eduTypeId = ui->cb_eduType->currentIndex();

    int row = ui->tw_addEducation->rowCount();
    ui->tw_addEducation->insertRow(row);

    QTableWidgetItem *professionItem = new QTableWidgetItem(profession);
    ui->tw_addEducation->setItem(row, 0, professionItem);

    QTableWidgetItem *eduTypeItem = new QTableWidgetItem(eduType);
    ui->tw_addEducation->setItem(row, 1, eduTypeItem);

    QVariantMap additionalData;
    additionalData["qualification"] = qualification;
    additionalData["eduPlace"] = eduPlace;
    additionalData["eduDate"] = eduDate.toString("yyyy-MM-dd");
    additionalData["eduDocImage"] = eduDocData;
    additionalData["typeId"] = eduTypeId;

    professionItem->setData(Qt::UserRole, additionalData);

    ui->le_addProfession->clear();
    ui->le_addQualification->clear();
    ui->cb_eduType->setCurrentIndex(-1);
    ui->le_addEduPlace->clear();
    ui->de_addEduDate->clear();
    ui->lb_AddEduDoc->clear();
}


//Изменить данные образования
void MainWindow::editEducation(QTableWidgetItem *item)
{
    if (!item) return;

    int row = item->row();
    QTableWidgetItem *professionItem = ui->tw_addEducation->item(row, 0);

    QVariantMap additionalData = professionItem->data(Qt::UserRole).toMap();

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Действие с записью");
    msgBox.setText("Вы хотите посмотреть информацию об этой записи или удалить её?");
    QPushButton *viewButton = msgBox.addButton("Посмотреть", QMessageBox::AcceptRole);
    QPushButton *deleteButton = msgBox.addButton("Удалить", QMessageBox::DestructiveRole);
    QPushButton *cancelButton = msgBox.addButton("Отмена", QMessageBox::RejectRole);

    msgBox.exec();
    QAbstractButton *clickedButton = msgBox.clickedButton();

    if (clickedButton == viewButton)
    {
        ui->le_addProfession->setText(professionItem->text());
        ui->le_addQualification->setText(additionalData["qualification"].toString());
        ui->cb_eduType->setCurrentText(ui->tw_addEducation->item(row, 1)->text());
        ui->le_addEduPlace->setText(additionalData["eduPlace"].toString());
        ui->de_addEduDate->setDate(QDate::fromString(additionalData["eduDate"].toString(), "yyyy-MM-dd"));

        QByteArray eduDocData = additionalData["eduDocImage"].toByteArray();
        QPixmap eduDocPixmap;
        eduDocPixmap.loadFromData(eduDocData);
        ui->lb_AddEduDoc->setPixmap(eduDocPixmap);

        QMessageBox::information(this, "Информация", "Данные загружены для просмотра и изменения.");
    }
    else if (clickedButton == deleteButton)
    {
        QMessageBox confirmBox(this);
        confirmBox.setWindowTitle("Удаление записи");
        confirmBox.setText("Вы уверены, что хотите удалить эту запись?");
        QPushButton *confirmYesButton = confirmBox.addButton("Да, удалить", QMessageBox::AcceptRole);
        QPushButton *confirmNoButton = confirmBox.addButton("Отмена", QMessageBox::RejectRole);
        confirmBox.exec();

        if (confirmBox.clickedButton() == confirmYesButton)
        {
            ui->tw_addEducation->removeRow(row);
            ui->le_addProfession->clear();
            ui->le_addQualification->clear();
            ui->cb_eduType->setCurrentIndex(-1);
            ui->le_addEduPlace->clear();
            ui->de_addEduDate->clear();
            ui->lb_AddEduDoc->clear();
            QMessageBox::information(this, "Удаление", "Запись успешно удалена.");
        }
    }
}

//Отчистить поля
void MainWindow::clear()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение", "Вы действительно хотите отчистить поля?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        ui->le_addProfession->clear();
        ui->le_addQualification->clear();
        ui->cb_eduType->setCurrentIndex(-1);
        ui->le_addEduPlace->clear();
        ui->de_addEduDate->clear();
        ui->lb_AddEduDoc->clear();
        QMessageBox::information(this, "Успех", "Поля очищены.");
    }
}

//Загрузить должности
void MainWindow::loadPosts()
{
    db = QSqlDatabase::database("db_kalugina");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_post->clear();
        ui->cb_post->setPlaceholderText("Должность");
        ui->cb_post->addItem("");
        query.prepare("SELECT id, name FROM posts");
        if(query.exec())
        {
            while (query.next())
            {
                int postId = query.value(0).toInt();
                QString postName = query.value(1).toString();
                ui->cb_post->addItem(postName, postId);
            }
        }
    }
}

//Загрузить отделы
void MainWindow::loadDepartaments()
{
    db = QSqlDatabase::database("db_kalugina");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_departament->clear();
        ui->cb_departament->setPlaceholderText("Отдел");
        ui->cb_departament->addItem("");
        query.prepare("SELECT id, name FROM departaments");
        if(query.exec())
        {
            while (query.next())
            {
                int departamentId = query.value(0).toInt();
                QString departamentName = query.value(1).toString();
                ui->cb_departament->addItem(departamentName, departamentId);
            }
        }
    }
}

//Загрузить языки
void MainWindow::loadLanguages()
{
    db = QSqlDatabase::database("db_kalugina");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_language->clear();
        ui->cb_language->setPlaceholderText("Язык");
        ui->cb_language->addItem("");
        query.prepare("SELECT id, name FROM languages");
        if(query.exec())
        {
            while (query.next())
            {
                int languageId = query.value(0).toInt();
                QString languageName = query.value(1).toString();
                ui->cb_language->addItem(languageName, languageId);
            }
        }
    }
}

//Загрузить пол
void MainWindow::loadGenders()
{
    db = QSqlDatabase::database("db_kalugina");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_gender->clear();
        ui->cb_gender->setPlaceholderText("Пол");
        ui->cb_gender->addItem("");
        query.prepare("SELECT id, name FROM genders");
        if(query.exec())
        {
            while (query.next())
            {
                int genderId = query.value(0).toInt();
                QString genderName = query.value(1).toString();
                ui->cb_gender->addItem(genderName, genderId);
            }
        }
    }
}

//Загрузить типы образования
void MainWindow::loadEduTypes()
{
    db = QSqlDatabase::database("db_kalugina");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_eduType->clear();
        ui->cb_eduType->setPlaceholderText("Образование");
        ui->cb_eduType->addItem("");
        query.prepare("SELECT id, name FROM education_types");
        if(query.exec())
        {
            while (query.next())
            {
                int educationId = query.value(0).toInt();
                QString educationName = query.value(1).toString();
                ui->cb_eduType->addItem(educationName, educationId);
            }
        }
    }
}

//Заполнить таблицу отделов
void MainWindow::fillDepartamentsTable()
{
    clearRowsInTable(ui->tw_departaments);
    ui->tw_departaments->setColumnHidden(1, true);
    ui->tw_departaments->setColumnWidth(0, 10);
    db = QSqlDatabase::database("db_kalugina");
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        query.exec("SELECT * FROM departaments;");
        while (query.next())
        {
            ui->tw_departaments->insertRow(ui->tw_departaments->rowCount());
            QCheckBox *chkb = new QCheckBox();
            chkb->setCheckState(Qt::Unchecked);
            ui->tw_departaments->setCellWidget(ui->tw_departaments->rowCount() - 1, 0, chkb);
            ui->tw_departaments->setItem(ui->tw_departaments->rowCount() - 1, 1, new QTableWidgetItem(query.value(0).toString()));
            ui->tw_departaments->setItem(ui->tw_departaments->rowCount() - 1, 2, new QTableWidgetItem(query.value(1).toString()));

            connect(chkb, &QCheckBox::stateChanged, this, &MainWindow::filterTable);
        }
        ui->tw_departaments->resizeRowsToContents();
    }
}

//Заполнить таблицу типов образований
void MainWindow::fillEduTypesTable()
{
    clearRowsInTable(ui->tw_eduTypes);
    ui->tw_eduTypes->setColumnHidden(1, true);
    ui->tw_eduTypes->setColumnWidth(0, 10);
    db = QSqlDatabase::database("db_kalugina");
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        query.exec("SELECT * FROM education_types;");
        while (query.next())
        {
            ui->tw_eduTypes->insertRow(ui->tw_eduTypes->rowCount());
            QCheckBox *chkb = new QCheckBox();
            chkb->setCheckState(Qt::Unchecked);
            ui->tw_eduTypes->setCellWidget(ui->tw_eduTypes->rowCount() - 1, 0, chkb);
            ui->tw_eduTypes->setItem(ui->tw_eduTypes->rowCount() - 1, 1, new QTableWidgetItem(query.value(0).toString()));
            ui->tw_eduTypes->setItem(ui->tw_eduTypes->rowCount() - 1, 2, new QTableWidgetItem(query.value(1).toString()));

            connect(chkb, &QCheckBox::stateChanged, this, &MainWindow::filterTable);
        }
        ui->tw_eduTypes->resizeRowsToContents();
    }
}

//Заполнить таблицу полов
void MainWindow::fillGendersTable()
{
    clearRowsInTable(ui->tw_genders);
    ui->tw_genders->setColumnHidden(1, true);
    ui->tw_genders->setColumnWidth(0, 10);
    db = QSqlDatabase::database("db_kalugina");
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        query.exec("SELECT * FROM genders;");
        while (query.next())
        {
            ui->tw_genders->insertRow(ui->tw_genders->rowCount());
            QCheckBox *chkb = new QCheckBox();
            chkb->setCheckState(Qt::Unchecked);
            ui->tw_genders->setCellWidget(ui->tw_genders->rowCount() - 1, 0, chkb);
            ui->tw_genders->setItem(ui->tw_genders->rowCount() - 1, 1, new QTableWidgetItem(query.value(0).toString()));
            ui->tw_genders->setItem(ui->tw_genders->rowCount() - 1, 2, new QTableWidgetItem(query.value(1).toString()));

            connect(chkb, &QCheckBox::stateChanged, this, &MainWindow::filterTable);
        }
        ui->tw_genders->resizeRowsToContents();
    }
}

//Получить ID отдела
QStringList MainWindow::getSelectedIDDepartaments()
{
    QStringList departaments;
    for (int row = 0; row < ui->tw_departaments->rowCount(); ++row)
    {
        QCheckBox *checkbox = qobject_cast<QCheckBox*>(ui->tw_departaments->cellWidget(row, 0));
        if (checkbox && checkbox->isChecked())
        {
            QString genreId = ui->tw_departaments->item(row, 1)->text();
            departaments.append(genreId);
        }
    }
    return departaments;
}

//Получить ID типа образования
QStringList MainWindow::getSelectedIDEduTypes()
{
    QStringList eduTypes;
    for (int row = 0; row < ui->tw_eduTypes->rowCount(); ++row)
    {
        QCheckBox *checkbox = qobject_cast<QCheckBox*>(ui->tw_eduTypes->cellWidget(row, 0));
        if (checkbox && checkbox->isChecked())
        {
            QString genreId = ui->tw_eduTypes->item(row, 1)->text();
            eduTypes.append(genreId);
        }
    }
    return eduTypes;
}

//Получить ID пола
QStringList MainWindow::getSelectedIDGenders()
{
    QStringList genders;
    for (int row = 0; row < ui->tw_genders->rowCount(); ++row)
    {
        QCheckBox *checkbox = qobject_cast<QCheckBox*>(ui->tw_genders->cellWidget(row, 0));
        if (checkbox && checkbox->isChecked())
        {
            QString genreId = ui->tw_genders->item(row, 1)->text();
            genders.append(genreId);
        }
    }
    return genders;
}

//Фильтр
void MainWindow::filterTable()
{
    QStringList departaments = getSelectedIDDepartaments();
    QStringList eduTypes = getSelectedIDEduTypes();
    QStringList genders = getSelectedIDGenders();

    if (departaments.isEmpty() && eduTypes.isEmpty() && genders.isEmpty())
    {
        clearRowsInTable(ui->tw_workers);
        loadWorkersTable();
        return;
    }

    QString queryStr = "SELECT DISTINCT w.id, p.name, p.surname, p.patronymic, d.name AS department_name, po.name AS post_name "
                       "FROM workers w "
                       "INNER JOIN pasports p ON w.id = p.worker_id "
                       "INNER JOIN departaments d ON w.departament_id = d.id "
                       "INNER JOIN posts po ON w.post_id = po.id "
                       "LEFT JOIN workers_educations we ON w.id = we.worker_id "
                       "LEFT JOIN educations e ON we.education_id = e.id "
                       "LEFT JOIN education_types et ON e.type_id = et.id";

    bool firstCondition = true;

    if (!departaments.isEmpty() || !eduTypes.isEmpty() || !genders.isEmpty())
    {
        queryStr += " WHERE";
    }

    if (!departaments.isEmpty())
    {
        queryStr += " w.departament_id IN (";
        for (int i = 0; i < departaments.size(); ++i)
        {
            queryStr += "?";
            if (i < departaments.size() - 1)
            {
                queryStr += ", ";
            }
        }
        queryStr += ")";
        firstCondition = false;
    }

    if (!eduTypes.isEmpty())
    {
        if (!firstCondition)
        {
            queryStr += " AND";
        }
        queryStr += " et.id IN (";
        for (int i = 0; i < eduTypes.size(); ++i)
        {
            queryStr += "?";
            if (i < eduTypes.size() - 1)
            {
                queryStr += ", ";
            }
        }
        queryStr += ")";
        firstCondition = false;
    }

    if (!genders.isEmpty())
    {
        if (!firstCondition)
        {
            queryStr += " AND";
        }
        queryStr += " p.gender_id IN (";
        for (int i = 0; i < genders.size(); ++i)
        {
            queryStr += "?";
            if (i < genders.size() - 1)
            {
                queryStr += ", ";
            }
        }
        queryStr += ")";
    }

    clearRowsInTable(ui->tw_workers);
    db = QSqlDatabase::database("db_kalugina");

    QSqlQuery query(db);
    query.prepare(queryStr);

    int bindIndex = 0;
    for (const QString &departament : departaments)
    {
        query.bindValue(bindIndex++, departament);
    }
    for (const QString &eduType : eduTypes)
    {
        query.bindValue(bindIndex++, eduType);
    }
    for (const QString &gender : genders)
    {
        query.bindValue(bindIndex++, gender);
    }

    if (!query.exec())
    {
        qDebug() << "Query execution failed:" << query.lastError().text();
        qDebug() << "Query:" << queryStr;
        return;
    }

    ui->tw_workers->setRowCount(0);

    int row = 0;
    while (query.next())
    {
        ui->tw_workers->insertRow(row);

        int workerId = query.value("id").toInt();
        QString name = query.value("name").toString();
        QString surname = query.value("surname").toString();
        QString patronymic = query.value("patronymic").toString();
        QString fullName = surname + " " + name + " " + patronymic;
        QString departmentName = query.value("department_name").toString();
        QString postName = query.value("post_name").toString();

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(workerId));
        QTableWidgetItem *fullNameItem = new QTableWidgetItem(fullName);
        QTableWidgetItem *departmentItem = new QTableWidgetItem(departmentName);
        QTableWidgetItem *postItem = new QTableWidgetItem(postName);

        ui->tw_workers->setItem(row, 0, idItem);
        ui->tw_workers->setItem(row, 1, fullNameItem);
        ui->tw_workers->setItem(row, 2, postItem);
        ui->tw_workers->setItem(row, 3, departmentItem);

        row++;
    }
}

//Отчистить таблицу
void MainWindow::clearRowsInTable(QTableWidget *table)
{
    int rowCount = table->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        table->removeRow(0);
    }
}

//Перейти в меню изменения данных
void MainWindow::toChange(int workerId)
{
    ui->qpb_addWorker->setVisible(false);
    ui->frame_main->setGeometry(ui->frame_main->x(), 1000, ui->frame_main->width(), ui->frame_main->height());
    ui->frame_addNewWorker->setGeometry(ui->frame_addNewWorker->x(), 0, ui->frame_addNewWorker->width(), ui->frame_addNewWorker->height());
    ui->frame_worker->setGeometry(ui->frame_worker->x(), 1000, ui->frame_worker->width(), ui->frame_worker->height());
    ui->frame_main->setEnabled(false);
    ui->frame_addNewWorker->setEnabled(true);
    ui->frame_worker->setEnabled(false);
    ui->tabWidget_2->setCurrentIndex(0);
    loadPosts();
    loadDepartaments();
    loadLanguages();
    loadGenders();
    loadEduTypes();
    loadWorkerData(workerId);
}

//Загрузить данные для измнения
void MainWindow::loadWorkerData(int workerId)
{
    db = QSqlDatabase::database("db_kalugina");

    if (!getDBConnection(db))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных!");
        return;
    }

    if (!db.isOpen())
    {
        QMessageBox::critical(this, "Ошибка", "База данных не открыта.");
        return;
    }

    QSqlQuery query(db);
    query.prepare(
        "SELECT w.phone, w.photo, w.post_id, w.departament_id, w.hire_date, "
        "p.name, p.surname, p.patronymic, p.gender_id, p.birth_date, p.birth_place, "
        "p.registration_place, p.seria, p.number, p.issue_place, p.issue_date, p.pasport_photo "
        "FROM workers w "
        "JOIN pasports p ON w.id = p.worker_id "
        "WHERE w.id = :worker_id");
    query.bindValue(":worker_id", workerId);

    if (!query.exec() || !query.next())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные сотрудника: " + query.lastError().text());
        return;
    }

    ui->le_addPhone->setText(query.value("phone").toString());
    ui->cb_post->setCurrentIndex(ui->cb_post->findData(query.value("post_id").toInt()));
    ui->cb_departament->setCurrentIndex(ui->cb_departament->findData(query.value("departament_id").toInt()));

    QPixmap photo;
    photo.loadFromData(query.value("photo").toByteArray());
    ui->lb_addPhoto->setPixmap(photo);

    ui->le_addName->setText(query.value("name").toString());
    ui->le_addSurname->setText(query.value("surname").toString());
    ui->le_addPatronymic->setText(query.value("patronymic").toString());
    ui->cb_gender->setCurrentIndex(ui->cb_gender->findData(query.value("gender_id").toInt()));
    ui->de_addBirthDate->setDate(query.value("birth_date").toDate());
    ui->le_addBirthPlace->setText(query.value("birth_place").toString());
    ui->le_addPlace->setText(query.value("registration_place").toString());
    ui->le_addSeria->setText(query.value("seria").toString());
    ui->le_addNumber->setText(query.value("number").toString());
    ui->le_addIssuePlace->setText(query.value("issue_place").toString());
    ui->de_addIssueDate->setDate(query.value("issue_date").toDate());

    QPixmap passportPhoto;
    passportPhoto.loadFromData(query.value("pasport_photo").toByteArray());
    ui->lb_addPasDoc->setPixmap(passportPhoto);

    loadWorkerEducations(workerId);
    loadWorkerLanguages(workerId);

    connect(ui->qpb_update, &QPushButton::clicked, this, [this, workerId]() { updateWorkerData(workerId); });
}

//Загрузить данные образования
void MainWindow::loadWorkerEducations(int workerId)
{
    ui->tw_addEducation->setRowCount(0);

    QSqlQuery query(db);
    query.prepare(
        "SELECT e.type_id, e.profession, e.graduation_date, e.education_place, "
        "e.education_document, e.qualification, t.name AS type_name "
        "FROM workers_educations we "
        "JOIN educations e ON we.education_id = e.id "
        "JOIN education_types t ON e.type_id = t.id "
        "WHERE we.worker_id = :worker_id");
    query.bindValue(":worker_id", workerId);

    if (!query.exec())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные об образовании: " + query.lastError().text());
        return;
    }

    while (query.next())
    {
        int row = ui->tw_addEducation->rowCount();
        ui->tw_addEducation->insertRow(row);

        QTableWidgetItem *professionItem = new QTableWidgetItem(query.value("profession").toString());
        QTableWidgetItem *eduTypeItem = new QTableWidgetItem(query.value("type_name").toString());

        ui->tw_addEducation->setItem(row, 0, professionItem);
        ui->tw_addEducation->setItem(row, 1, eduTypeItem);

        QVariantMap additionalData;
        additionalData["qualification"] = query.value("qualification").toString();
        additionalData["eduPlace"] = query.value("education_place").toString();
        additionalData["eduDate"] = query.value("graduation_date").toString();
        additionalData["eduDocImage"] = query.value("education_document").toByteArray();
        additionalData["typeId"] = query.value("type_id").toInt();

        professionItem->setData(Qt::UserRole, additionalData);
    }
}

//Загрузить данные языка
void MainWindow::loadWorkerLanguages(int workerId)
{
    ui->lw_language->clear();

    QSqlQuery query(db);
    query.prepare(
        "SELECT l.id, l.name FROM workers_languages wl "
        "JOIN languages l ON wl.language_id = l.id "
        "WHERE wl.worker_id = :worker_id");
    query.bindValue(":worker_id", workerId);

    if (!query.exec())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить языки сотрудника: " + query.lastError().text());
        return;
    }

    while (query.next())
    {
        QListWidgetItem *item = new QListWidgetItem(query.value("name").toString(), ui->lw_language);
        item->setData(Qt::UserRole, query.value("id").toInt());
        ui->lw_language->addItem(item);
    }
}

//Обвноить данные сотрудника
void MainWindow::updateWorkerData(int workerId)
{
    db = QSqlDatabase::database("db_kalugina");

    if (!getDBConnection(db))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных!");
        return;
    }

    if (!db.isOpen())
    {
        QMessageBox::critical(this, "Ошибка", "База данных не открыта.");
        return;
    }

    QSqlQuery query(db);
    query.prepare(
        "UPDATE workers SET phone = :phone, photo = :photo, post_id = :post_id, "
        "departament_id = :departament_id WHERE id = :worker_id");
    query.bindValue(":phone", ui->le_addPhone->text().trimmed());

    QPixmap photo = ui->lb_addPhoto->pixmap(Qt::ReturnByValue);
    QByteArray photoData;
    QBuffer photoBuffer(&photoData);
    photoBuffer.open(QIODevice::WriteOnly);
    photo.save(&photoBuffer, "PNG");

    query.bindValue(":photo", photoData);
    query.bindValue(":post_id", ui->cb_post->currentData().toInt());
    query.bindValue(":departament_id", ui->cb_departament->currentData().toInt());
    query.bindValue(":worker_id", workerId);

    if (!query.exec())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить данные сотрудника: " + query.lastError().text());
        return;
    }

    updatePassportData(workerId);
    updateWorkerLanguages(workerId);
    updateWorkerEducations(workerId);

    QMessageBox::information(this, "Успех", "Данные сотрудника успешно обновлены!");
}

//Обновить паспортные данные
void MainWindow::updatePassportData(int workerId)
{
    QSqlQuery query(db);
    query.prepare(
        "UPDATE pasports SET "
        "name = :name, surname = :surname, patronymic = :patronymic, gender_id = :gender_id, "
        "birth_date = :birth_date, birth_place = :birth_place, registration_place = :registration_place, "
        "seria = :seria, number = :number, issue_place = :issue_place, issue_date = :issue_date, "
        "pasport_photo = :pasport_photo "
        "WHERE worker_id = :worker_id");

    query.bindValue(":name", ui->le_addName->text().trimmed());
    query.bindValue(":surname", ui->le_addSurname->text().trimmed());
    query.bindValue(":patronymic", ui->le_addPatronymic->text().trimmed());
    query.bindValue(":gender_id", ui->cb_gender->currentData().toInt());
    query.bindValue(":birth_date", ui->de_addBirthDate->date());
    query.bindValue(":birth_place", ui->le_addBirthPlace->text().trimmed());
    query.bindValue(":registration_place", ui->le_addPlace->text().trimmed());
    query.bindValue(":seria", ui->le_addSeria->text().trimmed());
    query.bindValue(":number", ui->le_addNumber->text().trimmed());
    query.bindValue(":issue_place", ui->le_addIssuePlace->text().trimmed());
    query.bindValue(":issue_date", ui->de_addIssueDate->date());

    QPixmap passportPhoto = ui->lb_addPasDoc->pixmap(Qt::ReturnByValue);
    QByteArray passportPhotoData;
    QBuffer passportBuffer(&passportPhotoData);
    passportBuffer.open(QIODevice::WriteOnly);
    passportPhoto.save(&passportBuffer, "PNG");
    query.bindValue(":pasport_photo", passportPhotoData);
    query.bindValue(":worker_id", workerId);

    if (!query.exec())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить паспортные данные: " + query.lastError().text());
    }
}

//Обновить данные образования
void MainWindow::updateWorkerEducations(int workerId)
{
    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM workers_educations WHERE worker_id = :worker_id");
    deleteQuery.bindValue(":worker_id", workerId);

    if (!deleteQuery.exec())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось удалить старые данные об образовании: " + deleteQuery.lastError().text());
        return;
    }

    for (int row = 0; row < ui->tw_addEducation->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->tw_addEducation->item(row, 0);
        QVariantMap additionalData = item->data(Qt::UserRole).toMap();

        QSqlQuery insertQuery(db);
        insertQuery.prepare(
            "INSERT INTO educations (type_id, profession, graduation_date, education_place, education_document, qualification) "
            "VALUES (:type_id, :profession, :graduation_date, :education_place, :education_document, :qualification)");
        insertQuery.bindValue(":type_id", additionalData["typeId"]);
        insertQuery.bindValue(":profession", item->text());
        insertQuery.bindValue(":graduation_date", additionalData["eduDate"]);
        insertQuery.bindValue(":education_place", additionalData["eduPlace"]);
        insertQuery.bindValue(":education_document", additionalData["eduDocImage"]);
        insertQuery.bindValue(":qualification", additionalData["qualification"]);

        if (!insertQuery.exec())
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить данные об образовании: " + insertQuery.lastError().text());
            return;
        }

        int educationId = insertQuery.lastInsertId().toInt();

        QSqlQuery linkQuery(db);
        linkQuery.prepare(
            "INSERT INTO workers_educations (worker_id, education_id) VALUES (:worker_id, :education_id)");
        linkQuery.bindValue(":worker_id", workerId);
        linkQuery.bindValue(":education_id", educationId);

        if (!linkQuery.exec())
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось связать данные об образовании с сотрудником: " + linkQuery.lastError().text());
            return;
        }
    }
}

//Обновить данные языка
void MainWindow::updateWorkerLanguages(int workerId)
{
    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM workers_languages WHERE worker_id = :worker_id");
    deleteQuery.bindValue(":worker_id", workerId);

    if (!deleteQuery.exec())
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось удалить старые данные о языках: " + deleteQuery.lastError().text());
        return;
    }

    for (int i = 0; i < ui->lw_language->count(); ++i)
    {
        QListWidgetItem *item = ui->lw_language->item(i);
        int languageId = item->data(Qt::UserRole).toInt();

        QSqlQuery insertQuery(db);
        insertQuery.prepare(
            "INSERT INTO workers_languages (worker_id, language_id) VALUES (:worker_id, :language_id)");
        insertQuery.bindValue(":worker_id", workerId);
        insertQuery.bindValue(":language_id", languageId);

        if (!insertQuery.exec())
        {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить данные о языке: " + insertQuery.lastError().text());
            return;
        }
    }
}

//Удаление сотрудника
void MainWindow::deleteWorker(int workerId)
{
    if (getDBConnection(db))
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите удалить сотрудника?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            QSqlQuery query(db);

            query.prepare("SELECT education_id FROM workers_educations WHERE worker_id = :id");
            query.bindValue(":id", workerId);
            if (!query.exec())
            {
                qDebug() << "Ошибка при получении education_id: " << query.lastError().text();
                return;
            }

            QList<int> educationIds;
            while (query.next())
            {
                educationIds.append(query.value(0).toInt());
            }

            query.prepare("DELETE FROM workers_educations WHERE worker_id = :id");
            query.bindValue(":id", workerId);
            if (!query.exec())
            {
                qDebug() << "Ошибка при удалении из workers_educations: " << query.lastError().text();
                return;
            }

            foreach (int educationId, educationIds)
            {
                query.prepare("DELETE FROM educations WHERE id = :educationId");
                query.bindValue(":educationId", educationId);
                if (!query.exec())
                {
                    qDebug() << "Ошибка при удалении из educations: " << query.lastError().text();
                    return;
                }
            }

            query.prepare("DELETE FROM workers_languages WHERE worker_id = :id");
            query.bindValue(":id", workerId);
            if (!query.exec())
            {
                qDebug() << "Ошибка при удалении из workers_languages: " << query.lastError().text();
                return;
            }

            query.prepare("DELETE FROM pasports WHERE worker_id = :id");
            query.bindValue(":id", workerId);
            if (!query.exec())
            {
                qDebug() << "Ошибка при удалении из pasports: " << query.lastError().text();
                return;
            }

            query.prepare("DELETE FROM workers WHERE id = :id");
            query.bindValue(":id", workerId);
            if (!query.exec())
            {
                qDebug() << "Ошибка при удалении из workers: " << query.lastError().text();
                return;
            }

            showMessage("Сотрудник успешно удалён!", "", QMessageBox::Ok, QMessageBox::Information);
            showMainMenu();
        }
    }
}
