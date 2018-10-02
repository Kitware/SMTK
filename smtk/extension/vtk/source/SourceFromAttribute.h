//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extensions_vtk_source_SourceFromAttribute_h
#define __smtk_extensions_vtk_source_SourceFromAttribute_h
#ifndef __VTK_WRAP__

#include "smtk/AutoInit.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/source/Exports.h"
#include "smtk/extension/vtk/source/SourceGenerator.h"

#include "vtkAlgorithm.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace source
{

/// A GeneratorType for creating vtkSmartPointer<vtkAlgorithm>s
/// using smtk::resource::Resources as the key. This class extends
/// smtk::extension::vtk::source::SourceGenerator.
class VTKSMTKSOURCEEXT_EXPORT SourceFromAttribute
  : public smtk::common::GeneratorType<smtk::resource::ResourcePtr, vtkSmartPointer<vtkAlgorithm>,
      SourceFromAttribute>
{
public:
  bool valid(const smtk::resource::ResourcePtr&) const override;

  vtkSmartPointer<vtkAlgorithm> operator()(const smtk::resource::ResourcePtr&) override;
};
}
}
}
}

#endif // __VTK_WRAP__
#endif
