//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_operation_Launcher_h
#define __smtk_operation_Launcher_h

#include "smtk/CoreExports.h"
#include "smtk/common/Generator.h"
#include "smtk/operation/Operation.h"

#include <future>
#include <string>
#include <unordered_map>

namespace smtk
{
namespace operation
{

typedef std::function<std::shared_future<Operation::Result>(const Operation::Ptr&)> Launcher;

/// A functor for executing operations and returning futures of the result.
/// Multiple launch types are supported and can be accessed using the
/// LauncherMap's key.
class SMTKCORE_EXPORT Launchers
{
public:
  typedef std::unordered_map<std::string, Launcher> LauncherMap;

  /// Construct a launcher that launches operations using an asynchronous thread
  /// by default.
  Launchers();

  /// Construct a launcher that launches operations using a user-defined method
  /// by default.
  Launchers(const LauncherMap::mapped_type&);

  /// Add a method for launching operations, paired with its associated key.
  std::pair<Launchers::LauncherMap::iterator, bool> insert(
    const Launchers::LauncherMap::value_type&);

  /// Add a method for launching operations, paired with its associated key.
  std::pair<Launchers::LauncherMap::iterator, bool> emplace(Launchers::LauncherMap::value_type&&);

  /// Access a method for launching operations via its associated key.
  Launchers::LauncherMap::mapped_type& operator[](const Launchers::LauncherMap::key_type&);

  /// Remove a method for launching operations via its associated key.
  Launchers::LauncherMap::size_type erase(const Launchers::LauncherMap::key_type&);

  /// Launch an operation using the default launch method.
  std::shared_future<Operation::Result> operator()(const Operation::Ptr&);

  /// Launch an operation using the launch method associated to the input key.
  std::shared_future<Operation::Result> operator()(
    const Operation::Ptr&,
    const Launchers::LauncherMap::key_type&);

protected:
  LauncherMap m_launchers;
};
} // namespace operation
} // namespace smtk

#endif // __smtk_operation_Launcher_h
