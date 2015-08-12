//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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

#ifndef __smtkdiscrete_vtkCMBModelWriterV2_h
#define __smtkdiscrete_vtkCMBModelWriterV2_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"
#include <vector>

namespace smtk {
  namespace bridge {
    namespace discrete {
      class Session;
    }
  }
}

class vtkDiscreteModel;
class vtkCMBModelWriterBase;
class vtkModelEntity;
class vtkModelItem;
class vtkPolyData;

class SMTKDISCRETESESSION_EXPORT vtkCMBModelWriterV2 : public vtkObject
{
public:
  static vtkCMBModelWriterV2 * New();
  vtkTypeMacro(vtkCMBModelWriterV2,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Write the CMB file out.
  virtual bool Write(vtkDiscreteModel* Model, smtk::bridge::discrete::Session* session);

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
  virtual void SetUUIDData(vtkDiscreteModel* Model, vtkPolyData* Poly,
                           smtk::bridge::discrete::Session* session);

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
    const char* arrayName,
    smtk::bridge::discrete::Session* session);

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
