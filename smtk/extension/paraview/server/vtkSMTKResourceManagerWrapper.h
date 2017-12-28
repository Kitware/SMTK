//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKResourceManagerWrapper_h
#define smtk_extension_paraview_server_vtkSMTKResourceManagerWrapper_h

#include "smtk/extension/paraview/server/Exports.h"

#include "smtk/common/UUID.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkObject.h"

#include "json.hpp"

#include <functional>
#include <map>
#include <set>

/** \brief Server-side application state for ParaView-based SMTK apps.
  *
  * This object is instantiated on ParaView servers as they are connected and disconnected
  * from the client.
  * The client manages this object via a vtkSMSMTKResourceManagerProxy instance.
  *
  * This object owns both a resource manager and a selection manager.
  * Since ParaView has a single, application-wide selection, that design decision is
  * forced onto SMTK applications that use ParaView.
  *
  * VTK-style classes that interface between ParaView and SMTK can fetch
  * this object by calling its static Instance() method on any server process.
  * This allows vtkSMTKModelReader and other classes to work as sources
  * (no inputs required) instead of filters while still allowing resources
  * to be shared among them.
  * This is necessary because of the difference between ParaView/VTK's pipeline
  * model and SMTK's in-place operation model.
  */
class SMTKPVSERVEREXTPLUGIN_EXPORT vtkSMTKResourceManagerWrapper : public vtkObject
{
public:
  using UUID = smtk::common::UUID;
  using UUIDs = smtk::common::UUIDs;
  using json = nlohmann::json;

  vtkTypeMacro(vtkSMTKResourceManagerWrapper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  /// Return the singleton, creating it if it does not exist yet.
  static vtkSMTKResourceManagerWrapper* New();
  /// Return the singleton if it exists, or nullptr otherwise.
  static vtkSMTKResourceManagerWrapper* Instance();

  /// Set/get the pipeline source for the currently-active resource
  vtkSetObjectMacro(ActiveResource, vtkAlgorithmOutput);
  vtkGetObjectMacro(ActiveResource, vtkAlgorithmOutput);

  /// Return the server's application-wide resource manager.
  smtk::resource::ManagerPtr GetManager() const { return this->ResourceManager; }
  /// Return the server's application-wide selection handler.
  smtk::view::SelectionPtr GetSelection() const { return this->Selection; }
  /// Return the server's application-wide operation manager
  smtk::operation::ManagerPtr GetOperationManager() const { return this->OperationManager; }

  /// Return the SMTK selection source used by this class to indicate a hardware selection was made.
  const std::string& GetSelectionSource() const { return this->SelectionSource; }

  /// Return the SMTK selection value used by this class to indicate a component is selected.
  int GetSelectedValue() const { return this->SelectedValue; }

  /// Return the SMTK selection value used by this class to indicate a component is hovered.
  int GetHoveredValue() const { return this->HoveredValue; }

  /// Set/get the data object on which the user most recently made a selection.
  vtkSetObjectMacro(SelectedPort, vtkAlgorithmOutput);
  vtkGetObjectMacro(SelectedPort, vtkAlgorithmOutput);

  /// Set/get the selection object specifying the user's most recent selection.
  vtkSetObjectMacro(SelectionObj, vtkAlgorithmOutput);
  vtkGetObjectMacro(SelectionObj, vtkAlgorithmOutput);

  /// Set/get a JSON request. Call SetJSONRequest(), ProcessJSON(), and then GetJSONResponse().
  vtkGetStringMacro(JSONRequest);
  vtkSetStringMacro(JSONRequest);

  /// Set/get a JSON response. Call SetJSONRequest(), ProcessJSON(), and then GetJSONResponse().
  vtkGetStringMacro(JSONResponse);
  vtkSetStringMacro(JSONResponse);

  /// Processes the current JSONRequest and overwrites JSONResponse with a result.
  void ProcessJSON();

protected:
  vtkSMTKResourceManagerWrapper();
  ~vtkSMTKResourceManagerWrapper() override;

  void FetchHardwareSelection(json& response);
  void AddResource(json& response);
  void RemoveResource(json& response);

  vtkAlgorithmOutput* ActiveResource;
  vtkAlgorithmOutput* SelectedPort;
  vtkAlgorithmOutput* SelectionObj;
  char* JSONRequest;
  char* JSONResponse;
  smtk::resource::ManagerPtr ResourceManager;
  smtk::view::SelectionPtr Selection;
  smtk::operation::ManagerPtr OperationManager;
  std::string SelectionSource;
  int SelectionListener;
  int HoveredValue;
  int SelectedValue;

private:
  vtkSMTKResourceManagerWrapper(const vtkSMTKResourceManagerWrapper&) = delete;
  void operator=(const vtkSMTKResourceManagerWrapper&) = delete;
};

#endif
