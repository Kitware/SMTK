//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_vtkSMSMTKResourceManagerProxy_h
#define smtk_extension_paraview_appcomponents_vtkSMSMTKResourceManagerProxy_h

#include "smtk/extension/paraview/server/Exports.h"

#include "smtk/resource/Manager.h"

#include "smtk/view/Selection.h"

#include "vtkSMProxy.h"

#include "nlohmann/json.hpp"

class vtkSMSourceProxy;
class vtkSMRepresentationProxy;

/**\brief Proxy for SMTK's resource manager.
 *
 * Since no resource manager exists yet, this is just a clearing
 * house for selection synchronization across the client-server
 * connection.
 *
 * Note that selections in ParaView are a little bizarre; we only
 * get notified of the selection occurring on the client even though
 * it happens on the server. Also, the client does not get the list
 * of what's selected... you have to ask the server for that.
 * So, the methods in this class cater to that communication pattern:
 * the client dictates the selection to the server when the user changes
 * it on the client side;
 * and the client asks the server for the selection when it is
 * notified that a selection event occurred on the server.
 */
class SMTKPVSERVEREXTPLUGIN_EXPORT vtkSMSMTKResourceManagerProxy : public vtkSMProxy
{
  using json = nlohmann::json;

public:
  static vtkSMSMTKResourceManagerProxy* New();
  static vtkSMSMTKResourceManagerProxy* Instance();
  vtkTypeMacro(vtkSMSMTKResourceManagerProxy, vtkSMProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Return the client-side resource manager (mirrored on the server via this proxy).
  smtk::resource::ManagerPtr GetManager() const;

  /// Return the client-side selection manager (mirrored on the server via this proxy).
  smtk::view::SelectionPtr GetSelection() const;

  /// Return the client-side operation manager (mirrored on the server via this proxy).
  smtk::operation::ManagerPtr GetOperationManager() const;

  /// Call this to indicate which PV data has the active PV selection.
  void SetSelectedPortProxy(vtkSMSourceProxy* pxy);

  /// Call this to pass the PV selection specification to the wrapper.
  void SetSelectionObjProxy(vtkSMSourceProxy* pxy);

  /// Method called by client-side selection manager to send selection to server.
  template <typename T>
  void SendClientSelectionToServer(
    const T& seln, const T& added, const T& removed, const std::string& sender);

  /**\brief Fetch the selection from the server.
    *
    * This method is called by client when pqSelectionManager
    * informs us a hardware selection occurred on server.
    * The \a dataSource and \a selnSource are the values returned
    * by getSourceProxy() and getSelectionSource() on the pqOutputPort
    * passed to the pqSelectionManager's selectionChanged() signal.
    *
    * The method populates \a seln, \a added, and \a removed
    * based on values obtained from the source proxies.
    */
  template <typename T>
  void RecvClientSelectionFromServer(
    vtkSMSourceProxy* dataSource, vtkSMSourceProxy* selnSource, T& seln, T& added, T& removed);

  void FetchHardwareSelection();

  void AddResourceProxy(vtkSMSourceProxy* rsrc);
  void RemoveResourceProxy(vtkSMSourceProxy* rsrc);

  void SetRepresentation(vtkSMRepresentationProxy* pxy);
  void SetResourceForRepresentation(
    smtk::resource::ResourcePtr clientSideResource, vtkSMRepresentationProxy* pxy);

protected:
  vtkSMSMTKResourceManagerProxy();
  ~vtkSMSMTKResourceManagerProxy() override;

  void Send(const json& selnInfo);
  void Recv(vtkSMSourceProxy* dataSource, vtkSMSourceProxy* selnSource, json& selnInfo);

  json JSONRPCRequest(const json& request);
  json JSONRPCRequest(const std::string& request);
  void JSONRPCNotification(const json& note);
  void JSONRPCNotification(const std::string& note);

  smtk::resource::ManagerPtr Manager;
  smtk::view::SelectionPtr Selection;
  smtk::operation::ManagerPtr OperationManager;

private:
  vtkSMSMTKResourceManagerProxy(const vtkSMSMTKResourceManagerProxy&) = delete;
  void operator=(const vtkSMSMTKResourceManagerProxy&) = delete;
};

#endif
