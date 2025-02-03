#include "global.h"

bool getDBConnection(QSqlDatabase db)
{
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("db_kalugina");
    db.setUserName("root");
    db.setPassword("1234");
    return db.open();
}

int showMessage(QString message, QString windowTitle, QMessageBox::Icon icon, QList<QMessageBox::Button> buttons)
{
    QMessageBox msg;
    msg.setText(message);
    msg.setWindowTitle(windowTitle);
    msg.setIcon(icon);
    for(const auto &button:buttons)
    {
        msg.addButton(button);
    }
    if(buttons.length()>0)
    {
        msg.setDefaultButton(buttons.at(0));
    }
    return msg.exec();
}

int showMessage(QString message, QString title, QMessageBox::Button button, QMessageBox::Icon icon)
{
    QMessageBox msg;
    msg.setWindowTitle(title);
    msg.setText(message);
    msg.setDefaultButton(button);
    msg.setIcon(icon);
    return msg.exec();
}

int showMessage(QString message, QString title, QList<QMessageBox::Button> buttons, QMessageBox::Icon icon)
{
    QMessageBox msg;
    msg.setWindowTitle(title);
    msg.setText(message);
    for(QMessageBox::Button button : buttons)
    {
        msg.addButton(button);
    }
    msg.setDefaultButton(buttons.at(buttons.length()-1));
    msg.setIcon(icon);
    return msg.exec();
}
