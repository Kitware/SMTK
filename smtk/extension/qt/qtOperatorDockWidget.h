//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtOperatorDockWidget - the dockwidget for smtk model operators.
// .SECTION Description
// The main purpose of this class is to overwrite the closeEvent(), so that
// it can signal others that the operator panel is closing. The
// NOTE, QDockWidget::visibilityChanged(bool) This signal is emitted when
// the dock widget becomes visible (or invisible). This happens when the
// widget is hidden or shown, as well as when it is docked in a tabbed dock
// area and its tab becomes selected or unselected, which is not what we want.
// .SECTION Caveats

#ifndef __smtk_extension_qtOperatorDockWidget_h
#define __smtk_extension_qtOperatorDockWidget_h

#include "smtk/extension/qt/Exports.h"
#include <QDockWidget>

namespace smtk {
  namespace extension {

class SMTKQTEXT_EXPORT qtOperatorDockWidget : public QDockWidget
{
  Q_OBJECT

public:
  qtOperatorDockWidget(QWidget* p = NULL);
  ~qtOperatorDockWidget();

signals:
   void closing();

protected:
   virtual void closeEvent(QCloseEvent * event);

};

  } // namespace model
} // namespace smtk

#endif // __smtk_extension_qtOperatorDockWidget_h
