//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKModelRepresentation_h
#define smtk_extension_paraview_appcomponents_pqSMTKModelRepresentation_h

#include "pqPipelineRepresentation.h"

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/PublicPointerDefs.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKModelRepresentation : public pqPipelineRepresentation
{
  Q_OBJECT
  typedef pqPipelineRepresentation Superclass;

public:
  pqSMTKModelRepresentation(const QString& group, const QString& name, vtkSMProxy* repr,
    pqServer* server, QObject* parent = nullptr);
  ~pqSMTKModelRepresentation() override;

  void onInputChanged() override;

  /// Change the visibility of the specified component. Returns true if changed, false otherwise.
  bool setVisibility(smtk::resource::ComponentPtr comp, bool visible);

signals:
  /// Emitted from within setVisibility().
  void componentVisibilityChanged(smtk::resource::ComponentPtr comp, bool visible);

protected:
  virtual void handleSMTKSelectionChange(const std::string& src, smtk::view::SelectionPtr seln);

  void initialize() override;

  smtk::view::WeakSelectionPtr m_seln;
  int m_selnObserver;
};

#endif
