#include <QApplication>
#include <QPushButton>
#include <QTranslator>
#include <spklogging.h>
#include "spkui_general.h"
#include "spkstore.h"

#include "spkdownload.h"
#include "spkmsgbox.h"


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

//  SpkDownloadMgr dl;
//  dl.SetNewServers({
//                     "https://d1.store.deepinos.org.cn/",
//                     "https://d2.store.deepinos.org.cn/",
//                     "https://d3.store.deepinos.org.cn/",
//                     "https://d4.store.deepinos.org.cn/",
//                     "https://d5.store.deepinos.org.cn/"
//                   });
//  dl.SetDestinationFolder("/tmp/");
//  dl.StartNewDownload("store/office/cn.com.10jqka/cn.com.10jqka_1.6.1.2_amd64.deb", 0);
//  QObject::connect(&dl, &SpkDownloadMgr::DownloadStopped,
//                   [&](SpkDownloadMgr::TaskResult, int)
//  {
//    SpkMsgBox::StaticExec("Finished", "");
//  });

  return QApplication::exec();
}
