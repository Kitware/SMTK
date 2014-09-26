/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
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
// .NAME vtkCMBModelWriterV2 - Writes out a CMB version 2.0 file.
// .SECTION Description
// Writes out a CMB version 2.0 file.  It currently gets written
// out as a vtkPolyData using vtkXMLPolyDataWriter with the necessary
// information included in the field data.
// The format for storing the model entity Ids for each model entity group
// is not that straightforward since every model entity group could potentially
// be grouping a different amount of model entities.  The way that I chose to do it
// is to have a single index flat array that stores all of the needed information.
// The format of the data in the array is:
// number of model entities in first model entity group
// the Ids of the model entities in the first group
// number of model entities in second model entity group
// the Ids of the model entities in the second group
// ...
// number of model entities in last model entity group
// the Ids of the model entities in the last group

#ifndef __smtkcmb_vtkCMBModelWriterV2_h
#define __smtkcmb_vtkCMBModelWriterV2_h

#include "vtkSMTKCMBModelModule" // For export macro
#include "vtkObject.h"
#include <vector>


class vtkDiscreteModel;
class vtkCMBModelWriterBase;
class vtkModelEntity;
class vtkModelItem;
class vtkPolyData;

class VTKSMTKCMBMODEL_EXPORT vtkCMBModelWriterV2 : public vtkObject
{
public:
  static vtkCMBModelWriterV2 * New();
  vtkTypeMacro(vtkCMBModelWriterV2,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write the CMB file out.
  virtual bool Write(vtkDiscreteModel* Model);

  // Description:
  // Get/Set the data mode used for the file's data.  The options are
  // vtkXMLWriter::Ascii, vtkXMLWriter::Binary, and
  // vtkXMLWriter::Appended.
  vtkSetMacro(DataMode, int);
  vtkGetMacro(DataMode, int);
  void SetDataModeToAscii();
  void SetDataModeToBinary();
  void SetDataModeToAppended();

  // Description:
  // Get/Set the name of the output file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Return the version being written out.
  virtual int GetVersion()
  {return 2;}

protected:
  vtkCMBModelWriterV2();
  virtual ~vtkCMBModelWriterV2();

  // Description:
  // Set the vtkDiscreteModelVertex data in Poly.  Version 2 does not
  // contain this info.
  virtual void SetModelVertexData(vtkDiscreteModel* /*Model*/, vtkPolyData* /*Poly*/) {};

  // Description:
  // Set the vtkDiscreteModelEdge data in Poly.  Version 2 does not
  // contain this data.
  virtual void SetModelEdgeData(vtkDiscreteModel* /*Model*/, vtkPolyData* /*Poly*/) {};

  // Description:
  // Set the information for the analysis grid with respect to the
  // model grid data in poly.  Version 4 is the first to support this.
  virtual void SetAnalysisGridData(vtkDiscreteModel* model, vtkPolyData* poly);

  // Description:
  // Set the vtkDiscreteModelFace data in Poly.
  virtual void SetModelFaceData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Set the vtkCMBModelRegion data in Poly.
  virtual void SetModelRegionData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Set the vtkCMBMaterial data in Poly.
  virtual void SetMaterialData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Set the vtkModelEntityGroup data in Poly.
  virtual void SetModelEntityGroupData(vtkDiscreteModel* Model, vtkPolyData* Poly);

 // Description:
  // Set the vtkDiscreteModelEdge data in Poly.
  virtual void SetFloatingEdgeData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Add arrays to the master \a Poly data containing UUIDs for model entities.
  virtual void SetUUIDData(vtkDiscreteModel* Model, vtkPolyData* Poly);

  // Description:
  // Set the vtkCMBModelFaceData in Poly.
  virtual void SetModelEntityData(vtkPolyData* Poly,
                                  std::vector<vtkModelEntity*> & EntityIds,
                                  const char* BaseArrayName);
  // Description:
  // Add a field-data array (named \a arrayName) to \a poly
  // containing UUIDs of all the \a items.
  //
  // Any entry in \a items with no UUID will result in a
  // null UUID in the output array.
  virtual void SetModelItemUUIDs(
    vtkDiscreteModel* model, vtkPolyData* poly,
    std::vector<vtkModelItem*>& items,
    const char* arrayName);

  // Description:
  // Add the file version to the field data of the poly data
  virtual void AddFileVersion(vtkPolyData *poly);

private:
  vtkCMBModelWriterV2(const vtkCMBModelWriterV2&);  // Not implemented.
  void operator=(const vtkCMBModelWriterV2&);  // Not implemented.

  // Description:
  // The mode to use for the internal vtkXMLPolyDataWriter
  int DataMode;

  // Description:
  // The name of the file to be written.
  char* FileName;

};

#endif
