#include "qpushlabel.h"

QPushLabel::QPushLabel(QWidget *parent)
    : QLabel{parent}
{

}

bool QPushLabel :: event (QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress)
    {
        emit clicked();
    }
    else if (e->type() == QEvent::MouseButtonDblClick)
    {
        emit doubleClicked();
    }
    return QWidget :: event (e);
}
