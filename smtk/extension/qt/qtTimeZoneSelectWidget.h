//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtTimeZoneSelectWidget - ui panel for selecting timezone
// .SECTION Description
// .SECTION Caveats

#ifndef __smtk_extension_qtTimeZoneSelectWidget_h
#define __smtk_extension_qtTimeZoneSelectWidget_h

#include "smtk/extension/qt/Exports.h"
#include <QWidget>
class Ui_qtTimeZoneSelectWidget;

namespace smtk {
  namespace extension {

class SMTKQTEXT_EXPORT qtTimeZoneSelectWidget : public QWidget
{
  Q_OBJECT

 public:
  qtTimeZoneSelectWidget(QWidget* parent = NULL);
  ~qtTimeZoneSelectWidget();

 public slots:

 signals:

 protected slots:

 protected:
  Ui_qtTimeZoneSelectWidget *UI;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_extension_qtTimeZoneSelectWidget_h
