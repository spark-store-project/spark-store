//
// Created by rigoligo on 2021/5/8.
//

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QLibrary>
#include <QDir>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QScreen>
#include <QPluginLoader>
#include <QStyleFactory>
#include <csignal>
#include <execinfo.h>

#include "spkui_general.h"
#include "spkmsgbox.h"
#include "spkpopup.h"
#include "spklogging.h"

namespace SpkUi
{
  UiMetaObject SpkUiMetaObject;
  SpkUiStyle CurrentStyle;
  QString StylesheetBase, CurrentStylesheet;
  QColor ColorLine, ColorBack;
  QSize PrimaryScreenSize;
  SpkDtkPlugin *DtkPlugin = nullptr;
  QStyle *OldSystemStyle = nullptr;

  std::map<Qss::ColorSetIndex, QColor> CurrentColorSet;

  SpkPopup *Popup;

  namespace States
  {
    bool IsDDE = false, IsUsingDtkPlugin = false;
  }

  namespace Priv
  {
    bool CrashHandlerActivated;
  }

  void Initialize()
  {
    // Obtain global stylesheets
    QFile ObtainStylesheet;
    ObtainStylesheet.setFileName(":/stylesheets/stylesheets/mainwindow_dark.css");
    ObtainStylesheet.open(QIODevice::ReadOnly);
    StylesheetBase = ObtainStylesheet.readAll();
    ObtainStylesheet.close();

    CurrentStyle = SpkUiStyle::Invalid;
    SetGlobalStyle(Light, false);

    // Initalize crash handler
    signal(SIGSEGV, SpkUi::CrashSignalHandler);
    signal(SIGABRT, SpkUi::CrashSignalHandler);
    signal(SIGFPE, SpkUi::CrashSignalHandler);

    // Prepare theme following for DDE
    if((States::IsDDE = CheckIsDeepinDesktop()))
      PrepareForDeepinDesktop();

    // Misc data initialization
    PrimaryScreenSize = QGuiApplication::primaryScreen()->size();
  }

  void GuessAppropriateTheme()
  {

  }

  bool CheckIsDeepinDesktop()
  {
    QString Desktop(getenv("XDG_CURRENT_DESKTOP"));
    // This method of checking is from DTK source code.
    if(Desktop.contains("deepin", Qt::CaseInsensitive) ||
       Desktop.contains("tablet", Qt::CaseInsensitive))
      return true;
    else
      return false;
  }

  void PrepareForDeepinDesktop()
  {
#ifndef NDEBUG
    qApp->addLibraryPath(qApp->applicationDirPath() + "/plugin/dtkplugin");
#else
    // You must `make install' before these work in release mode
    qApp->addLibraryPath("/usr/local/lib");
    qApp->addLibraryPath("/usr/lib");
#endif
    if(!qgetenv("SPARK_NO_DTK_PLUGIN").toInt())
    {
      QPluginLoader p("libspkdtkplugin");
      if(p.load())
      {
        auto i = qobject_cast<SpkDtkPlugin*>(p.instance());
        if(i)
        {
          DtkPlugin = i;
          States::IsUsingDtkPlugin = true;
        }

        i->Initialize();

        SpkUiMetaObject.SetAccentColor(i->GetAccentColor()); // Match OS accent color
        SpkUiMetaObject.SetDarkLightTheme(i->GetIsDarkTheme()); // Match OS dark theme type

        QObject::connect(i, &SpkDtkPlugin::AccentColorChanged,
                         &SpkUiMetaObject, &UiMetaObject::SetAccentColor);
        QObject::connect(i, &SpkDtkPlugin::DarkLightThemeChanged,
                         &SpkUiMetaObject, &UiMetaObject::SetDarkLightTheme);
      }
    }

    // NOTE: Chameleon style kept adding unwanted blue focus indication border
    // to widgets that shouldn't have borders.
    // We need to eliminate this irritating problem.
    if(qgetenv("SPARK_NO_QSTYLE_CHANGE").toInt())
      return;
    OldSystemStyle = QStyleFactory::create("chameleon"); // TreeWidget doesn't work well with Fusion
    auto styles = QStyleFactory::keys();
    styles.removeAll("chameleon");
    if(styles.contains("Fusion"))
    {
      auto style = QStyleFactory::create("Fusion");
      qApp->setStyle(style);
    }
    else if(styles.size()) // What? This shouldn't happen.
      qApp->setStyle(QStyleFactory::create(styles[0]));
    else // Duh...
      sWarn(QObject::tr("Cannot find styles other than 'chameleon'! You may see widgets "
                        "with unwanted blue borders."));
  }

