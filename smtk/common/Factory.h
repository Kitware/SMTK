//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_Factory_h
#define smtk_common_Factory_h

#include "smtk/CoreExports.h"
#include "smtk/TupleTraits.h"

#include "smtk/common/TypeName.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index_container.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

#include <memory>
#include <type_traits>
#include <unordered_set>

namespace smtk
{
namespace common
{
namespace factory
{
/// Tags for using Factory's convenience Interfaces.
struct SMTKCORE_EXPORT Type
{
};
struct SMTKCORE_EXPORT Name
{
};
struct SMTKCORE_EXPORT Index
{
};
template<typename... Args>
using Inputs = std::tuple<Args...>;
} // namespace factory

/**\brief A Factory is a class that constructs instances of registered classes
  *       which all inherit a common \a BaseType.
  *
  * A class can be created by keying off of its type (as a template parameter),
  * its type-index (an integer hash of its type name),
  * or its type name. Additionally, instance constructors with several input
  * signatures (InputTypes) of arbitrary length can be provided to the Factory
  * instance at compile-time.
  *
  * The factory provides two sets of calls:
  *
  * + methods that begin with `create` return unique pointers to the instances they create.
  * + methods that begin with `make` return shared pointers to the instances they create.
  *
  * The latter is sometimes necessary as some compilers are unable to properly convert
  * unique pointers to shared pointers (i.e., when an explicit default deleter is used).
  *
  * Here is an example of its use:
  *
  * ```c++
  * class Base
  * {
  * public:
  *   Base() {...}
  *   Base(int i) {...}
  *   Base(double d, const std::string& s) {...}
  *
  *   virtual ~Base() {}
  * };
  *
  * class Derived1 : public Base {...};
  * class Derived2 : public Base {...};
  *
  * ...
  *
  * Factory<Base, void, int, smtk::common::factory::Inputs<double, const std::string&> > factory;
  * factory.registerType<Derived1>();
  * factory.registerType<Derived2>();
  *
  * // Create via type
  * std::shared_ptr<Derived1> d_a = factory.make<Derived1>();
  * //   – or –
  * std::unique_ptr<Derived1> d_a = factory.create<Derived1>();
  *
  * // Create via index
  * std::shared_ptr<Base> d_b = factory.makeFromIndex(typeid(Derived1).hash_code());
  * //   – or –
  * std::unique_ptr<Base> d_b = factory.createFromIndex(typeid(Derived1).hash_code());
  *
  * // Create via name
  * std::shared_ptr<Base> d_c = factory.makeFromName("Derived1");
  * //   – or –
  * std::unique_ptr<Base> d_c = factory.createFromName("Derived1");
  *
  * // Create with arguments
  * std::unique_ptr<Base> d_d = factory.createFromName("Derived1", 3.14, std::string("hello"));
  *
  * // Create using convenience interface:
  * std::unique_ptr<Base> d_e = factory.get<smtk::common::factory::Index>()
  *                              .create(typeid(Derived1).hash_code(), 14);
  *
  * ```
  */
template<typename BaseType, typename... InputTypes>
class SMTK_ALWAYS_EXPORT Factory
{
  // Create functors declare a create method's input signature at class scope,
  // rather than templating on the signature at function scope. Doing so allows
  // us to disentangle the derived type template resolution from the input
  // parameter resolution, enabling us to fix down the specialization at
  // different points in the execution of the Factory's "create" calls.
  //
  // NOTE: the second template parameter is a dummy, needed because explicit
  // template specializations are not allowed within an unspecialized outer
  // class.
  template<typename InputType, bool = true>
  class CreateWithInput
  {
  public:
    // Because the input type is a class template, it does not fit the forwarding
    // reference signature. We therefore need to support lvalue and rvalue
    // signatures independently.
    template<typename Type>
    BaseType* operator()(InputType&& args) const
    {
      return new Type(args);
    }

    template<typename Type>
    BaseType* operator()(InputType& args) const
    {
      return new Type(args);
    }
  };

  // Create functors are specialized for tuple inputs. Tuples are used to convey
  // multi-parameter constructor signatures.
  template<typename... XInputTypes, bool dummy>
  class CreateWithInput<std::tuple<XInputTypes...>, dummy>
  {
  public:
    template<typename Type>
    BaseType* operator()(XInputTypes&&... args) const
    {
      return new Type(std::forward<XInputTypes>(args)...);
    }

    template<typename Type>
    BaseType* operator()(XInputTypes&... args) const
    {
      return new Type(args...);
    }
  };

