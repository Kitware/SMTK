//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKSelectionFilterBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKSelectionFilterBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityTypeBits.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QActionGroup>

class vtkSMSMTKWrapperProxy;
class pqServer;

/**\brief Toolbar buttons to filter how selections in 3-D renderviews are transferred to SMTK.
  *
  * This installs a filter on a given SMTK selection object.
  * When the application uses a "filtered" selection mode to modify
  * the selection, this filter will prune some items provided to the
  * modifySelection call -- optionally replacing them with suggestions
  * appropriate to the filter.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKSelectionFilterBehavior : public QActionGroup
{
  Q_OBJECT
  using Superclass = QActionGroup;

public:
  pqSMTKSelectionFilterBehavior(QObject* parent = nullptr);
  ~pqSMTKSelectionFilterBehavior() override;

  static pqSMTKSelectionFilterBehavior* instance();

  void setSelection(smtk::view::SelectionPtr selnMgr);

protected Q_SLOTS:
  virtual void onFilterChanged(QAction* a);
  virtual void startBlockSelectionInActiveView();
  virtual void filterSelectionOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);
  virtual void unfilterSelectionOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);

protected:
  /// Install a filter on the selection using current flags (m_modelFilterMask).
  void installFilter();

  class pqInternal;
  pqInternal* m_p;
  smtk::model::BitFlags m_modelFilterMask;
  smtk::view::SelectionPtr m_selection;

private:
  Q_DISABLE_COPY(pqSMTKSelectionFilterBehavior);
};

#endif
