//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_RuntimeTypeContainer_h
#define smtk_common_RuntimeTypeContainer_h

#include "smtk/common/TypeContainer.h"

namespace smtk
{
namespace common
{
/**\brief A container that can hold objects by their exact compile-time
  *       type as well as a commonly-inherited, run-time type.
  *
  * This class extends TypeContainer with a runtime API that
  * stores objects of a common type under user-provided "type names"
  * chosen at run time. (These "type names" do not need to name
  * actual types, but they may.)
  *
  * ## Insertion and retrieval
  *
  * For example, consider a TypeContainer used to hold metadata
  * objects. Compile-time objects can be inserted during the build
  * but users may need to add "programmable" metadata objects at
  * run-time. The application can provide a base RuntimeMetadata
  * class with virtual methods and then allow users to "name"
  * instances of this base class as if each represented its own type.
  *
  * ```cpp
  * struct MetadataA { const char* name = "Foo"; };
  * struct MetadataB { const char* name = "Bar"; };
  * class RuntimeMetadata
  * {
  * public:
  *   RuntimeMetadata(const std::string& name) : m_name(name) { }
  *   virtual std::string name() const { return m_name; }
  * private:
  *   std::string m_name;
  * };
  *
  * class Baz : public RuntimeMetadata
  * {
  * public:
  *   Baz() : RuntimeMetadata("Baz") { };
  * };
  *
  * RuntimeTypeContainer allMetadata(MetadataA(), MetadataB());
  * …
  * allMetadata.emplaceRuntime("Baz", RuntimeMetadata("Baz"));
  * ```
  *
  * In this example, there are now two "compile-time" entries
  * in `allMetadata` plus one "run-time" entry. You can fetch
  * "Foo" and "Bar" by knowing their type at compile time:
  * ```cpp
  * auto& foo = allMetadata.get<Foo>();
  * auto& bar = allMetadata.get<Bar>();
  * ```
  * You can fetch "Baz" by knowing *either* its type or that it
  * inherits RuntimeMetadata:
  * ```cpp
  * auto bazDirect = allMetadata.get<Baz>();
  * auto bazByBase = allMetadata.getRuntime<RuntimeMetadata>("Baz");
  * ```
  *
  * Importantly, (1) you can store multiple "run-time" objects sharing the
  * same base class in the type container as long as their declared types
  * are unique and (2) the declared type does not need to match any actual
  * type. However, (3) if fetching by a run-time object's base type, the
  * template parameter must exactly match the type used to insert the
  * object into the TypeContainer.
  *
  * For example:
  * ```cpp
  * class Xyzzy : public Baz { };
  * allMetadata.emplaceRuntime<RuntimeMetadata>("Xyzzy", Xyzzy());
  * auto dataBad = allMetadata.getRuntime<Baz>("Xyzzy"); // Will NOT work.
  * auto dataGood = allMetadata.getRuntime<RuntimeMetadata>("Xyzzy"); // Correct.
  * ```
  *
  * ## Introspection of runtime type-data
  *
  * It is also important for consumers of a RuntimeTypeContainer to
  * be able to fetch the list of run-time objects and their base types.
  * To this end, you can access a map from run-time storage types to
  * the user-provided "type names."
  *
  * ```cpp
  * // Fetch a set of string-tokens naming the base types of runtime objects:
  * auto runtimeBases = allMetadata.runtimeBaseTypes();
  *
  * // Return the type-names of objects that "inherit" a given runtime base type:
  * auto runtimeObjectTypes = allMetadata.runtimeTypeNames("RuntimeMetadata");
  *
  * ```
  */
class SMTKCORE_EXPORT RuntimeTypeContainer : public TypeContainer
{
public:
  /// Construct an empty RuntimeTypeContainer.
  RuntimeTypeContainer() = default;

  /// Construct a RuntimeTypeContainer whose contents are copied from an existing
  /// RuntimeTypeContainer.
  RuntimeTypeContainer(const RuntimeTypeContainer&);

  /// Move the contents of one TypeContainer into a new TypeContainer.
  RuntimeTypeContainer(RuntimeTypeContainer&&) = default;

