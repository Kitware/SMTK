//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_vtkSMTKEncodeSelection_h
#define smtk_extension_paraview_appcomponents_vtkSMTKEncodeSelection_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "vtkPVEncodeSelectionForServer.h"

#include <memory>

class vtkSMTKResourceRepresentation;
class pqSMTKWrapper;
namespace smtk
{
namespace resource
{
class Resource;
}
namespace view
{
class Selection;
}
} // namespace smtk

/**\brief Intercept ParaView selections.
  *
  * ParaView's usual framework for selection notification occurs
  * after information we need is removed from the selection.
  * We override the class (vtkPVEncodeSelectionForServer) used by
  * ParaView to transform the selection by registering this class
  * with vtkObjectFactory (see pqSMTKAppComponentsAutoStart) and
  * perform our selection work at that point.
  */
class SMTKPQCOMPONENTSEXT_EXPORT vtkSMTKEncodeSelection : public vtkPVEncodeSelectionForServer
{
public:
  static vtkSMTKEncodeSelection* New();
  vtkTypeMacro(vtkSMTKEncodeSelection, vtkPVEncodeSelectionForServer);

  vtkSMTKEncodeSelection(const vtkSMTKEncodeSelection&) = delete;
  vtkSMTKEncodeSelection& operator=(const vtkSMTKEncodeSelection&) = delete;

  bool ProcessSelection(
    vtkSelection* rawSelection,
    vtkSMRenderViewProxy* viewProxy,
    bool multipleSelectionsAllowed,
    vtkCollection* selectedRepresentations,
    vtkCollection* selectionSources,
    int modifier,
    bool selectBlocks) override;

protected:
  vtkSMTKEncodeSelection();
  ~vtkSMTKEncodeSelection() override;

  /// Method called by ProcessSelection to perform SMTK-related selection.
  void ProcessRawSelection(
    vtkSelection* rawSelection,
    vtkSMRenderViewProxy* viewProxy,
    int modifier,
    bool selectBlocks);

  bool ProcessResource(
    pqSMTKWrapper* wrapper,
    const std::shared_ptr<smtk::resource::Resource>& resource,
    const std::shared_ptr<smtk::view::Selection>& smtkSelection,
    vtkSMTKResourceRepresentation* resourceRep,
    vtkSelection* rawSelection,
    vtkSMRenderViewProxy* vtkNotUsed(viewProxy),
    int modifier,
    bool selectBlocks);
};

#endif // smtk_extension_paraview_appcomponents_vtkSMTKEncodeSelection_h
