//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBParserBase -
// .SECTION Description
// Abstract base class for parsing a vtkPolyData to create a vtkDiscreteModel.

#ifndef __smtkdiscrete_vtkCMBParserBase_h
#define __smtkdiscrete_vtkCMBParserBase_h

#include "smtk/bridge/discrete/discreteSessionExports.h" // For export macro
#include "vtkObject.h"
#include "vtkSmartPointer.h" //needed for classification
#include <map> //needed for classification

class vtkCharArray;
class vtkDiscreteModel;
class vtkDiscreteModelGeometricEntity;
class vtkDataArray;
class vtkIdList;
class vtkIdTypeArray;
class vtkModel;
class vtkModelEntity;
class vtkPolyData;

class SMTKDISCRETESESSION_EXPORT vtkCMBParserBase : public vtkObject
{
public:
  vtkTypeMacro(vtkCMBParserBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual bool Parse(vtkPolyData* MasterPoly, vtkDiscreteModel* Model) = 0;

  // Description:
  // Function to set the geometry of the model so that every parser
  // does not need to be specified as a friend class of vtkDiscreteModel.
  void SetGeometry(vtkDiscreteModel* Model, vtkObject* Geometry);

  // Description:
  // Function to set the cells of the model geometric entity so that
  // every parser does not need to be specified as a friend class of vtkDiscreteModel.
  bool AddCellsToGeometry(vtkDiscreteModelGeometricEntity* Entity,
                           vtkIdList* MasterCellIds);

  // Description:
  // Function to set the unique persistent Id of an entity so that every parser
  // does not need to be specified as a friend class of vtkModelEntity.
  void SetUniquePersistentId(vtkModelEntity* Entity, vtkIdType Id);

  // Description:
  // Function to set the MaxId of all entities in the model so that every parser
  // does not need to be specified as a friend class of vtkModel.
  // It checks that MaxId is larger than the current value.
  void SetLargestUsedUniqueId(vtkModel* Model, vtkIdType MaxId);

  // Description:
  // Function to output vtkIdTypeArray given a data array.  This is needed
  // since the array may be read in as an vtkIntArray (they are essentially the
  // same on 32 bit machines) but cannot be cast to a vtkIdTypeArray.
  // If Array is not a vtkIdTypeArray, it will allocate a new array
  // which the caller must delete.  Otherwise it will increase the reference
  // count to the original array such that the caller must delete the array as well.
  vtkIdTypeArray* NewIdTypeArray(vtkDataArray* Array);

  // Description:
  // Function to set the information for mapping from the model grid to the
  // analysis grid.  It assumes the boundary topology is the same and that we
  // have the full mapping information.  We are
  // unsure what type of array we are getting from the reader
  // so we will convert it internally to a vtkIdTypeArray.
  void SetAnalysisGridInfo(
    vtkDiscreteModel* model, vtkDataArray* pointMapArray, vtkDataArray* cellMapArray,
    vtkCharArray* canonicalSideArray);

protected:
  vtkCMBParserBase();
  virtual ~vtkCMBParserBase();

  //Description:
  // Convert a flat cell Id space where some cells represent edges
  // or faces, and return a mapping from model item id to cell or edge ids
  typedef std::map<vtkIdType, vtkSmartPointer<vtkIdList> > CellToModelType;
  typedef CellToModelType::iterator CellToModelIterator;
  void SeparateCellClassification(vtkDiscreteModel* model,
                                  vtkIdTypeArray* cellClassification,
                                   vtkCMBParserBase::CellToModelType& cellToModelMap) const;
private:
  vtkCMBParserBase(const vtkCMBParserBase&);  // Not implemented.
  void operator=(const vtkCMBParserBase&);  // Not implemented.
};

#endif
