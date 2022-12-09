#include "settingspage.h"
#include "ui_settingspage.h"

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPage)
{
    ui->setupUi(this);
}

void SettingsPage::setTheme(bool dark)
{
    if(dark)
    {
        this->setStyleSheet("#frame{background-color: #252525;border-radius:14px;border:1px solid rgb(64, 64, 64);}");
    }else {
        //亮色模式
        this->setStyleSheet("#frame{background-color: #ffffff;border-radius:14px;border:1px solid rgb(229,229,229);}");
    }
}

SettingsPage::~SettingsPage()
{
    delete ui;
}
