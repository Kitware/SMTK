//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtModelPanel - the ui panel for smtk model.
// .SECTION Description
// .SECTION Caveats

#ifndef _qtModelPanel_h
#define _qtModelPanel_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtModelView.h"

#include "smtk/common/UUID.h"

#include <QWidget>

namespace smtk {
  namespace model {

class SMTKQTEXT_EXPORT qtModelPanel : public QWidget
{
  Q_OBJECT

public:
  qtModelPanel(QWidget* p = NULL);
  ~qtModelPanel();

  smtk::model::qtModelView* getModelView();

public slots:
  void onClearSelection();

signals:

protected:

private:
  class qInternal;
  qInternal* Internal;

};

  } // namespace model
} // namespace smtk

#endif // !_qtModelPanel_h
