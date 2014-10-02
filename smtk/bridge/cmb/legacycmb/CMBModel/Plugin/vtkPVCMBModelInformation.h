//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkPVCMBModelInformation - Light object for holding information
// about a model face object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVCMBModelInformation_h
#define __vtkPVCMBModelInformation_h

#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
#include <string>
#include <map>

class vtkTransform;
class vtkIntArray;

class VTK_EXPORT vtkPVCMBModelInformation : public vtkPVInformation
{
public:
  static vtkPVCMBModelInformation* New();
  vtkTypeMacro(vtkPVCMBModelInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  virtual void AddInformation(vtkPVInformation* info);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);

  vtkGetObjectMacro(Transform, vtkTransform);
  vtkGetVector3Macro(Translation, double);
  vtkGetVector3Macro(Orientation, double);
  vtkGetMacro(Scale, double);
  vtkGetVector3Macro(Color, double);
  vtkGetMacro(NumberOfPoints, int);
  vtkGetMacro(NumberOfCells, int);
  const char *GetObjectType() { return this->ObjectType.c_str(); }

  vtkGetObjectMacro(ModelFaceInfoArray, vtkIntArray);
  vtkGetObjectMacro(SplitModelFaces, vtkIntArray);

  virtual int GetModelFaceId();
  virtual int GetShellId();
  virtual int GetMaterialId();
  virtual int GetInfoArrayBCStartIndex();
  virtual int GetMasterCellId(int idx);
  virtual vtkIdType GetModelEntityId(unsigned int flatidx);

  //BTX
protected:
  vtkPVCMBModelInformation();
  ~vtkPVCMBModelInformation();

  // Data information collected from remote processes.
  vtkTransform  *Transform;
  double         Translation[3];
  double         Orientation[3];
  double         Scale;
  double         Color[3];
  int            NumberOfPoints;
  int            NumberOfCells;

  std::string ObjectType;

  vtkIntArray    *ModelFaceInfoArray;
  vtkIntArray    *SplitModelFaces;
  vtkIntArray    *CellIdMapArray;

  std::map<int, vtkIdType> EnityIdsMap;

private:

  vtkPVCMBModelInformation(const vtkPVCMBModelInformation&); // Not implemented
  void operator=(const vtkPVCMBModelInformation&); // Not implemented
  //ETX
};

#endif
