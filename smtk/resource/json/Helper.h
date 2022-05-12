//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_json_Helper_h
#define smtk_resource_json_Helper_h

#include "smtk/resource/Resource.h"

#include "smtk/common/Managers.h"
#include "smtk/common/ThreadPool.h"
#include "smtk/common/TypeName.h"

#include <exception>
#include <future>
#include <string>
#include <vector>

namespace smtk
{
namespace resource
{
namespace json
{

/// A helper for serializing resources.
///
/// This is needed in order to provide access to smtk::common::Managers
/// data that comes from the application and may be needed during
/// (de)serialization.
class SMTKCORE_EXPORT Helper
{
public:
  /// JSON data type
  using json = nlohmann::json;

  /// Destructor is public, but you shouldn't use it.
  ~Helper();
  /// Copy construction and assignment are disallowed.
  Helper(const Helper&) = delete;
  void operator=(const Helper&) = delete;

  /// Return the helper "singleton".
  ///
  /// The object returned is a per-thread instance
  /// at the top of a stack that may be altered using
  /// the pushInstance() and popInstance() methods.
  /// This allows nested deserializers to each have
  /// their own context that appears to be globally
  /// available.
  static Helper& instance();

  /// Push a new helper instance on the local thread's stack.
  ///
  /// The returned \a Helper will have the same managers as
  /// the previous (if any) helper.
  static Helper& pushInstance(const smtk::resource::Resource::Ptr& parent);

  /// Pop a helper instance off the local thread's stack.
  static void popInstance();

  /// Return the nesting level (i.e., the number of helper instances in the stack).
  ///
  /// The outermost helper will return 1 (assuming you have called instance() first).
  static std::size_t nestingDepth();

  /// Set/get the managers to use when serializing/deserializing.
  ///
  /// Call setManagers() with an instance of all your application's
  /// managers before attempting to serialize/deserialize as helpers
  /// are allowed to use managers as needed.
  void setManagers(const smtk::common::Managers::Ptr& managers);
  smtk::common::Managers::Ptr managers();

  /// Reset the helper's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these resources is recommended since
  /// it will free memory.
  void clear();

  /// Returns true if the helper is for deserializing top-level or child resources.
  bool topLevel() const { return m_topLevel; }

  /// Return the resource currently being deserialized.
  smtk::resource::Resource::Ptr resource() const { return m_parent; }

  /// Return the resource currently being deserialized, cast to a
  /// shared pointer to a \a ResourceType . This will return null if
  /// the current resource does not inherit \a ResourceType.
  template<typename ResourceType>
  std::shared_ptr<ResourceType> resourceAs() const
  {
    return std::dynamic_pointer_cast<ResourceType>(m_parent);
  }

  /// Return a thread pool for readers to use.
  ///
  /// This is intended to offload I/O for external data to
  /// background threads that can be joined before the read
  /// operation completes.
  static smtk::common::ThreadPool<>& threadPool() { return m_threadPool; }

  std::vector<std::future<void>>& futures() { return m_futures; }

protected:
  Helper();
  smtk::common::Managers::Ptr m_managers;
  /// m_topLevel indicates whether pushInstance() (false) or instance() (true)
  /// was used to create this helper.
  bool m_topLevel = true;
  smtk::resource::Resource::Ptr m_parent;
  std::vector<std::future<void>> m_futures;
  static smtk::common::ThreadPool<> m_threadPool;
};

} // namespace json
} // namespace resource
} // namespace smtk

#endif // smtk_resource_json_Helper_h
