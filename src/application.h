#ifndef APPLICATION_H
#define APPLICATION_H

#include <DApplication>

DWIDGET_USE_NAMESPACE

class Application : public DApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    void handleAboutAction() override;

    static void checkAppConfigLocation();

    void setVersionAndBuildDateTime(const QString &version, const QString &buildDateTime);

private:
    void initAboutDialog();

private:
    QString m_version;
    QString m_buildDateTime;
};

#endif // APPLICATION_H
