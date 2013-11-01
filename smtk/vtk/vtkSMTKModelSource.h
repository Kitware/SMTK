#ifndef __smtk_vtk_ModelSource_h
#define __smtk_vtk_ModelSource_h

#include "smtk/vtkSMTKExports.h"
#include "vtkPolyDataAlgorithm.h"
#include "smtk/PublicPointerDefs.h"

class VTKSMTK_EXPORT vtkSMTKModelSource : public vtkPolyDataAlgorithm
{
public:
  static vtkSMTKModelSource* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkSMTKModelSource,vtkPolyDataAlgorithm);

  vtkGetObjectMacro(CachedOutput,vtkPolyData);

  smtk::model::ModelBodyPtr GetModel();
  void SetModel(smtk::model::ModelBodyPtr);

  void Dirty();

protected:
  vtkSMTKModelSource();
  virtual ~vtkSMTKModelSource();

  void GenerateRepresentationFromModel(
    vtkPolyData* poly, smtk::model::ModelBodyPtr model);

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  //virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  void SetCachedOutput(vtkPolyData*);

  // Instance storage:
  smtk::model::ModelBodyPtr Model;
  vtkPolyData* CachedOutput;

private:
  vtkSMTKModelSource(const vtkSMTKModelSource&); // Not implemented.
  void operator = (const vtkSMTKModelSource&); // Not implemented.
};

#endif // __smtk_vtk_ModelSource_h
