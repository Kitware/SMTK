//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extensions_vtk_source_SourceGenerator_h
#define __smtk_extensions_vtk_source_SourceGenerator_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/common/Generator.h"

#include "smtk/extension/vtk/source/Exports.h"

#include "vtkAlgorithm.h"
#include "vtkSmartPointer.h"

template class VTKSMTKSOURCEEXT_EXPORT
  smtk::common::Generator<smtk::resource::ResourcePtr, vtkSmartPointer<vtkAlgorithm> >;

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace source
{
using Generator =
  smtk::common::Generator<smtk::resource::ResourcePtr, vtkSmartPointer<vtkAlgorithm> >;
}
}
}
}

#endif
