//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_VTKMeshCellSelection_h
#define smtk_view_VTKMeshCellSelection_h
#ifndef __VTK_WRAP__

#include "smtk/extension/paraview/server/RespondToVTKSelection.h"

namespace smtk
{
namespace view
{

/**\brief An operation that handles cell selections on mesh resources.
  *
  * This operation, invoked with a vtkSelection dataset, creates a
  * new meshset and adds it to the SMTK selection when VTK cell indices
  * are provided.
  */
class SMTKPVSERVEREXT_EXPORT VTKMeshCellSelection : public smtk::view::RespondToVTKSelection
{
public:
  using Result = smtk::operation::Operation::Result;
  smtkTypeMacro(VTKMeshCellSelection);
  smtkCreateMacro(VTKMeshCellSelection);
  smtkSharedFromThisMacro(smtk::view::RespondToVTKSelection);
  smtkSuperclassMacro(smtk::view::RespondToVTKSelection);
  virtual ~VTKMeshCellSelection();

protected:
  VTKMeshCellSelection();

  /**\brief A convenience method that subclasses may use internally
    *       to handle VTK index selections.
    *
    * This will create meshsets and select them to match cells
    * selected in VTK.
    *
    * Note that the selection is filtered.
    */
  bool transcribeCellIdSelection(Result& result);

  /// Simply call transcribeCellIdSelection().
  Result operateInternal() override;

private:
  const char* xmlDescription() const override;
};

} //namespace view
} // namespace smtk

#endif
#endif // smtk_view_VTKMeshCellSelection_h
