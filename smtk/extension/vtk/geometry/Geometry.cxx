//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/geometry/Geometry.h"

#include "smtk/io/Logger.h"

#include "vtkDataObject.h"
#include "vtkFieldData.h"
#include "vtkUnsignedCharArray.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{

void Geometry::addColorArray(
  vtkDataObject* data,
  const std::vector<double>& rgba,
  const std::string& arrayName)
{
  if (rgba.size() != 4 || rgba[3] <= 0.0)
  {
    return;
  }
  if (!data)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Bad object passed to addColorArray.");
    return;
  }

  vtkNew<vtkUnsignedCharArray> colorArray;
  colorArray->SetNumberOfComponents(4);
  colorArray->SetNumberOfTuples(1);
  colorArray->SetName(arrayName.c_str());
  for (int i = 0; i < 4; ++i)
  {
    colorArray->FillComponent(i, rgba[i]);
  }
  data->GetFieldData()->AddArray(colorArray.GetPointer());
}

} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk
