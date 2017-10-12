//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#pragma once

#include "smtk/extension/paraview/server/Exports.h"

#include "smtk/common/UUID.h"

#include "smtk/resource/Component.h"

#include "vtkObject.h"

#include <functional>
#include <map>
#include <set>

class SMTKPVSERVEREXTPLUGIN_EXPORT vtkSMTKResourceManagerWrapper : public vtkObject
{
public:
  using UUID = smtk::common::UUID;
  using UUIDs = smtk::common::UUIDs;

  vtkTypeMacro(vtkSMTKResourceManagerWrapper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkSMTKResourceManagerWrapper* New();

  vtkGetStringMacro(JSONRequest);
  vtkSetStringMacro(JSONRequest);

  vtkGetStringMacro(JSONResponse);
  vtkSetStringMacro(JSONResponse);

  void ProcessJSON();

  //BTX
  /// Provide access to the current selection
  const UUIDs& GetCurrentSelection() const { return this->Selection; }
  // Provide a way for representations to ask for updates when the selection is changed.
  /* These can't work until there's a real resource manager:
  typedef std::set<smtk::resource::Component::Ptr> Components;
  typedef std::function<void(
    const Components&, const Components&, const Components&, const std::string&)>
    SelectionChangedFunction;
    */
  // Just use UUIDs until the ResourceManager exists:
  typedef std::function<void(const UUIDs&, const UUIDs&, const UUIDs&, const std::string&)>
    SelectionChangedFunction;
  int Listen(SelectionChangedFunction fn, bool sendCurrentSelectionImmediately);
  bool Unlisten(int handle);
  //ETX

protected:
  vtkSMTKResourceManagerWrapper();
  ~vtkSMTKResourceManagerWrapper() override;

  char* JSONRequest;
  char* JSONResponse;
  std::map<int, SelectionChangedFunction> SelectionListeners;
  UUIDs Selection;

private:
  vtkSMTKResourceManagerWrapper(const vtkSMTKResourceManagerWrapper&) = delete;
  void operator=(const vtkSMTKResourceManagerWrapper&) = delete;
};
