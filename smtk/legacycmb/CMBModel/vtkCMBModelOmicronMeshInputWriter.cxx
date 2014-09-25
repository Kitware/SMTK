/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkCMBModelOmicronMeshInputWriter.h"

#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkModelUserName.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkModelItemIterator.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelOmicronMeshInputWriter);

//----------------------------------------------------------------------------
vtkCMBModelOmicronMeshInputWriter::vtkCMBModelOmicronMeshInputWriter()
{
}

//----------------------------------------------------------------------------
vtkCMBModelOmicronMeshInputWriter:: ~vtkCMBModelOmicronMeshInputWriter()
{
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
void vtkCMBModelOmicronMeshInputWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
