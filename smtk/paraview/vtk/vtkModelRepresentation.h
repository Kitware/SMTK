#ifndef __smtk_vtk_ModelRepresentation_h
#define __smtk_vtk_ModelRepresentation_h

#include "smtk/vtkSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

#include "vtkRenderedRepresentation.h"

class vtkActor;
class vtkApplyColors;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkTransformFilter;

/**\brief Generate a VTK pipeline to represent an SMTK model.
  *
  * This representation only works with vtkModelSource, not vtkModelMultiBlockSource.
  * It requires the model to have per-body or per-face tessellation information.
  */
class VTKSMTK_EXPORT vtkModelRepresentation : public vtkRenderedRepresentation
{
public:
  static vtkModelRepresentation* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkModelRepresentation,vtkRenderedRepresentation);

  /**\brief Get/set the selection mask.
    *
    * The selection mask is bit-wise ANDed with each selected entity's flags
    * before that entity is added to the pedigree ID selection output by
    * ConvertSelection().
    *
    * This can be used to force only edges, faces, or vertices to be selected.
    */
  //@{
  vtkGetMacro(SelectionMask,int);
  vtkSetMacro(SelectionMask,int);
  //@}

  /// Set/get the model (used for selection masking). This is really a hack.
  //@{
  virtual void SetModel(smtk::model::ManagerPtr model)
    {
    if (this->Model == model)
      return;
    this->Model = model;
    this->Modified();
    }
  smtk::model::ManagerPtr GetModel()
    { return this->Model; }
  //@}

  virtual void ApplyViewTheme(vtkViewTheme* theme);

  vtkGetObjectMacro(Transform,vtkTransformFilter);
  vtkGetObjectMacro(ApplyColors,vtkApplyColors);
  vtkGetObjectMacro(Mapper,vtkPolyDataMapper);
  vtkGetObjectMacro(Actor,vtkActor);

protected:
  vtkModelRepresentation();
  virtual ~vtkModelRepresentation();

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
  int SelectionMask;
  smtk::model::ManagerPtr Model;

private:
  vtkModelRepresentation(const vtkModelRepresentation&); // Not implemented.
  void operator = (const vtkModelRepresentation&); // Not implemented.
};

#endif // __smtk_vtk_ModelRepresentation_h
