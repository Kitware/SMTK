//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_representation_pqSMTKModelRepresentation_h
#define smtk_extension_paraview_representation_pqSMTKModelRepresentation_h

#include "pqPipelineRepresentation.h"

#include "smtk/extension/paraview/representation/Exports.h"

#include "smtk/PublicPointerDefs.h"

class SMTKREPRESENTATIONPLUGIN_EXPORT pqSMTKModelRepresentation : public pqPipelineRepresentation
{
  Q_OBJECT
  typedef pqPipelineRepresentation Superclass;

public:
  pqSMTKModelRepresentation(const QString& group, const QString& name, vtkSMProxy* repr,
    pqServer* server, QObject* parent = nullptr);
  ~pqSMTKModelRepresentation() override;

protected:
  virtual void handleSMTKSelectionChange(
    const std::string& src, smtk::resource::SelectionManagerPtr seln);
};

#endif
