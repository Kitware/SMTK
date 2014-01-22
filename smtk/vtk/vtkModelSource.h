#ifndef __smtk_vtk_ModelSource_h
#define __smtk_vtk_ModelSource_h

#include "smtk/vtkSMTKExports.h"
#include "vtkPolyDataAlgorithm.h"
#include "smtk/PublicPointerDefs.h"

namespace smtk {
  namespace model {

/**\brief A VTK filter that provides polydata for an SMTK storage instance.
  *
  */
class VTKSMTK_EXPORT vtkModelSource : public vtkPolyDataAlgorithm
{
public:
  static vtkModelSource* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkModelSource,vtkPolyDataAlgorithm);

  vtkGetObjectMacro(CachedOutput,vtkPolyData);

  smtk::model::StoragePtr GetModel();
  void SetModel(smtk::model::StoragePtr);

  void Dirty();

protected:
  vtkModelSource();
  virtual ~vtkModelSource();

  void GenerateRepresentationFromModel(
    vtkPolyData* poly, smtk::model::StoragePtr model);

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  //virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  void SetCachedOutput(vtkPolyData*);

  // Instance storage:
  smtk::model::StoragePtr Model;
  vtkPolyData* CachedOutput;

private:
  vtkModelSource(const vtkModelSource&); // Not implemented.
  void operator = (const vtkModelSource&); // Not implemented.
};

  } // namespace model
} // namespace smtk

#endif // __smtk_vtk_ModelSource_h
