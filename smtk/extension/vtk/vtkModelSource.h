//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_vtk_ModelSource_h
#define __smtk_vtk_ModelSource_h

#include "smtk/extension/vtk/vtkSMTKExports.h"

#include "smtk/model/Cursor.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkPolyDataAlgorithm.h"

/**\brief A VTK filter that provides polydata for an SMTK model manager instance.
  *
  */
class VTKSMTK_EXPORT vtkModelSource : public vtkPolyDataAlgorithm
{
public:
  static vtkModelSource* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkModelSource,vtkPolyDataAlgorithm);

  vtkGetObjectMacro(CachedOutput,vtkPolyData);

  smtk::model::Cursors GetEntities();
  void SetEntities(const smtk::model::Cursors&);

  void Dirty();

protected:
  vtkModelSource();
  virtual ~vtkModelSource();

  struct SortByDim {
    bool operator () (const smtk::model::Cursor& a, const smtk::model::Cursor& b)
      {
      return
        (a.dimension() < b.dimension()) ||
        (a.dimension() == b.dimension() && a < b) ?
        true : false;
      }
  };
  typedef std::set<smtk::model::Cursor,SortByDim> CursorsByDim;

  void AccumulateSortedEntities(
    CursorsByDim& accum, vtkIdType& npts, smtk::model::Cursors& toplevel);
  void GenerateRepresentationFromModel(
    vtkPolyData* poly, const CursorsByDim& model, vtkIdType npts);

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  //virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  void SetCachedOutput(vtkPolyData*);

  // Instance storage:
  smtk::model::Cursors Entities;
  vtkPolyData* CachedOutput;

private:
  vtkModelSource(const vtkModelSource&); // Not implemented.
  void operator = (const vtkModelSource&); // Not implemented.
};

#endif // __smtk_vtk_ModelSource_h
