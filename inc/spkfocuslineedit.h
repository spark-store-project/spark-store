
#pragma once

#include <QtWidgets/QLineEdit>

class SpkFocusLineEdit final : public QLineEdit
{
    Q_OBJECT
  public:
    explicit SpkFocusLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}

  protected:
    void focusInEvent(QFocusEvent *e) override { emit focusGained(); }
    void focusOutEvent(QFocusEvent *e) override { emit focusLost(); }

  signals:
    void focusGained();
    void focusLost();
};