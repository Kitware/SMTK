#ifndef CROPTOOL_HPP
#define CROPTOOL_HPP

class vtkImplicitFunction;
class vtkExtractGeometry;
class vtkTransform;
class vtkImplicitBoolean;

class CropTool 
{
public:
  CropTool();
  ~CropTool();
  enum FuncType {NONE = 0, PLANEX, PLANEY, PLANEZ, CYLINDER, SPHERE};
  void type_activated(const int num, const int type_num);
  void radius_value_changed(const int num, const int new_val);
  void xyz_value_changed(const int num, const int xyz[3]);
  void rotate_value_changed(const int num, const int new_vals[2]);
  void toggle_changed(const int num, const bool checked);
private:
  FuncType func_types[3];
  vtkImplicitBoolean *boolFunction;
  vtkImplicitFunction *theFunction;
  vtkImplicitFunction *funcs[3];
  vtkTransform *get_transform(const int num);
  vtkImplicitFunction *get_the_function(const bool force = true);
  void set_the_function(vtkImplicitFunction *this_func);
};


#endif
