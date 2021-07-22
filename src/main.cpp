#include <QApplication>
#include <QPushButton>
#include <spklogging.h>
#include "spkui_general.h"
#include "spkstore.h"


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QString LogPath = "";

  SpkStore store(false, LogPath);

  return QApplication::exec();
}
