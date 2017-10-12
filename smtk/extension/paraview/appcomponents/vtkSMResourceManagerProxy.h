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

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "nlohmann/json.hpp"

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
class SMTKPQCOMPONENTSEXT_EXPORT vtkSMResourceManagerProxy : public vtkSMProxy
{
  using json = nlohmann::json;

public:
  static vtkSMResourceManagerProxy* New();
  vtkTypeMacro(vtkSMResourceManagerProxy, vtkSMProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Method called by client-side selection manager to send selection to server.
  template <typename T>
  void sendClientSelectionToServer(
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
  void recvClientSelectionFromServer(
    vtkSMSourceProxy* dataSource, vtkSMSourceProxy* selnSource, T& seln, T& added, T& removed);

protected:
  vtkSMResourceManagerProxy();
  ~vtkSMResourceManagerProxy() override;

  void send(const json& selnInfo);
  void recv(vtkSMSourceProxy* dataSource, vtkSMSourceProxy* selnSource, json& selnInfo);

private:
  vtkSMResourceManagerProxy(const vtkSMResourceManagerProxy&) = delete;
  void operator=(const vtkSMResourceManagerProxy&) = delete;
};
