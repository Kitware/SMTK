//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKSettings_h
#define smtk_extension_paraview_server_vtkSMTKSettings_h

#include "smtk/extension/paraview/server/Exports.h"

#include "vtkObject.h"
#include "vtkSmartPointer.h"

/**\brief Expose settings in ParaView related to SMTK.
  *
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKSettings : public vtkObject
{
public:
  static vtkSMTKSettings* New();
  vtkTypeMacro(vtkSMTKSettings, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;
  virtual ~vtkSMTKSettings();

  /**\brief Return the singleton.
   */
  static vtkSMTKSettings* GetInstance();

  /**\brief Indicate whether mouse-hovering should be enabled.
    *
    * By default, this is enabled. However, when dealing with large models
    * or slow rendering, users may wish to disable it.
    */
  vtkGetMacro(HighlightOnHover, bool);
  vtkSetMacro(HighlightOnHover, bool);

  /**\brief Choose how selections should be rendered.
    *
    * See the server-manager XML for details.
    */
  vtkGetMacro(SelectionRenderStyle, int);
  vtkSetMacro(SelectionRenderStyle, int);
  static constexpr int SolidSelectionStyle = 0;     // Match value in smconfig.xml
  static constexpr int WireframeSelectionStyle = 1; // Match value in smconfig.xml

  /**\brief Choose how resources should be presented.
    *
    * See the server-manager XML for details.
    */
  vtkGetMacro(ResourceTreeStyle, int);
  vtkSetMacro(ResourceTreeStyle, int);
  static constexpr int HierarchicalStyle = 0; // Match value in smconfig.xml
  static constexpr int TwoLevelStyle = 1;     // Match value in smconfig.xml

protected:
  vtkSMTKSettings();

  bool HighlightOnHover;
  int SelectionRenderStyle;
  int ResourceTreeStyle;

private:
  vtkSMTKSettings(const vtkSMTKSettings&) = delete;
  void operator=(const vtkSMTKSettings&) = delete;

  static vtkSmartPointer<vtkSMTKSettings> Instance;
};

#endif // smtk_extension_paraview_server_vtkSMTKSettings_h