  // A Metdata class that holds the create methods for a specific construction
  // signature (InputType, defined at class-template scope) and derived type to
  // create (Type, defined explicitly using the class's constructor). This class
  // is the composing element for the factory's Metadata instance.
  template<typename InputType, bool = true>
  class MetadataForInput
  {
  public:
    template<typename Type>
    MetadataForInput(identity<Type>)
      : m_rcreate([](InputType&& args) -> BaseType* {
        return CreateWithInput<InputType>().template operator()<Type>(
          std::forward<InputType>(args));
      })
      , m_lcreate([](InputType& args) -> BaseType* {
        return CreateWithInput<InputType>().template operator()<Type>(args);
      })
    {
    }

    // Support for lvalue and rvalue input parameters requires the use of two
    // functors.
    BaseType* create(InputType&& args) const { return m_rcreate(std::forward<InputType>(args)); }
    BaseType* create(InputType& args) const { return m_lcreate(args); }

  private:
    std::function<BaseType*(InputType&&)> m_rcreate;
    std::function<BaseType*(InputType&)> m_lcreate;
  };

  template<typename... XInputTypes, bool dummy>
  class MetadataForInput<std::tuple<XInputTypes...>, dummy>
  {
  public:
    template<typename Type>
    MetadataForInput(identity<Type>)
      : m_create([](XInputTypes&... args) -> BaseType* {
        return CreateWithInput<std::tuple<XInputTypes...>>().template operator()<Type>(args...);
      })
    {
    }

    BaseType* create(XInputTypes&... args) const { return m_create(args...); }

  private:
    std::function<BaseType*(XInputTypes&...)> m_create;
  };

  // A specialization for default constructors that accept no parameters, since
  // the above generic signatures are not quite generic enough to handle empty
  // parameter packs.
  template<bool dummy>
  class MetadataForInput<void, dummy>
  {
  public:
    template<typename Type>
    MetadataForInput(identity<Type>)
      : m_create([]() -> BaseType* { return new Type; })
    {
    }

    BaseType* create() const { return m_create(); }

  private:
    std::function<BaseType*()> m_create;
  };

  // MetadataForInputs is the composite of each type of MetadataForInput,
  // templated across input signatures. There is one instance for each derived
  // type that the Factory can create.
  template<typename... XInputTypes>
  class MetadataForInputs : public MetadataForInput<XInputTypes>...
  {
  public:
    // Once we support c++17, we can expose parent create methods and avoid
    // the need to static_cast to the right MetadataForInput type. This will
    // give us more flexibility by allowing create() to implicitly cast its
    // input parameters.
    // using XInputTypes::create...;

    template<typename Type>
    MetadataForInputs(identity<Type> identity)
      : MetadataForInput<XInputTypes>(identity)...
      , m_index(typeid(Type).hash_code())
      , m_name(smtk::common::typeName<Type>())
    {
    }

    const std::size_t& index() const { return m_index; }
    const std::string& name() const { return m_name; }

  private:
    std::size_t m_index;
    std::string m_name;
  };

  // Our Metadata type has an unwieldy name, so we create a convenience typedef
  // for readability.
  typedef MetadataForInputs<typename recursive<std::remove_reference, InputTypes>::type...>
    Metadata;

  // Internally, we hold our metadata instances using a boost multiindex array.
  // This allows us to access metadata in constant time using either the type
  // index or type name as a key.

  struct ByIndex
  {
  };
  struct ByName
  {
  };

  typedef boost::multi_index_container<
    Metadata,
    boost::multi_index::indexed_by<
      boost::multi_index::hashed_unique<
        boost::multi_index::tag<ByIndex>,
        boost::multi_index::const_mem_fun<Metadata, const std::size_t&, &Metadata::index>>,
      boost::multi_index::hashed_unique<
        boost::multi_index::tag<ByName>,
        boost::multi_index::const_mem_fun<Metadata, const std::string&, &Metadata::name>>>>
    Container;

  // A convenience Interface is provided that singles out one of the three modes
  // of creation: by type, by type name and by type index.
  template<typename TagType, bool>
  class Interface;

public:
  /// Register a Type to the factory.
  template<typename Type>
  bool registerType()
  {
    static_assert(
      std::is_base_of<BaseType, Type>::value,
      "Cannot register a type that does not inherit from the Factory's BaseType.");
    return registerType(Metadata(identity<Type>()));
  }

  /// Register a Typeusing the Type's Metadata.
  bool registerType(Metadata&& metadata)
  {
    return m_metadata.emplace(std::forward<Metadata>(metadata)).second;
  }

  /// Register a tuple of Types.
  template<typename Tuple>
  bool registerTypes()
  {
    return registerTypes<0, Tuple>();
  }

