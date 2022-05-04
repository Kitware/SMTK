//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKPipelineSelectionBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKPipelineSelectionBehavior_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityTypeBits.h"
#include "smtkPQComponentsExtModule.h"

#include "smtk/view/SelectionObserver.h"

#include "smtk/extension/paraview/appcomponents/fixWrap.h"

#include <QObject>

#include <string>

class vtkSMSMTKWrapperProxy;
class pqPipelineSource;
class pqServer;

/**\brief Keep the ParaView pipeline browser and SMTK selections in sync.
  *
  * When the SMTK selection is updated and contains resources (as opposed
  * to components), select those resources in the pipeline browser.
  * Similarly, when pipeline sources in ParaView are selected, replace
  * the SMTK selection with the related resources.
  *
  * This behavior can also be configured to update the attribute editor
  * panel (pqSMTKAttributePanel) when the selection is set to an
  * attribute resource. (The default behavior is to show the resource
  * when it is selected.)
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKPipelineSelectionBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKPipelineSelectionBehavior(QObject* parent = nullptr);
  ~pqSMTKPipelineSelectionBehavior() override;

  /// This behavior is a singleton.
  static pqSMTKPipelineSelectionBehavior* instance(QObject* parent = nullptr);

  /// Set which integer bit(s) to modify in the SMTK selection when a pipeline source is selected.
  void setSelectionValue(const std::string& selectionValue);
  const std::string& selectionValue() const { return m_selectionValue; }

  /// Return whether selecting an attribute resource should cause that
  /// resource to appear in the attribute-editor panel (assuming it is not marked private).
  bool displayAttributeResourcesOnSelection() const
  {
    return m_displayAttributeResourcesOnSelection;
  }
public Q_SLOTS:
  /// Set whether selecting an attribute resource should cause that
  /// resource to appear in the attribute-editor panel (assuming it is not marked private).
  ///
  /// This is true by default.
  virtual void setDisplayAttributeResourcesOnSelection(bool shouldDisplay);

protected Q_SLOTS:
  virtual void onActiveSourceChanged(pqPipelineSource* source);
  virtual void observeSelectionOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);
  virtual void unobserveSelectionOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);

protected:
  bool m_changingSource{ false };
  bool m_displayAttributeResourcesOnSelection{ true };
  std::string m_selectionValue;
  std::map<smtk::view::SelectionPtr, smtk::view::SelectionObservers::Key> m_selectionObservers;

private:
  Q_DISABLE_COPY(pqSMTKPipelineSelectionBehavior);
};

#endif
