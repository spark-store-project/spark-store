#include <QApplication>
#include <QPushButton>
#include <QTranslator>
#include <spklogging.h>
#include "spkui_general.h"
#include "spkstore.h"

#include "spkdownload.h"
#include "spkimgviewer.h"


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  qRegisterMetaType<SpkDownloadMgr::TaskResult>("TaskResult");

  QString LogPath = "";

  QTranslator tr;
  QLocale loc;
  QString trDir;
#ifndef NDEBUG
  trDir = ".";
#else // Translation files (zh.qm etc) installed to this path!
  trDir = "/usr/share/spark-store/translations/";
#endif
  if(tr.load(loc, "", "", "."))
    a.installTranslator(&tr);

  SpkStore store(false, LogPath);

  return QApplication::exec();
}
