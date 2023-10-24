#ifndef DATACOLLECTORANDUPLOADER_H
#define DATACOLLECTORANDUPLOADER_H

#include <QObject>

class DataCollectorAndUploader : public QObject
{
    Q_OBJECT
public:
    explicit DataCollectorAndUploader(QObject *parent = nullptr);
    void collectAndUploadData();

signals:
    void uploadSuccessful();
    void uploadFailed(QString errorString);

private:
    void collectData();
};

#endif // DATACOLLECTORANDUPLOADER_H
