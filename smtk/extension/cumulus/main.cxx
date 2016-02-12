#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QStringList>

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "mainwindow.h"

#include <cstdio>

void printUsage();

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QString url;

  QStringList args = QCoreApplication::arguments();

  if (args.length() != 3) {
    printUsage();
    return EXIT_FAILURE;
  }

  for (QStringList::const_iterator it = args.constBegin() + 1,
       itEnd = args.constEnd(); it != itEnd; ++it) {
    if (*it == "-u" || *it == "--url") {
      url = *(++it);
      break;
    }
    else if (*it == "-h" || *it == "-H" || *it == "--help" || *it == "-help") {
      printUsage();
      return EXIT_SUCCESS;
    }
    else {
      qWarning(qPrintable(QObject::tr("Unrecognized command line option: %s")),
               qPrintable(*it));
      printUsage();
      return EXIT_FAILURE;
    }
  }

  if (url.isEmpty()) {
    printUsage();
    return EXIT_FAILURE;
  }

  QApplication::setQuitOnLastWindowClosed(false);

  cumulus::MainWindow window;
  window.girderUrl(url);
  window.show();

  return app.exec();
}

void printUsage()
{
  qWarning("%s\n\n%s",
           qPrintable(QObject::tr("Usage: cumulus [options]")),
           qPrintable(QObject::tr("Options:")));

  const char *format = "      %3s %-20s   %s";

  qWarning(format, "-u,", "--url",
           qPrintable(QObject::tr("The URL for the cumulus server.")));
}
