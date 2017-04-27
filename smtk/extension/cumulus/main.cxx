//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <QDir>
#include <QSettings>
#include <QStringList>

#include <QApplication>
#include <QMessageBox>

#include <QSslSocket>

#include "mainwindow.h"

#include <cstdio>

void printUsage();

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  QString url;

  QStringList args = QCoreApplication::arguments();

  if (args.length() != 3)
  {
    printUsage();
    return EXIT_FAILURE;
  }

  for (QStringList::const_iterator it = args.constBegin() + 1, itEnd = args.constEnd(); it != itEnd;
       ++it)
  {
    if (*it == "-u" || *it == "--url")
    {
      url = *(++it);
      break;
    }
    else if (*it == "-h" || *it == "-H" || *it == "--help" || *it == "-help")
    {
      printUsage();
      return EXIT_SUCCESS;
    }
    else
    {
      qWarning(qPrintable(QObject::tr("Unrecognized command line option: %s")), qPrintable(*it));
      printUsage();
      return EXIT_FAILURE;
    }
  }

  if (url.isEmpty())
  {
    printUsage();
    return EXIT_FAILURE;
  }

  if (!QSslSocket::supportsSsl())
  {
    QMessageBox::critical(NULL, QObject::tr("SSL support"),
      QObject::tr("SSL support is required, please ensure Qt has been compiled with SSL support."));
    return EXIT_FAILURE;
  }

  cumulus::MainWindow window;
  window.girderUrl(url);
  window.show();

  return app.exec();
}

void printUsage()
{
  qWarning("%s\n\n%s", qPrintable(QObject::tr("Usage: cumulus [options]")),
    qPrintable(QObject::tr("Options:")));

  const char* format = "      %3s %-20s   %s";

  qWarning(format, "-u,", "--url", qPrintable(QObject::tr("The URL for the cumulus server.")));
}
