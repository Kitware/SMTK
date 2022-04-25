//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKResourceRepresentation_h
#define smtk_extension_paraview_appcomponents_pqSMTKResourceRepresentation_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "pqPipelineRepresentation.h"

#include "smtk/view/SelectionObserver.h"

#include "smtk/PublicPointerDefs.h"

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS protected
#define Q_OBJECT
#endif

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResourceRepresentation : public pqPipelineRepresentation
{
  Q_OBJECT
  typedef pqPipelineRepresentation Superclass;

public:
  pqSMTKResourceRepresentation(
    const QString& group,
    const QString& name,
    vtkSMProxy* repr,
    pqServer* server,
    QObject* parent = nullptr);
  ~pqSMTKResourceRepresentation() override;

  void onInputChanged() override;

  /// Change the visibility of the specified component. Returns true if changed, false otherwise.
  bool setVisibility(smtk::resource::ComponentPtr comp, bool visible);

Q_SIGNALS:
  /// Emitted from within setVisibility().
  void componentVisibilityChanged(smtk::resource::ComponentPtr comp, bool visible);

protected Q_SLOTS:
  /// Called when vtkSMTKSettings singleton is modified.
  ///
  /// Currently, this will update selection render-style (wireframe/solid).
  void updateSettings();

protected:
  virtual void handleSMTKSelectionChange(const std::string& src, smtk::view::SelectionPtr seln);

  smtk::view::WeakSelectionPtr m_seln;
  smtk::view::SelectionObservers::Key m_selnObserver;
};

#endif
