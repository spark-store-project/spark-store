#ifndef APPLICATION_H
#define APPLICATION_H

#include <DApplication>

#include <QJsonObject>

DWIDGET_USE_NAMESPACE

class MainWindow;
class Application : public DApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    void handleAboutAction() override;

    static void checkAppConfigLocation();

    void setBuildDateTime(const QString &buildDateTime);
    void setMainWindow(MainWindow *window);

private:
    void initAboutDialog();
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 4, 0))
    void initFeatureDisplayDialog();
#endif

private:
    QJsonObject m_featuresJsonObj;
    QString m_version;
    QString m_buildDateTime;

    MainWindow *m_mainWindow = nullptr;
};

#endif // APPLICATION_H
