//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qt_testing_cxx_qtEventFilter_h
#define __smtk_extension_qt_testing_cxx_qtEventFilter_h

#include <QObject>

class qtEventFilter : public QObject
{
  Q_OBJECT

public:
  qtEventFilter(QObject* parent = nullptr);
  virtual ~qtEventFilter();

signals:
  void reset();
  void toggleItem();
  void toggleLink();
  void popDown();

protected:
  bool eventFilter(QObject* src, QEvent* event);
};

#endif
