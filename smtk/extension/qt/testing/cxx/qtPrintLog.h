//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qt_testing_cxx_qtPrintLog_h
#define __smtk_extension_qt_testing_cxx_qtPrintLog_h

#include <QObject>

#include "smtk/io/Logger.h"

#include <iostream>

class qtPrintLog : public QObject
{
  Q_OBJECT

public:
  qtPrintLog(smtk::io::Logger& log)
    : Log(log)
  {
  }

  virtual ~qtPrintLog() {}

public slots:
  void print()
  {
    std::cout << "Received: " << Log.convertToString(false) << std::endl;
    Log.reset();
  }

private:
  smtk::io::Logger& Log;
};

#endif
