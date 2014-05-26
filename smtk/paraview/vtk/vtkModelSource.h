#ifndef __smtk_vtk_ModelSource_h
#define __smtk_vtk_ModelSource_h

#include "smtk/vtkSMTKExports.h"
#include "vtkPolyDataAlgorithm.h"
#include "smtk/PublicPointerDefs.h"


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

  smtk::model::ManagerPtr GetModel();
  void SetModel(smtk::model::ManagerPtr);

  void Dirty();

protected:
  vtkModelSource();
  virtual ~vtkModelSource();

  void GenerateRepresentationFromModel(
    vtkPolyData* poly, smtk::model::ManagerPtr model);

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  //virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  void SetCachedOutput(vtkPolyData*);

  // Instance storage:
  smtk::model::ManagerPtr Model;
  vtkPolyData* CachedOutput;

private:
  vtkModelSource(const vtkModelSource&); // Not implemented.
  void operator = (const vtkModelSource&); // Not implemented.
};

#endif // __smtk_vtk_ModelSource_h
