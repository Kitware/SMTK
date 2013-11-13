#ifndef __smtk_vtk_ModelMultiBlockSource_h
#define __smtk_vtk_ModelMultiBlockSource_h

#include "smtk/vtkSMTKExports.h"
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "smtk/PublicPointerDefs.h"

class vtkPolyData;

class VTKSMTK_EXPORT vtkSMTKModelMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkSMTKModelMultiBlockSource* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkSMTKModelMultiBlockSource,vtkMultiBlockDataSetAlgorithm);

  vtkGetObjectMacro(CachedOutput,vtkMultiBlockDataSet);

  smtk::model::StoragePtr GetModel();
  void SetModel(smtk::model::StoragePtr);

  void Dirty();

protected:
  vtkSMTKModelMultiBlockSource();
  virtual ~vtkSMTKModelMultiBlockSource();

  void GenerateRepresentationFromModelEntity(
    vtkPolyData* poly, smtk::model::StoragePtr model, const smtk::util::UUID& uid);
  void GenerateRepresentationFromModel(
    vtkMultiBlockDataSet* mbds, smtk::model::StoragePtr model);

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  //virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  void SetCachedOutput(vtkMultiBlockDataSet*);

  // Instance storage:
  smtk::model::StoragePtr Model;
  vtkMultiBlockDataSet* CachedOutput;

private:
  vtkSMTKModelMultiBlockSource(const vtkSMTKModelMultiBlockSource&); // Not implemented.
  void operator = (const vtkSMTKModelMultiBlockSource&); // Not implemented.
};

#endif // __smtk_vtk_ModelMultiBlockSource_h
