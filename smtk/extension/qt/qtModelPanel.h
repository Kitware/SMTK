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

#ifndef __smtk_extension_qtModelPanel_h
#define __smtk_extension_qtModelPanel_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtModelView.h"

#include "smtk/PublicPointerDefs.h"

#include <QWidget>

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtModelPanel : public QWidget
{
  Q_OBJECT

public:
  qtModelPanel(QWidget* p = NULL);
  ~qtModelPanel();

  qtModelView* getModelView();

  enum enumTreeView
  {
    VIEW_BY_TOPOLOGY = 0,
    VIEW_BY_ENTITY_LIST
  };

public slots:
  void onClearSelection();
  void onViewTypeChanged();
  void resetView(qtModelPanel::enumTreeView enType, smtk::model::ManagerPtr modelMgr);

private:
  class qInternal;
  qInternal* Internal;
};

} // namespace model
} // namespace smtk

#endif // __smtk_extension_qtModelPanel_h