  /// Copy the contents of an existing TypeContainer into this one.
  RuntimeTypeContainer& operator=(const RuntimeTypeContainer&);

  /// Move the contents of an existing TypeContainer into this one.
  RuntimeTypeContainer& operator=(RuntimeTypeContainer&&) = default;

  /// Construct a RuntimeTypeContainer instance from any number of elements. Elements
  /// are added in the order they appear in the constructor, so subsequent values
  /// for the same type will be ignored.
  template<
    typename Arg,
    typename... Args,
    typename std::enable_if<!std::is_base_of<TypeContainer, Arg>::value, int>::type = 0>
  RuntimeTypeContainer(const Arg& arg, const Args&... args)
  {
    insertAll(arg, args...);
  }

  ~RuntimeTypeContainer() override = default;

  /// Check if a Type is present in the TypeContainer.
  bool containsRuntime(smtk::string::Token declaredType) const
  {
    return (m_container.find(declaredType.id()) != m_container.end());
  }

  /// Insert a runtime \a RuntimeType instance into the TypeContainer.
  /// Note that if the type already exists in the container, the insertion will fail.
  template<typename RuntimeType, typename ActualType>
  bool insertRuntime(smtk::string::Token declaredType, const ActualType& value)
  {
    static_assert(
      std::is_convertible<ActualType*, RuntimeType*>::value,
      "Inserted object must inherit the requested base Type.");
    bool didInsert =
      m_container
        .emplace(std::make_pair(
          declaredType.id(),
#ifdef SMTK_HAVE_CXX_14
          std::make_unique<WrapperFor<RuntimeType>>(std::make_unique<ActualType>(value))
#else
          std::unique_ptr<Wrapper>(
            new WrapperFor<RuntimeType>(std::unique_ptr<ActualType>(new ActualType((value)))))
#endif
            ))
        .second;
    if (didInsert)
    {
      m_runtimeObjects[smtk::common::typeName<RuntimeType>()].insert(declaredType);
    }
    return didInsert;
  }

  /// Insert a \a Type instance into the RuntimeTypeContainer
  /// if it does not exist already or replace it if it does.
  template<typename RuntimeType, typename ActualType>
  bool insertOrAssignRuntime(smtk::string::Token declaredType, const ActualType& value)
  {
    if (this->containsRuntime(declaredType))
    {
      this->eraseRuntime(declaredType);
    }
    return this->insertRuntime<RuntimeType>(declaredType.id(), value);
  }

  /// Emplace a RuntimeType instance into the TypeContainer.
  template<typename RuntimeType, typename... Args>
  bool emplaceRuntime(smtk::string::Token declaredType, Args&&... args)
  {
    bool didInsert = m_container
                       .emplace(std::make_pair(
                         declaredType.id(),
#ifdef SMTK_HAVE_CXX_14
                         std::make_unique<WrapperFor<RuntimeType>>(
                           std::make_unique<RuntimeType>(std::forward<Args>(args)...))
#else
                         std::unique_ptr<Wrapper>(
                           new WrapperFor<RuntimeType>(std::unique_ptr<RuntimeType>(
                             new RuntimeType(std::forward<Args>(args)...))))
#endif
                           ))
                       .second;
    if (didInsert)
    {
      m_runtimeObjects[smtk::common::typeName<RuntimeType>()].insert(declaredType);
    }
    return didInsert;
  }

  /// Access a Type instance, and throw if it is not in the TypeContainer.
  template<typename RuntimeType>
  const RuntimeType& getRuntime(smtk::string::Token declaredType) const
  {
    auto search = m_container.find(declaredType.id());
    if (search == m_container.end())
    {
      throw BadTypeError(declaredType.data());
    }
    // TODO: Check that \a RuntimeType was the type used to insert \a declaredType.
    // Throw BadTypeError(smtk::common::typeName<RuntimeType>()) if not.

    return *(static_cast<WrapperFor<RuntimeType>*>(search->second.get()))->value;
  }

