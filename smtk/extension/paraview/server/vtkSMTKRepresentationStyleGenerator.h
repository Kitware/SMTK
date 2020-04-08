//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_representation_vtkSMTKRepresentationStyleGenerator_h
#define smtk_extension_paraview_representation_vtkSMTKRepresentationStyleGenerator_h

#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"

#include "smtk/common/Generator.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"

#include <memory>
#include <tuple>

namespace smtk
{
namespace resource
{
class Resource;
}
}

#ifndef smtkPVServerExt_EXPORTS
extern
#endif
  template class SMTKPVSERVEREXT_EXPORT smtk::common::Generator<smtk::resource::ResourcePtr,
    vtkSMTKResourceRepresentation::StyleFromSelectionFunction>;

/// Declare the class used to _generate_ a style function.
///
/// The vtkSMTKResourceRepresentation class uses this class to create instances of
/// a function to style the selection.
using vtkSMTKRepresentationStyleGenerator = smtk::common::Generator<smtk::resource::ResourcePtr,
  vtkSMTKResourceRepresentation::StyleFromSelectionFunction>;

/// Declare the class used to _register_ a style function.
///
/// Plugins create objects of this type to register a new style function.
template <typename T>
class vtkSMTKRepresentationStyleSupplier
  : public smtk::common::GeneratorType<smtk::resource::ResourcePtr,
      vtkSMTKResourceRepresentation::StyleFromSelectionFunction, T>
{
public:
  using StyleFromSelectionFunction = vtkSMTKResourceRepresentation::StyleFromSelectionFunction;
};

#endif
