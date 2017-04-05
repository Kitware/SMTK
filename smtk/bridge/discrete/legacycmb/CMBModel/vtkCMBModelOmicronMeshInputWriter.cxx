//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBModelOmicronMeshInputWriter.h"

#include "vtkCompositeDataIterator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkModelItemIterator.h"
#include "vtkModelMaterial.h"
#include "vtkModelUserName.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelOmicronMeshInputWriter);

vtkCMBModelOmicronMeshInputWriter::vtkCMBModelOmicronMeshInputWriter()
{
}

vtkCMBModelOmicronMeshInputWriter:: ~vtkCMBModelOmicronMeshInputWriter()
{
}

bool vtkCMBModelOmicronMeshInputWriter::Write(
  vtkDiscreteModel* model, vtkCMBModelOmicronMeshInputWriterBase* base)
{
  ofstream buffer(base->GetFileName());
  if(!buffer)
    {
    vtkErrorMacro("Could not open file " << base->GetFileName());
    return 0;
    }
  buffer << base->GetGeometryFileName() << "\n";
  buffer << "tetgen_options: " << base->GetTetGenOptions() << "\n";
  bool retcode = this->Write(model, buffer);
  buffer.close();
  return retcode;
}

bool vtkCMBModelOmicronMeshInputWriter::Write(
  vtkDiscreteModel* model, std::ostream& buffer)
{
  if (!model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }

  buffer << "number_of_regions: "
       << model->GetNumberOfModelEntities(vtkModelRegionType) << "\n";

  buffer.precision(16);
  vtkModelItemIterator* regions = model->NewIterator(vtkModelRegionType);
  for(regions->Begin();!regions->IsAtEnd();regions->Next())
    {
    vtkDiscreteModelRegion* region =
      vtkDiscreteModelRegion::SafeDownCast(regions->GetCurrentItem());
    const char* regionName = vtkModelUserName::GetUserName(region);
    buffer << "(Object filename, Material ID): \"" << regionName
         << "\" " << region->GetUniquePersistentId() << "\n";

    if(double *pointInside = region->GetPointInside())
      {
      buffer << "(point inside object): " << pointInside[0]
           << " " << pointInside[1] << " " << pointInside[2] << "\n";
      }
    else
      {
      vtkErrorMacro("Missing required PointInside info!");
      return 0;
      }

    }
  regions->Delete();

  return 1;
}

void vtkCMBModelOmicronMeshInputWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
