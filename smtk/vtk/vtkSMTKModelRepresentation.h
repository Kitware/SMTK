#ifndef __smtk_vtk_ModelRepresentation_h
#define __smtk_vtk_ModelRepresentation_h

#include "smtk/vtkSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

#include "vtkRenderedRepresentation.h"

class vtkActor;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkTransformFilter;

/**\brief Generate a VTK dataset used to represent an SMTK model.
  *
  * This requires the model to have per-body or per-face tessellation information.
  */
class VTKSMTK_EXPORT vtkSMTKModelRepresentation : public vtkRenderedRepresentation
{
public:
  static vtkSMTKModelRepresentation* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkSMTKModelRepresentation,vtkRenderedRepresentation);

  virtual void SetModel(smtk::model::ModelBodyPtr model);
  smtk::model::ModelBodyPtr GetModel();

  virtual void Dirty();

  virtual void ApplyViewTheme(vtkViewTheme* theme);

protected:
  vtkSMTKModelRepresentation();
  virtual ~vtkSMTKModelRepresentation();

  void GenerateRepresentationFromModel(
    vtkPolyData* poly, smtk::model::ModelBodyPtr model);

  virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  virtual void PrepareForRendering(vtkRenderView* view);
  virtual bool AddToView(vtkView* view);
  virtual bool RemoveFromView(vtkView* view);
  virtual vtkSelection* ConvertSelection(vtkView* view, vtkSelection* selection);

  void SetCachedOutput(vtkPolyData*);
  vtkGetObjectMacro(CachedOutput,vtkPolyData);

  void SetTransform(vtkTransformFilter*);
  vtkGetObjectMacro(Transform,vtkTransformFilter);

  void SetMapper(vtkPolyDataMapper*);
  vtkGetObjectMacro(Mapper,vtkPolyDataMapper);

  void SetActor(vtkActor*);
  vtkGetObjectMacro(Actor,vtkActor);

  // Instance storage:
  smtk::model::ModelBodyPtr Model;
  vtkPolyData* CachedOutput;
  vtkTransformFilter* Transform;
  vtkPolyDataMapper* Mapper;
  vtkActor* Actor;

private:
  vtkSMTKModelRepresentation(const vtkSMTKModelRepresentation&); // Not implemented.
  void operator = (const vtkSMTKModelRepresentation&); // Not implemented.
};

#endif // __smtk_vtk_ModelRepresentation_h
