#ifndef QPUSHLABEL_H
#define QPUSHLABEL_H
#include <QtWidgets/QLabel>
#include <QEvent>
#include <QWidget>

class QPushLabel : public QLabel
{
    Q_OBJECT
public:
    explicit QPushLabel(QWidget *parent = nullptr);

signals:
    void clicked();
    void doubleClicked();

protected:
    bool event(QEvent *e);
};

#endif // QPUSHLABEL_H
