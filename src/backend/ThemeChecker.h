#ifndef THEMECHECKER_H
#define THEMECHECKER_H

#include <QObject>
#include <QTimer>


class ThemeChecker : public QObject
{
    Q_OBJECT
public:
    explicit ThemeChecker(QObject *parent = nullptr);

signals:
    void themeChanged(bool isDark);

public slots:
    void checkThemeChange();


private:
    int lastColorSchema;
    QTimer *timer;
};

#endif // THEMECHECKER_H
