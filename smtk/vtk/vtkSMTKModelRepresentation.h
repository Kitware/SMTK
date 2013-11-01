#ifndef __smtk_vtk_ModelRepresentation_h
#define __smtk_vtk_ModelRepresentation_h

#include "smtk/vtkSMTKExports.h"

#include "vtkRenderedRepresentation.h"

class vtkActor;
class vtkApplyColors;
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

  virtual void ApplyViewTheme(vtkViewTheme* theme);

  vtkGetObjectMacro(Transform,vtkTransformFilter);
  vtkGetObjectMacro(ApplyColors,vtkApplyColors);
  vtkGetObjectMacro(Mapper,vtkPolyDataMapper);
  vtkGetObjectMacro(Actor,vtkActor);

protected:
  vtkSMTKModelRepresentation();
  virtual ~vtkSMTKModelRepresentation();

  //virtual int FillInputPortInformation(int port, vtkInformation* request);
  //virtual int FillOutputPortInformation(int port, vtkInformation* request);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  virtual void PrepareForRendering(vtkRenderView* view);
  virtual bool AddToView(vtkView* view);
  virtual bool RemoveFromView(vtkView* view);
  virtual vtkSelection* ConvertSelection(vtkView* view, vtkSelection* selection);

  void SetTransform(vtkTransformFilter*);
  void SetApplyColors(vtkApplyColors*);
  void SetMapper(vtkPolyDataMapper*);
  void SetActor(vtkActor*);

  // Instance storage:
  vtkTransformFilter* Transform;
  vtkApplyColors* ApplyColors;
  vtkPolyDataMapper* Mapper;
  vtkActor* Actor;

private:
  vtkSMTKModelRepresentation(const vtkSMTKModelRepresentation&); // Not implemented.
  void operator = (const vtkSMTKModelRepresentation&); // Not implemented.
};

#endif // __smtk_vtk_ModelRepresentation_h
