//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_server_vtkSMTKWrapper_h
#define smtk_extension_paraview_server_vtkSMTKWrapper_h

#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"

#include "smtk/common/UUID.h"

#include "smtk/view/SelectionObserver.h"

#include "smtk/PublicPointerDefs.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkObject.h"

#include "nlohmann/json.hpp"

#include <functional>
#include <map>
#include <set>

class vtkMultiBlockDataSet;
class vtkPVDataRepresentation;
class vtkSMTKResource;
class vtkSelection;

/** \brief Server-side application state for ParaView-based SMTK apps.
  *
  * This object is instantiated on ParaView servers as they are connected and disconnected
  * from the client.
  * The client manages this object via a vtkSMSMTKWrapperProxy instance.
  *
  * This object manages a resource manager, an operation manager, and a selection.
  * The resource and operation managers are the ones created/owned by the smtk::environment;
  * as plugins are loaded that contain new types of resources and operations, these managers
  * are updated.
  * Since ParaView has a single, application-wide selection, that design decision is
  * forced onto SMTK applications that use ParaView, hence the inclusion of a selection here.
  */
class SMTKPVSERVEREXT_EXPORT vtkSMTKWrapper : public vtkObject
{
public:
  using UUID = smtk::common::UUID;
  using UUIDs = smtk::common::UUIDs;
  using json = nlohmann::json;
  /// A multiblock output from vtkResourceMultiBlockSource and a selection acting on it.
  using SelectedResource = std::pair<vtkMultiBlockDataSet*, vtkSelection*>;
  /// A subset of resources and selections that reside on the server holding this wrapper.
  using SelectedResources = std::set<SelectedResource>;

  vtkTypeMacro(vtkSMTKWrapper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  /// Create a new SMTK wrapper. There should usually be one per server-side process.
  static vtkSMTKWrapper* New();

  /// Set/get the pipeline source for the currently-active resource
  vtkSetObjectMacro(ActiveResource, vtkAlgorithmOutput);
  vtkGetObjectMacro(ActiveResource, vtkAlgorithmOutput);

  /// Return the server's application-wide resource manager.
  smtk::resource::ManagerPtr GetResourceManager() const { return this->ResourceManager; }
  /// Return the server's application-wide operation manager.
  smtk::operation::ManagerPtr GetOperationManager() const { return this->OperationManager; }
  /// Return the server's application-wide geometry manager.
  smtk::geometry::ManagerPtr GetGeometryManager() const { return this->GeometryManager; }
  /// Return the server's application-wide project manager.
  smtk::project::ManagerPtr GetProjectManager() const { return this->ProjectManager; }
  /// Return the server's application-wide view manager.
  smtk::view::ManagerPtr GetViewManager() const { return this->ViewManager; }

  /// Return the server's application-wide selection handler.
  smtk::view::SelectionPtr GetSelection() const { return this->Selection; }

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

  void SetRepresentation(vtkPVDataRepresentation* rep);
  vtkPVDataRepresentation* GetRepresentation();

protected:
  vtkSMTKWrapper();
  ~vtkSMTKWrapper() override;

  void FetchHardwareSelection(json& response);
  void AddResourceFilter(json& response);
  void RemoveResourceFilter(json& response);

  vtkSMTKResource* GetVTKResource(vtkAlgorithm*);

  vtkPVDataRepresentation* Representation = nullptr;
  vtkAlgorithmOutput* ActiveResource;
  vtkAlgorithmOutput* SelectedPort;
  vtkAlgorithmOutput* SelectionObj;
  char* JSONRequest;
  char* JSONResponse;
  smtk::resource::ManagerPtr ResourceManager;
  smtk::operation::ManagerPtr OperationManager;
  smtk::geometry::ManagerPtr GeometryManager;
  smtk::project::ManagerPtr ProjectManager;
  smtk::view::ManagerPtr ViewManager;
  smtk::view::SelectionPtr Selection;
  std::string SelectionSource;
  int HoveredValue;
  int SelectedValue;

private:
  vtkSMTKWrapper(const vtkSMTKWrapper&) = delete;
  void operator=(const vtkSMTKWrapper&) = delete;
};

#endif