  /// For default-constructible types, access a RuntimeType instance, creating one if it
  /// is not in the RuntimeTypeContainer.
  template<typename RuntimeType>
  typename std::enable_if<std::is_default_constructible<RuntimeType>::value, RuntimeType&>::type
  getRuntime(smtk::string::Token declaredType) noexcept
  {
    auto search = m_container.find(declaredType.id());
    if (search == m_container.end())
    {
      search = m_container
                 .emplace(std::make_pair(
                   declaredType.id(),
#ifdef SMTK_HAVE_CXX_14
                   std::make_unique<WrapperFor<RuntimeType>>(std::make_unique<RuntimeType>())
#else
                   std::unique_ptr<Wrapper>(
                     new WrapperFor<RuntimeType>(std::unique_ptr<RuntimeType>(new RuntimeType)))
#endif
                     ))
                 .first;
    }
    // TODO: Check that \a RuntimeType was the type used to insert \a declaredType.
    // Throw BadTypeError(smtk::common::typeName<RuntimeType>()) if not.

    return *(static_cast<WrapperFor<RuntimeType>*>(search->second.get()))->value;
  }

  /// For non-default-constructible types, access a Type instance; throw if it is
  /// not in the TypeContainer.
  template<typename RuntimeType>
  typename std::enable_if<!std::is_default_constructible<RuntimeType>::value, RuntimeType&>::type
  getRuntime(smtk::string::Token declaredType)
  {
    auto search = m_container.find(declaredType.id());
    if (search == m_container.end())
    {
      throw BadTypeError(declaredType.data());
    }
    // TODO: Check that \a RuntimeType was the type used to insert \a declaredType.
    // Throw BadTypeError(smtk::common::typeName<RuntimeType>()) if not.

    return *(static_cast<WrapperFor<RuntimeType>*>(search->second.get()))->value;
  }

  /// Remove a specific type of object from the container.
  ///
  /// This is overridden from the base class to ensure m_runtimeObjects is maintained.
  template<typename Type>
  bool erase()
  {
    auto it = m_container.find(this->keyId<Type>());
    if (it != m_container.end())
    {
      // Check whether the object was indexed in m_runtimeObjects
      // and remove it if needed.
      auto baseTypeIt = m_runtimeObjects.find(it->second->objectType());
      if (baseTypeIt != m_runtimeObjects.end())
      {
        // Note that the only way erase<T>() can work is if the declared
        // type matches the declared type-name in it->first:
        baseTypeIt->second.erase(smtk::string::Token::fromHash(it->first));
      }
      // Now erase the container entry.
      m_container.erase(it);
      return true;
    }
    return false;
  }

  /// Remove a specific type of object from the container.
  bool eraseRuntime(smtk::string::Token declaredType)
  {
    auto it = m_container.find(declaredType.id());
    if (it != m_container.end())
    {
      // Check whether the object was indexed in m_runtimeObjects
      // and remove it if needed.
      auto baseTypeIt = m_runtimeObjects.find(it->second->objectType());
      if (baseTypeIt != m_runtimeObjects.end())
      {
        baseTypeIt->second.erase(declaredType);
      }
      // Now erase the container entry.
      m_container.erase(it);
      return true;
    }
    return false;
  }

  /// Erase all objects held by the container.
  void clear() noexcept
  {
    this->TypeContainer::clear();
    m_runtimeObjects.clear();
  }

  /// Report the base classes used to insert runtime objects.
  std::unordered_set<smtk::string::Token> runtimeBaseTypes();

  /// Report the declared type-names of objects inserted using the given \a base type
  std::unordered_set<smtk::string::Token> runtimeTypeNames(smtk::string::Token baseType);

  /// Report the declared type-names of objects inserted using the given \a base type
  template<typename RuntimeType>
  std::unordered_set<smtk::string::Token> runtimeTypeNames()
  {
    return this->runtimeTypeNames(smtk::common::typeName<RuntimeType>());
  }

protected:
  /// Map from RuntimeType type-names (i.e., the common base classes
  /// used to insert values) to declared types.
  std::unordered_map<smtk::string::Token, std::unordered_set<smtk::string::Token>> m_runtimeObjects;
};
} // namespace common
} // namespace smtk

#endif
