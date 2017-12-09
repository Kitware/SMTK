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

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityTypeBits.h"

#include <QActionGroup>

class vtkSMSMTKResourceManagerProxy;
class pqServer;

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKSelectionFilterBehavior : public QActionGroup
{
  Q_OBJECT
  using Superclass = QActionGroup;

public:
  pqSMTKSelectionFilterBehavior(QObject* parent = nullptr);
  ~pqSMTKSelectionFilterBehavior() override;

  static pqSMTKSelectionFilterBehavior* instance();

  void setSelection(smtk::view::SelectionPtr selnMgr);

protected slots:
  virtual void onFilterChanged(QAction* a);
  virtual void filterSelectionOnServer(vtkSMSMTKResourceManagerProxy* mgr, pqServer* server);
  virtual void unfilterSelectionOnServer(vtkSMSMTKResourceManagerProxy* mgr, pqServer* server);

protected:
  /// Install a filter on the selection using current flags (m_modelFilterMask and m_acceptMeshes).
  void installFilter();

  class pqInternal;
  pqInternal* m_p;
  bool m_acceptMeshes;
  smtk::model::BitFlags m_modelFilterMask;
  smtk::view::SelectionPtr m_selection;

private:
  Q_DISABLE_COPY(pqSMTKSelectionFilterBehavior);
};

#endif
