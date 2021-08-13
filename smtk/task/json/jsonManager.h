//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Manager_h
#define smtk_task_json_Manager_h

#include "smtk/task/Manager.h"

#include "smtk/common/Managers.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace task
{
namespace json
{

/// Tools for saving and restoring the state of a task manager.
class SMTKCORE_EXPORT jsonManager
{
public:
  /// Serialize the task manager.
  ///
  /// Obviously, \a managers must hold a task manager before you
  /// call this method. Depending on the task instances it holds,
  /// other managers may be required.
  static bool serialize(
    const std::shared_ptr<smtk::common::Managers>& managers,
    nlohmann::json& json);
  /// Deserialize the task manager.
  ///
  /// Obviously, \a managers must hold or be able to create a
  /// task manager. Depending on the task instances being deserialized,
  /// this method may access and modify other managers held by the
  /// \a managers instance.
  static bool deserialize(
    const std::shared_ptr<smtk::common::Managers>& managers,
    const nlohmann::json& json);
};

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_Manager_h
