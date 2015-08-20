#include "CropTool.hpp"
#include "vtkPlane.h"
#include "vtkCylinder.h"
#include "vtkSphere.h"
#include "vtkTransform.h"
#include "vtkExtractGeometry.h"
#include "vtkImplicitBoolean.h"
#include "vtkMOABUtils.h"

#include "assert.h"

using namespace moab;

CropTool::CropTool()
{
  func_types[0] = func_types[1] = func_types[2] = NONE;
  funcs[0] = funcs[1] = funcs[2] = NULL;
  boolFunction = NULL;
  theFunction = NULL;
}

CropTool::~CropTool()
{
  vtkMOABUtils::remove_geom_extractors();

  if (NULL != boolFunction) {
      // need to remove the geometry extractor from the pipeline, then delete it
    vtkMOABUtils::remove_geom_extractors();
    boolFunction->Delete();
  }
}

vtkImplicitFunction *CropTool::get_the_function(const bool) 
{
  return theFunction;
}

void CropTool::set_the_function(vtkImplicitFunction *this_func) 
{
  if (NULL != theFunction) {
    vtkMOABUtils::remove_geom_extractors();
    theFunction->Delete();
  }

  theFunction = this_func;
  vtkMOABUtils::add_geom_extractors(theFunction);
}

void CropTool::type_activated(const int num, const int type_num)
{
    //vtkImplicitFunction *the_func = NULL;
  
  vtkPlane *plane;
  vtkCylinder *cyl;
  vtkSphere *sph;
  
  switch (type_num) {
    case NONE:
      func_types[num] = NONE;
      break;
    case PLANEX:
      plane = vtkPlane::New();
      assert (NULL != plane);
      plane->SetNormal(1.0, 0.0, 0.0);
      funcs[num] = plane;
        //bool_func->AddFunction(plane);
        //theFunction = plane;
      func_types[num] = PLANEX;
      break;
    case PLANEY:
      plane = vtkPlane::New();
      assert (NULL != plane);
      plane->SetNormal(0.0, 1.0, 0.0);
      funcs[num] = plane;
        //bool_func->AddFunction(plane);
        //theFunction = plane;
      func_types[num] = PLANEY;
      break;
    case PLANEZ:
      plane = vtkPlane::New();
      assert (NULL != plane);
      plane->SetNormal(0.0, 0.0, 1.0);
      funcs[num] = plane;
        //bool_func->AddFunction(plane);
        //theFunction = plane;
      func_types[num] = PLANEZ;
      break;
    case CYLINDER:
      cyl = vtkCylinder::New();
      cyl->SetRadius(1.0);
      funcs[num] = cyl;
        //bool_func->AddFunction(cyl);
      func_types[num] = CYLINDER;
      break;
    case SPHERE:
      sph = vtkSphere::New();
      sph->SetRadius(1.0);
      funcs[num] = sph;
        //bool_func->AddFunction(sph);
      func_types[num] = SPHERE;
      break;
  }

  if (funcs[num] == NULL && theFunction != NULL) {
    vtkMOABUtils::remove_geom_extractors();
    theFunction->Delete();
    theFunction = NULL;
  }
  else
    set_the_function(funcs[num]);
}

void CropTool::radius_value_changed(const int num, const int new_val)
{
  if (num < 0 || num > 2 || funcs[num] == NULL) {
    std::cerr << "No function of that type." << std::endl;
    return;
  }
  
  vtkCylinder *cyl = vtkCylinder::SafeDownCast(funcs[num]);
  if (NULL != cyl) {
    cyl->SetRadius(((double)new_val)*0.1);
    return;
  }
  
  vtkSphere *sph = vtkSphere::SafeDownCast(funcs[num]);
  if (NULL != sph) {
    sph->SetRadius(((double)new_val)*0.1);
    return;
  }
  
  std::cerr << "Can't set radius on a plane." << std::endl;
}

void CropTool::xyz_value_changed(const int num, const int new_xyz[3])
{
  double dxyz[3] = {(double) new_xyz[0], (double) new_xyz[1], (double) new_xyz[2]};
  dxyz[0] *= 0.1; dxyz[1] *= 0.1; dxyz[2] *= 0.1;
   
  if (func_types[num] >= PLANEX && func_types[num] <= PLANEZ) {
    vtkPlane *this_plane = vtkPlane::SafeDownCast(funcs[num]);
    assert(NULL != this_plane);
    this_plane->SetOrigin(dxyz);
  }
  else if (func_types[num] == CYLINDER) {
    vtkCylinder *this_cyl = vtkCylinder::SafeDownCast(funcs[num]);
    assert(NULL != this_cyl);
    this_cyl->SetCenter(dxyz);
  }
  else if (func_types[num] == SPHERE) {
    vtkSphere *this_sph = vtkSphere::SafeDownCast(funcs[num]);
    assert(NULL != this_sph);
    this_sph->SetCenter(dxyz);
  }
  else {
    std::cerr << "No cropping function there." << std::endl;
  }
}

void CropTool::rotate_value_changed(const int num, const int new_vals[2])
{
  vtkTransform *transf = get_transform(num);
  transf->Identity();
  int val_num = 0;
  if (func_types[num] == PLANEY || func_types[num] == PLANEZ || func_types[num] == CYLINDER)
    transf->RotateX((double)new_vals[val_num++]);
  if (func_types[num] == PLANEX || func_types[num] == PLANEZ || func_types[num] == CYLINDER)
    transf->RotateY((double)new_vals[val_num++]);
  if (func_types[num] == PLANEX || func_types[num] == PLANEY || func_types[num] == CYLINDER)
    transf->RotateZ((double)new_vals[val_num++]);
}

void CropTool::toggle_changed(const int , const bool )
{
    //geomExtractor->SetExtractInside((checked ? 1 : 0));
}

vtkTransform *CropTool::get_transform(const int num) 
{
  if (num < 0 || num > 2 || funcs[num] == NULL) {
    std::cerr << "No function of that type." << std::endl;
    return NULL;
  }

    // get a transform for this function
  vtkAbstractTransform *temp = funcs[num]->GetTransform();
  vtkTransform *transf;
  
  if (NULL == temp) {
    transf = vtkTransform::New();
    funcs[num]->SetTransform(transf);
  }
  else {
    transf = vtkTransform::SafeDownCast(temp);
  }
  
  return transf;
}
