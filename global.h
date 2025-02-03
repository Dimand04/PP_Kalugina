#ifndef GLOBAL_H
#define GLOBAL_H
#include <QSqlDatabase>
#include <qmessagebox.h>
#include <QProcess>
#include <QDebug>
#include <QString>

bool getDBConnection(QSqlDatabase db);
int showMessage(QString message, QString windowTitle, QMessageBox::Icon, QList<QMessageBox::Button>buttons);
int showMessage(QString, QString, QMessageBox::Button, QMessageBox::Icon);
int showMessage(QString, QString, QList<QMessageBox::Button>, QMessageBox::Icon);

#endif // GLOBAL_H