  void SetGlobalStyle(const SpkUiStyle aStyle, const bool aPreserveAccentColor)
  {
    if(aStyle == CurrentStyle) // Don't waste precious CPU time parsing new style sheet!
      return;
    CurrentStyle = aStyle;
    Qss::ColorSet tempset;
    switch(aStyle)
    {
      case Light:
        tempset = Qss::LightColorSet;
        ColorLine = Qt::black;
        break;
      case Dark:
        tempset = Qss::DarkColorSet;
        ColorLine = Qt::white;
        break;
    }
    if(aPreserveAccentColor)
    {
      for(auto i : Qss::AccentColorExceptions)
        tempset[i] = CurrentColorSet[i];
    }
    CurrentColorSet = tempset;
    CurrentStylesheet = StylesheetFromColors(CurrentColorSet);
    qApp->setStyleSheet(CurrentStylesheet);
  }

  QString WriteStackTrace(const QString &aStackTrace)
  {
    QString path = QDir::homePath() + "/.local/share/spark-store/crash/";
    QFile StackTraceFile;
    if(!QDir().exists(path))
      if(!QDir().mkpath(path))
        return QObject::tr("Stack trace directory %1 cannot be created. "
                           "Stack trace wasn't saved.").arg(path);
    path += QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
    while(QFile::exists(path))
      path += "_";
    StackTraceFile.setFileName(path);
    StackTraceFile.open(QIODevice::WriteOnly);
    if(StackTraceFile.isOpen() && StackTraceFile.isWritable())
    {
      QTextStream StackTraceWriter;
      StackTraceWriter.setDevice(&StackTraceFile);
      StackTraceWriter << QDateTime::currentDateTime().toLocalTime().toString() << "\n\n";
      StackTraceWriter << aStackTrace;
      StackTraceFile.close();
      return QObject::tr("Stack trace written to \"%1\".").arg(path);
    }
    return QObject::tr("Stack trace file %1 cannot be opened. "
                       "Stack trace wasn't saved.").arg(path);
  }

  void CrashSignalHandler(int sig)
  {
    QString msg(QObject::tr("Program has received signal %1 during normal execution.\n\n")),
            trace;
    switch(sig)
    {
      case SIGSEGV:
        msg = msg.arg(QObject::tr("\"SIGSEGV\" (Segmentation fault)"));
        goto CRASH;
      case SIGFPE:
        msg = msg.arg(QObject::tr("\"SIGFPE\" (Arithmetic exception)"));
        goto CRASH;
      case SIGABRT:
        msg = msg.arg(QObject::tr("\"SIGABRT\" (Abort)"));
      CRASH:
        {
          if(Priv::CrashHandlerActivated) // If error occured in the handler...
          {
            signal(SIGABRT, SIG_DFL); // Return control flow to OS and give up
            raise(SIGABRT);
            exit(2);
          }
          Priv::CrashHandlerActivated = true;
          void* TraceStack[StackTraceArraySize];
          int StackTraceSize = backtrace(TraceStack, StackTraceArraySize);
          auto TraceTextArray = backtrace_symbols(TraceStack, StackTraceArraySize);
          trace = QString(QObject::tr("Stack trace:\n"));
          for(int i = 0; i < StackTraceSize; i++)
            trace += QString::number(i) + ">  " + QString(TraceTextArray[i]) + '\n';
          msg += QObject::tr("Spark Store cannot continue.\n\n");
          msg += WriteStackTrace(trace);
          SpkMsgBox::StaticExec(msg, QObject::tr("Spark Store Crashed"), QMessageBox::Critical,
                                QMessageBox::Ok, trace);
          exit(2);
        }
      default:
        sErrPop(QObject::tr("Unknown signal %1 received in crash handler. "
                            "Program internals may be corrupted. Please decide if you want "
                            "to continue execution.").arg(sig));
    }
  }

  QIcon GetThemedIcon(QString name)
  {
    if(CurrentStyle == SpkUiStyle::Dark)
      name += "-dark";
    return QIcon(":/icons/" + name + ".svg");
  }

  QString StylesheetFromColors(Qss::ColorSet aColors)
  {
    QString ret = StylesheetBase;
    foreach(auto &i, aColors)
      ret = ret.replace(Qss::ColorSet2Token.at(i.first), i.second.name());
    return ret;
  }

  QColor ColorTextOnBackground(QColor c)
  {
    // From https://github.com/feiyangqingyun/qtkaifajingyan
    double gray = (0.299 * c.red() + 0.587 * c.green() + 0.114 * c.blue()) / 255;
    return gray > 0.5 ? Qt::black : Qt::white;
  }

  void UiMetaObject::SetAccentColor(QColor aColor)
  {
    CurrentColorSet[Qss::AccentColor] = aColor.lighter(90);
    CurrentColorSet[Qss::AccentColorHighlighted] = aColor.lighter(105);
    CurrentColorSet[Qss::TextOnAccentColor] = ColorTextOnBackground(aColor);
    qApp->setStyleSheet(StylesheetFromColors(CurrentColorSet));
  }

  void UiMetaObject::SetDarkLightTheme(bool isDark)
  {
    if(isDark)
      SetGlobalStyle(Dark, true);
    else
      SetGlobalStyle(Light, true);
  }
}
