//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_UIElementState_h
#define smtk_view_UIElementState_h

#include "smtk/CoreExports.h"

#include "smtk/string/Token.h"

#include "smtk/SharedFromThis.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace view
{

/// A base class for user interface elements
/// that wish to serialize their state.
///
/// UI elements, such as panels, should
/// 1. Inherit or own an instance of this class.
/// 2. Insert this class into a map of such instances
///    held by the smtk::common::Managers object used
///    for application context.
///
/// Read and write operations will then iterate the
/// map to serialize or deserialize each entry.
class SMTKCORE_EXPORT UIElementState
{
public:
  smtkTypenameMacroBase(UIElementState);

  /// Return an (application-unique) token for the type of user-interface
  /// element this state object will serialize/deserialize.
  ///
  /// We suggest the type-name of the UI element (e.g., "pqSMTKTaskPanel")
  /// that owns or inherits this UIElementState instance.
  virtual smtk::string::Token elementType() const = 0;

  /// Return the UI element's current, in-memory state to be serialized.
  virtual nlohmann::json configuration() = 0;

  /// Using the deserialized configuration \a data, configure the user
  /// interface element to match it.
  virtual bool configure(const nlohmann::json& data) = 0;
};

} // namespace view
} // namespace smtk

#endif // smtk_view_UIElementState_h
