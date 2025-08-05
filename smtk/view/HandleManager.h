//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_HandleManager_h
#define smtk_view_HandleManager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/resource/Lock.h"
#include "smtk/resource/Resource.h"
#include "smtk/view/Configuration.h"

#include <map>

namespace smtk
{
namespace view
{

/**\brief A utility for views manage dependencies on objects of various types.
  *
  * This class, which behaves as a singleton, manages "handles" (unique strings)
  * for objects which may or may not be persistent. Instances of any class
  * that inherits `std::enable_shared_from_this<>` and uses the SMTK type macros
  * (so that typeToken() and related methods are defined) may be assigned handles.
  *
  * The intent is for handles to be used as references during the lifetime of
  * a process (usually the server process, while the client would be a browser
  * which may be remote) in order to refer to items that may not be present on
  * the client.  Handles are not unique across multiple processes.
  * Handle strings are not preserved on disk; they live only for the duration of
  * the process.
  *
  * Remote views of data may then query the process where the objects reside to
  * obtain information on the objects or to modify the objects.
  */
class SMTKCORE_EXPORT HandleManager : smtkEnableSharedPtr(HandleManager)
{
public:
  smtkTypeMacroBase(HandleManager);
  smtkCreateMacro(HandleManager);

  HandleManager();
  HandleManager(const HandleManager&) = delete;
  HandleManager& operator=(const HandleManager&) = delete;

  static Ptr instance();

  /// Do not manage the given \a object, but return the handle string for it.
  template<typename T>
  std::string handleString(const T* object) const
  {
    if (!object)
    {
      return "0x0";
    }
    std::ostringstream hstr;
    hstr << std::hex << object << std::dec;
    return hstr.str();
  }

  /// Return true if the \a object has a pre-existing handle and false otherwise.
  template<typename T>
  bool hasHandle(const T* object) const
  {
    auto key = this->handleString(object);
    return m_objects.find(key) != m_objects.end();
  }

  template<typename T>
  std::string handle(T* object)
  {
    if (!object)
    {
      return "0x0";
    }
    auto immediateType = object->typeToken();
    auto itit = m_typeMap.find(immediateType);
    if (itit == m_typeMap.end())
    {
      auto inh = object->classHierarchy();
      for (std::size_t ii = 1; ii < inh.size(); ++ii)
      {
        // std::cerr << "**  Mapping " << inh[ii - 1].data() << " to " << inh[ii].data() << "\n";
        m_typeMap[inh[ii - 1]] = inh[ii];
      }
      // Force the base-most type to map to itself:
      m_typeMap[*inh.rbegin()] = *inh.rbegin();
      // std::cerr << "**  Mapping " << inh.rbegin()->data() << " to " << inh.rbegin()->data() << "\n";
    }
    std::ostringstream hstr;
    hstr << std::hex << object << std::dec;
    auto obit = m_objects.find(hstr.str());
    if (obit != m_objects.end())
    {
      return obit->second.m_handle;
    }
    obit = m_objects.emplace(hstr.str(), HandleEntry(immediateType, object)).first;
    return obit->second.m_handle;
  }

  template<typename T>
  T* fromHandle(const std::string& handle)
  {
    auto obit = m_objects.find(handle);
    if (obit == m_objects.end())
    {
      std::cerr << "No object for handle " << handle << "\n";
      return nullptr;
    }
    if (
      obit->second.m_resource &&
      obit->second.m_resource->locked() == smtk::resource::LockType::Write)
    {
      std::cerr << "Resource owning " << handle << " is locked for writing.\n";
      return nullptr;
    }
    smtk::string::Token typeToken(smtk::common::typeName<T>());
    if (!this->typeResolves(typeToken, obit->second.m_type))
    {
      std::cerr << "Object for handle " << handle << " is type " << obit->second.m_type.data()
                << " not " << typeToken.data() << "\n";
      return nullptr;
    }
    return reinterpret_cast<T*>(obit->second.m_object);
  }

  /// Event types for handles.
  enum EventType
  {
    Created,  // Objects with the given handles were created.
    Expunged, // Objects with the given handles were expunged.
    Modified  // Objects with the given handles have been modified.
  };

  /// The manager aggregates changes to handles by the type of object involved.
  using HandlesByType = std::unordered_map<smtk::string::Token, std::set<std::string>>;

  /// Observers are passed a collection of handles (strings) grouped by the type of object
  /// at the handle (a string token), and the type of event.
  ///
  /// Each time an operation complete, the the observer may be called twice: once
  /// for expunged handles and once for modified handles.
  using Observer = std::function<void(const HandlesByType&, EventType)>;

  /// The type of the object which holds a collection of observers to invoke.
  using Observers = smtk::common::Observers<Observer>;

  /// Return the collection of observers so observers can be added or removed.
  ///
  /// These observers will only be called from within an operation observer,
  /// at which point the locks on relevant resources will be held.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /// Tell this manager to monitor operations run for \a opMgr to scrub deleted objects.
  void registerOperationManager(const smtk::operation::Manager::Ptr& opMgr);

protected:
  /// This method is invoked by the operation observer to remove expunged entries.
  void handleOperation(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  /// Use m_typeMap to validate that a pointer is of the requested type (or a subclass).
  bool typeResolves(smtk::string::Token requestedType, smtk::string::Token immediateType);
  bool typeResolvesRecursive(smtk::string::Token requestedType, smtk::string::Token immediateType);

  /// Map object->typeName() values to their superclass type-name.
  ///
  /// Searching this map repeatedly until no match is found will return the
  /// name of the object's base-most type (i.e., to a key of m_objects).
  /// This exists for type validation before casting.
  std::unordered_map<smtk::string::Token, smtk::string::Token> m_typeMap;

  /// Data held for each handle.
  struct HandleEntry
  {
    HandleEntry(smtk::string::Token immediateType, smtk::view::Configuration* object)
      : m_type(immediateType)
      , m_resource(nullptr)
      , m_object(object)
    {
      std::ostringstream ptr;
      ptr << std::hex << object << std::dec;
      m_handle = ptr.str();
    }

    HandleEntry(smtk::string::Token immediateType, smtk::attribute::Item* object)
      : m_type(immediateType)
      , m_resource(object->attribute()->parentResource())
      , m_object(object)
    {
      std::ostringstream ptr;
      ptr << std::hex << object << std::dec;
      m_handle = ptr.str();
    }

    template<typename T>
    HandleEntry(smtk::string::Token immediateType, T* object)
      : m_type(immediateType)
      , m_resource(object->parentResource())
      , m_object(object)
    {
      std::ostringstream ptr;
      ptr << std::hex << object << std::dec;
      m_handle = ptr.str();
    }

    /// The exact type of the data.
    smtk::string::Token m_type;
    /// The resource owning the handle.
    ///
    /// This is used to check the lock state. If a handle is locked, requests
    /// for its underlying data (fromHandle<>) will be denied.
    smtk::resource::Resource* m_resource;
    /// The handle of the data.
    std::string m_handle;
    /// A pointer to the object.
    void* m_object;
  };

  /// Map an object to a tuple holding its type and its handle-string.
  std::unordered_map<std::string, HandleEntry> m_objects;

  /// Functions to call when an object underlying a handle is modified or expunged.
  Observers m_observers;

  /// The currently-registered operation manager to observe for updates.
  smtk::operation::Manager::Ptr m_operationManager;

  /// The observer key for a method that monitors operations
  /// in order to remove handles as they are expunged.
  ///
  /// This manager deals with components, resources, and
  /// attribute-items.
  smtk::operation::Observers::Key m_watcher;
};

} // namespace view
} // namespace smtk

#endif
