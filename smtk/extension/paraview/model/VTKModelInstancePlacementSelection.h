//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_VTKModelInstancePlacementSelection_h
#define smtk_view_VTKModelInstancePlacementSelection_h
#ifndef __VTK_WRAP__

#include "smtk/extension/paraview/model/smtkPVModelExtModule.h" // For export macro

#include "smtk/extension/paraview/server/RespondToVTKSelection.h"

class vtkIdTypeArray;

namespace smtk
{
namespace view
{

/**\brief An operation that handles selection of individual
  *       instance placements on model resources.
  *
  * A single smtk::model::Instance may have many placements
  * of its prototype. Users may wish to select a subset of
  * them for manipulation.
  *
  * This operation, invoked with a vtkSelection dataset, creates a
  * new instance and adds it to the SMTK selection.
  */
class SMTKPVMODELEXT_EXPORT VTKModelInstancePlacementSelection
  : public smtk::view::RespondToVTKSelection
{
public:
  using Result = smtk::operation::Operation::Result;
  smtkTypeMacro(VTKModelInstancePlacementSelection);
  smtkCreateMacro(VTKModelInstancePlacementSelection);
  smtkSharedFromThisMacro(smtk::view::RespondToVTKSelection);
  smtkSuperclassMacro(smtk::view::RespondToVTKSelection);
  virtual ~VTKModelInstancePlacementSelection();

protected:
  VTKModelInstancePlacementSelection();

  /**\brief Create an ephemeral instance that is a copy of some
    *       placements from a source instance.
    *
    * The returned instance will also have a deletion operation
    * added to the resource manager's garbage collector, so that
    * it will be deleted when no longer held by more than its
    * owning resource. See the garbage collector documentation for
    * more information about the instance's lifecycle.
    */
  smtk::model::EntityPtr temporaryInstance(
    const smtk::model::EntityPtr& sourceInstance, vtkIdTypeArray* sourcePlacements);

  /**\brief A convenience method that subclasses may use internally
    *       to handle VTK index selections.
    *
    * This will create instances and select them to match points/cells
    * selected in VTK. The newly-created instances will be added to
    * the result so that the UI can be updated to show them.
    *
    * Note that the selection is filtered.
    */
  bool transcribePlacementSelection(Result& result);

  /// Simply call transcribeCellIdSelection().
  Result operateInternal() override;

private:
  const char* xmlDescription() const override;
};

} //namespace view
} // namespace smtk

#endif
#endif // smtk_view_VTKModelInstancePlacementSelection_h