  /// Unregister a Type.
  template<typename Type>
  bool unregisterType()
  {
    return unregisterType(typeid(Type).hash_code());
  }

  /// Unregister a Type using its type name.
  bool unregisterType(const std::string& typeName)
  {
    return m_metadata.template get<ByName>().erase(typeName) > 0;
  }

  /// Unregister a Type using its type index.
  bool unregisterType(const std::size_t& typeIndex)
  {
    return m_metadata.template get<ByIndex>().erase(typeIndex) > 0;
  }

  /// Unregister a tuple of Types.
  template<typename Tuple>
  bool unregisterTypes()
  {
    return unregisterTypes<0, Tuple>();
  }

  /// Determine whether or not a Type is available.
  template<typename Type>
  bool contains() const
  {
    return contains(typeid(Type).hash_code());
  }

  /// Determine whether or not a Type is available using its type name.
  bool contains(const std::string& typeName) const
  {
    auto& byName = m_metadata.template get<ByName>();
    return byName.find(typeName) != byName.end();
  }

  /// Determine whether or not a Type  is available using its type index.
  bool contains(const std::size_t typeIndex) const
  {
    auto& byIndex = m_metadata.template get<ByIndex>();
    return byIndex.find(typeIndex) != byIndex.end();
  }

  /// Create an instance of a Type.
  template<typename Type, typename... Args>
  std::unique_ptr<Type> create(Args&&... args) const
  {
    return std::unique_ptr<Type>{ static_cast<Type*>(
      this->createFromIndex_(typeid(Type).hash_code(), std::forward<Args>(args)...).release()) };
  }

  /// Create an instance of a Type using its type name.
  template<typename... Args>
  std::unique_ptr<BaseType> createFromName(const std::string& typeName, Args&&... args) const
  {
    auto& byName = m_metadata.template get<ByName>();
    auto search = byName.find(typeName);
    if (search != byName.end())
    {
      return createFromIndex_(search->index(), std::forward<Args>(args)...);
    }
    return std::unique_ptr<BaseType>();
  }

  /// Create an instance of a Type using its type name.
  template<typename... Args>
  std::unique_ptr<BaseType> createFromIndex(const std::size_t& typeIndex, Args&&... args) const
  {
    return createFromIndex_<Args...>(typeIndex, std::forward<Args>(args)...);
  }

  /// Make a shared instance of a Type.
  template<typename Type, typename... Args>
  std::shared_ptr<Type> make(Args&&... args) const
  {
    return std::shared_ptr<Type>{ static_cast<Type*>(
      this->createFromIndex_(typeid(Type).hash_code(), std::forward<Args>(args)...).release()) };
  }

  /// Make a shared instance of a Type using its type name.
  template<typename... Args>
  std::shared_ptr<BaseType> makeFromName(const std::string& typeName, Args&&... args) const
  {
    auto& byName = m_metadata.template get<ByName>();
    auto search = byName.find(typeName);
    if (search != byName.end())
    {
      return createFromIndex_(search->index(), std::forward<Args>(args)...);
    }
    return std::shared_ptr<BaseType>();
  }

  /// Create an instance of a Type using its type name.
  template<typename... Args>
  std::shared_ptr<BaseType> makeFromIndex(const std::size_t& typeIndex, Args&&... args) const
  {
    return createFromIndex_<Args...>(typeIndex, std::forward<Args>(args)...);
  }

  /// Access a convenience Interface for creating objects that singles out one of
  /// the three modes of creation: by type, by type name and by type index.
  template<typename TagType>
  Interface<TagType, true> get() const
  {
    return Interface<TagType, true>(*this);
  }

private:
  // The implementation for creating an instance from a type index is split into
  // three parts to handle the following three input parameter configurations:
  //
  // 1. No input parameters: remove arguments from the calls, since references to
  //    void are disallowed
  // 2. One input parameter: access the metadata using the input parameter type
  // 3. Multiple input parameters: combine the parameters into a tuple and access
  //    the metadata using the tuple type
  template<typename... Args>
  typename std::enable_if<sizeof...(Args) == 0, std::unique_ptr<BaseType>>::type createFromIndex_(
    const std::size_t& typeIndex) const
  {
    auto& byIndex = m_metadata.template get<ByIndex>();
    auto search = byIndex.find(typeIndex);
    if (search != byIndex.end())
    {
      return std::unique_ptr<BaseType>{
        static_cast<const MetadataForInput<void>&>(*search).create()
      };
    }
    return std::unique_ptr<BaseType>();
  }
  template<typename Arg>
  std::unique_ptr<BaseType> createFromIndex_(const std::size_t& typeIndex, Arg&& arg) const
  {
    auto& byIndex = m_metadata.template get<ByIndex>();
    auto search = byIndex.find(typeIndex);
    if (search != byIndex.end())
    {
      const Metadata& metadata = *search;
      return std::unique_ptr<BaseType>{
        static_cast<const MetadataForInput<typename std::remove_reference<Arg>::type>&>(metadata)
          .create(std::forward<Arg>(arg))
      };
    }
    return std::unique_ptr<BaseType>();
  }
  template<typename... Args>
  typename std::enable_if<(sizeof...(Args) > 0), std::unique_ptr<BaseType>>::type createFromIndex_(
    const std::size_t& typeIndex,
    Args&&... args) const
  {
    auto& byIndex = m_metadata.template get<ByIndex>();
    auto search = byIndex.find(typeIndex);
    if (search != byIndex.end())
    {
      return std::unique_ptr<BaseType>{
        static_cast<
          const MetadataForInput<std::tuple<typename std::remove_reference<Args>::type...>>&>(
          *search)
          .create(args...)
      };
    }
    return std::unique_ptr<BaseType>();
  }

