//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qt_testing_cxx_UnitTestEmittingStringBuffer_h
#define smtk_extension_qt_testing_cxx_UnitTestEmittingStringBuffer_h

#include <QObject>

class TestEmittingStringBuffer : public QObject
{
  Q_OBJECT

public:
  TestEmittingStringBuffer(QObject* parent)
    : QObject(parent)
  {
  }

Q_SIGNALS:
  void finished();

public Q_SLOTS:
  void run();
};

#endif
