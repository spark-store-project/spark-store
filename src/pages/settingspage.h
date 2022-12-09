#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

namespace Ui {
class SettingsPage;
}

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    void setTheme(bool dark);
    ~SettingsPage();

private:
    Ui::SettingsPage *ui;
};

#endif // SETTINGSPAGE_H
