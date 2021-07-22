
#pragma once

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include "spkdialog.h"

class SpkAbout : public SpkDialog
{
  public:
    SpkAbout(QWidget* parent = nullptr);
    ~SpkAbout();
    static void Show();

  private:
    QHBoxLayout *mIconLay;
    QLabel *mSpkIcon;
    QLabel *mSpkVersion;
    QLabel *mDescriptionText;
};