  // Helper methods for walking the types in a tuple.
  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type registerTypes()
  {
    bool registered = this->registerType<typename std::tuple_element<I, Tuple>::type>();
    return registered && registerTypes<I + 1, Tuple>();
  }
  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type registerTypes()
  {
    return true;
  }
  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type unregisterTypes()
  {
    bool unregistered = this->unregisterType<typename std::tuple_element<I, Tuple>::type>();
    return unregistered && unregisterTypes<I + 1, Tuple>();
  }
  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type unregisterTypes()
  {
    return true;
  }

  // A common base to the interface classes that sets copy/assignment
  // restrictions and hold a reference to the parent Factory instance.
  class InterfaceBase
  {
  public:
    InterfaceBase(const InterfaceBase&) = delete;
    InterfaceBase& operator=(const InterfaceBase&) = delete;
    InterfaceBase& operator=(InterfaceBase&&) = delete;

  protected:
    friend class Factory;

    InterfaceBase(const Factory& factory)
      : m_factory(factory)
    {
    }
    InterfaceBase(InterfaceBase&&) noexcept = default;

    const Factory& m_factory;
  };

  // Specialization that uses the template Type to select the right metadata
  // type.
  template<bool dummy>
  class Interface<factory::Type, dummy> : private InterfaceBase
  {
  public:
    template<typename Type>
    bool contains() const
    {
      return InterfaceBase::m_factory.template contains<Type>();
    }

    template<typename Type, typename... Args>
    std::unique_ptr<Type> create(Args&&... args) const
    {
      return std::unique_ptr<Type>{ static_cast<Type*>(
        InterfaceBase::m_factory
          .createFromIndex(typeid(Type).hash_code(), std::forward<Args>(args)...)
          .release()) };
    }

  private:
    friend class Factory;

    Interface(const Factory& factory)
      : InterfaceBase(factory)
    {
    }
    Interface(Interface&&) noexcept = default;
  };

  // Specialization that uses the type name to select the right metadata type.
  template<bool dummy>
  class Interface<factory::Name, dummy> : private InterfaceBase
  {
  public:
    bool contains(const std::string& typeName) const
    {
      return InterfaceBase::m_factory.contains(typeName);
    }

    template<typename... Args>
    std::unique_ptr<BaseType> create(const std::string& typeName, Args&&... args) const
    {
      return InterfaceBase::m_factory.template createFromName<Args...>(
        typeName, std::forward<Args>(args)...);
    }

  private:
    friend class Factory;

    Interface(const Factory& factory)
      : InterfaceBase(factory)
    {
    }
    Interface(Interface&&) noexcept = default;
  };

  // Specialization that uses the type index to select the right metadata type.
  template<bool dummy>
  class Interface<factory::Index, dummy> : private InterfaceBase
  {
  public:
    bool contains(const std::size_t& typeIndex) const
    {
      return InterfaceBase::m_factory.contains(typeIndex);
    }

    template<typename... Args>
    std::unique_ptr<BaseType> create(const std::size_t& typeIndex, Args&&... args) const
    {
      return InterfaceBase::m_factory.template createFromIndex<Args...>(
        typeIndex, std::forward<Args>(args)...);
    }

  private:
    friend class Factory;

    Interface(const Factory& factory)
      : InterfaceBase(factory)
    {
    }
    Interface(Interface&&) noexcept = default;
  };

  // The container of metadata.
  Container m_metadata;
};
} // namespace common
} // namespace smtk

#endif
