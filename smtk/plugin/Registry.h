//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_plugin_Registry_h
#define smtk_plugin_Registry_h

#include "smtk/common/CompilerInformation.h"

#include "smtk/CoreExports.h"
#include "smtk/TupleTraits.h"

#include <map>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

#ifdef SMTK_MSVC
#include "smtk/plugin/Sentinel.h"
#endif

namespace smtk
{
namespace plugin
{

/// Resources and operations both have optional managers that can be used as an
/// interface for their construction, to track their existence and to attach
/// observers to their actions. Additional modeling sessions and other plugins
/// can augment the functionality of these managers, but they must register
/// themselves with a manager to do so. Additionally, plugins that can be
/// unloaded must remove themselves from managers, or else managers may try to
/// call routines that are no longer in memory. To facilitate this registration
/// process, we introduce a Registry that accepts as template parameters a
/// Registrar, or class with pairs of "registerTo" and "unregisterFrom" methods
/// for each manager type to which the plugin  registers, and a variable number
/// of manager classes. The Registrar's registration methods are called for
/// each appropriate manager upon the Registry's construction, and the
/// Registrar's unregister methods are called for each manager (if they are
/// still in memory) upon the Registry's deletion.
///
/// The Registry class is implemented via templates to facilitate a) the
/// registration to only a subset of available managers, b) the abilty to
/// accommodate additional managers in the future without incurring a runtime
/// dispatch penalty, c) construction of an object that can call new code
/// during its own destruction (pure virtual methods should not be called in a
/// base class's destructor), and d) to keep homologous Manager classes from
/// being entwined via a superfluous base class.

namespace detail
{
/// This template class takes as arguments a Registrar and a Manager. It
/// performs a compile-time check that the Registrar either registers to and
/// unregisters from the manager type, or does neither (this is a check to
/// ensure that Registrars clean up after themselves when their plugins go
/// away). The resulting compile-time value "type" reflects whether or not the
/// Registrar registers to the Manager.
template<typename Registrar, typename Manager>
class RegistersTo
{
  template<typename X>
  static std::true_type testRegisterTo(
    decltype(std::declval<Registrar>().registerTo(std::declval<std::shared_ptr<X>>()))*);
  template<typename X>
  static std::false_type testRegisterTo(...);

  template<typename X>
  static std::true_type testUnregisterFrom(
    decltype(std::declval<Registrar>().unregisterFrom(std::declval<std::shared_ptr<X>>()))*);
  template<typename X>
  static std::false_type testUnregisterFrom(...);

  static_assert(
    std::is_same<
      decltype(testRegisterTo<Manager>(nullptr)),
      decltype(testUnregisterFrom<Manager>(nullptr))>::value,
    "Registrar must be able to both register and unregister from a manager.");

public:
  using type = decltype(testRegisterTo<Manager>(nullptr));
};

/// ManagerCount is a singleton map of Schwarz counters for pairs of Manager
/// instances and Registrar types.
class SMTKCORE_EXPORT ManagerCount
{
public:
  static ManagerCount& instance() { return m_instance; }

  template<typename Registrar, typename Manager>
  std::size_t& operator[](Manager* manager)
  {
    return this->operator[](
      std::make_pair(manager, std::type_index(typeid(Registrar)).hash_code()));
  }

private:
  ManagerCount();
  ~ManagerCount();

  std::size_t& operator[](const std::pair<void*, std::size_t>& key);

  static ManagerCount m_instance;

  struct Internals;
  Internals* m_internals;
};

/// MaybeRegister accepts a Registrar, a Manager and a boolean type. If the
/// boolean type is false, then the Registrar is not associated with the
/// Manager and nothing happens. If the type is true, then a Schwarz counter
/// for each Manager is set up to call the Registrar exactly once when a
/// Registry enters the program's scope and again when the Registry leaves
/// scope.
template<typename Registrar, typename Manager, typename Choice>
class MaybeRegister;

template<typename Registrar, typename Manager>
class MaybeRegister<Registrar, Manager, std::false_type>
{
public:
  MaybeRegister(const std::shared_ptr<Manager>&) {}
  virtual ~MaybeRegister() = default;

  template<typename M>
  bool contains(const std::shared_ptr<M>&) const
  {
    return false;
  }
};

template<typename Registrar, typename Manager>
class SMTK_ALWAYS_EXPORT MaybeRegister<Registrar, Manager, std::true_type>
{
public:
  MaybeRegister(const std::shared_ptr<Manager>& manager)
    : m_Manager(manager)
  {
    if (ManagerCount::instance().operator[]<Registrar, Manager>(manager.get())++ == 0)
    {
      (void)Registrar().registerTo(m_Manager);
    }
  }

  ~MaybeRegister()
  {
    if (
      m_Manager && --ManagerCount::instance().operator[]<Registrar, Manager>(m_Manager.get()) == 0)
    {
      (void)Registrar().unregisterFrom(m_Manager);
    }
  }

  template<typename M>
  bool contains(const std::shared_ptr<M>&) const
  {
    return false;
  }

  bool contains(const std::shared_ptr<Manager>& manager) const { return manager == m_Manager; }

private:
  std::shared_ptr<Manager> m_Manager;
};

/// Registrars may declare dependencies to other Registrars by defining a type
/// "Dependencies" as a std::tuple of additional Registrars. This declaration is
/// optional; if omitted, then a Registrar will only register the things it
/// explicitly lists. The detail::Dependencies class either returns a
/// Registrar's dependencies or an empty tuple if none are defined.
template<typename Registrar>
class Dependencies
{
  template<typename R>
  static typename R::Dependencies deps(typename R::Dependencies*);

  template<typename R>
  static std::tuple<> deps(...);

  template<typename>
  struct is_tuple : std::false_type
  {
  };

  template<typename... T>
  struct is_tuple<std::tuple<T...>> : std::true_type
  {
  };

public:
  using type = decltype(deps<Registrar>(nullptr));

  static_assert(is_tuple<type>::value, "Registrar dependencies must be given as a std::tuple.");
};

/// AllDependenciesWithDuplicates is an intermediate step in constructing a flat
/// list of all of the dependencies of a Registrar.
template<typename... Registrars>
struct AllDependenciesWithDuplicates;

template<typename Registrar, typename... Registrars>
struct AllDependenciesWithDuplicates<Registrar, Registrars...>
{
  // We are combining:
  //  a) the dependencies of Registrar,
  //  b) the dependencies of Registrar's dependencies, and
  //  c) the dependencies of the remaining Registrars in the parameter pack.
  using type = decltype(std::tuple_cat(
    std::declval<typename Dependencies<Registrar>::type>(),
    std::declval<
      typename AllDependenciesWithDuplicates<typename Dependencies<Registrar>::type>::type>(),
    std::declval<typename AllDependenciesWithDuplicates<Registrars...>::type>()));
};

// Specialization to strip out the contents of tuples.
template<typename T>
struct AllDependenciesWithDuplicates<std::tuple<T>>
{
  using type = typename AllDependenciesWithDuplicates<T>::type;
};

// Specialization to handle Registrars with no dependencies.
template<>
struct AllDependenciesWithDuplicates<std::tuple<>>
{
  using type = std::tuple<>;
};

// Specialization to end the list walk.
template<>
struct AllDependenciesWithDuplicates<>
{
  using type = std::tuple<>;
};

/// AllDependencies accepts a Registrar and returns a tuple of all of its
/// dependencies. It performs a recursive search across its dependent
/// Registrars.
template<typename Registrar>
struct AllDependencies
{
  using type =
    decltype(typename smtk::remove_from_tuple<
             Registrar,
             typename smtk::unique_tuple<typename smtk::flatten_tuple<
               typename AllDependenciesWithDuplicates<Registrar>::type>::type>::type>::type());
};
} // namespace detail

/// Since the Registry class is a template class, we introduce a non-templated
/// base class RegistryBase so that Registries can be manipulated in aggregate.
/// RegistryBase also holds the registries associated with the Registry's
/// dependencies.
class RegistryBase
{
public:
  virtual ~RegistryBase()
  {
    for (auto it = m_registries.begin(); it != m_registries.end(); ++it)
    {
      delete (*it);
    }
  }

protected:
  std::vector<RegistryBase*> m_registries;
};

/// Given a Registrar (a class that describes how to register and unregister a
/// plugin) and one or more Managers, the Registry is a class composed of
/// MaybeRegister-s for the Registrar and each Manager (and also a base class
/// RegistryBase for good measure). The lifetime of a plugin's registration to
/// the managers passed to the Registry is then guaranteed to be at least as
/// long as the lifetime of the Registry. Multiple Registries can exist with
/// overlapping registration conditions; multiple registration and early
/// unregistration are avoided via the use of Schwarz counters in
/// MaybeRegister.
///
/// Both gcc and clang are savvy to template class descriptions using parameter
/// packs. MSVC isn't quite there yet. The following code is bifurcated
/// according to compiler type because, even though the two code paths produce
/// functionally equivalent code, the first implementation does so with much
/// less compile-time (and library size) overhead. Eventually, only the first
/// implementation should be used.

#ifndef SMTK_MSVC

template<typename Registrar, typename Manager, typename... T>
class Registry
  : public detail::
      MaybeRegister<Registrar, Manager, typename detail::RegistersTo<Registrar, Manager>::type>
  , public detail::MaybeRegister<Registrar, T, typename detail::RegistersTo<Registrar, T>::type>...
  , public RegistryBase
{
public:
  Registry(const std::shared_ptr<Manager>& manager, const std::shared_ptr<T>&... managers)
    : detail::
        MaybeRegister<Registrar, Manager, typename detail::RegistersTo<Registrar, Manager>::type>(
          manager)
    , detail::MaybeRegister<Registrar, T, typename detail::RegistersTo<Registrar, T>::type>(
        managers)...
    , RegistryBase()
  {
    this->registerDependencies<0, typename detail::AllDependencies<Registrar>::type>(
      manager, std::forward<const std::shared_ptr<T>&>(managers)...);
  }

  template<typename M>
  bool contains(const std::shared_ptr<M>& m)
  {
    auto tmp = dynamic_cast<
      detail::MaybeRegister<Registrar, M, typename detail::RegistersTo<Registrar, M>::type>*>(this);
    return tmp != nullptr && tmp->contains(m);
  }

  ~Registry() override = default;

  typedef int DoNotRegisterDependencies;

  Registry(
    DoNotRegisterDependencies,
    const std::shared_ptr<Manager>& manager,
    const std::shared_ptr<T>&... managers)
    : detail::
        MaybeRegister<Registrar, Manager, typename detail::RegistersTo<Registrar, Manager>::type>(
          manager)
    , detail::MaybeRegister<Registrar, T, typename detail::RegistersTo<Registrar, T>::type>(
        managers)...
    , RegistryBase()
  {
  }

private:
  template<std::size_t I, typename Tuple>
  typename std::enable_if<I != std::tuple_size<Tuple>::value>::type registerDependencies(
    const std::shared_ptr<Manager>& manager,
    const std::shared_ptr<T>&... managers)
  {
    m_registries.push_back(new Registry<typename std::tuple_element<I, Tuple>::type, Manager, T...>(
      0, manager, std::forward<const std::shared_ptr<T>&>(managers)...));
    return registerDependencies<I + 1, Tuple>(
      manager, std::forward<const std::shared_ptr<T>&>(managers)...);
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I == std::tuple_size<Tuple>::value>::type registerDependencies(
    const std::shared_ptr<Manager>&,
    const std::shared_ptr<T>&...)
  {
    return;
  }
};

#else

template<typename Registrar, typename Manager = detail::Sentinel, typename... T>
class Registry
  : public detail::
      MaybeRegister<Registrar, Manager, typename detail::RegistersTo<Registrar, Manager>::type>
  , public Registry<Registrar, T...>
{
public:
  Registry(const std::shared_ptr<Manager>& manager, const std::shared_ptr<T>&... managers)
    : detail::
        MaybeRegister<Registrar, Manager, typename detail::RegistersTo<Registrar, Manager>::type>(
          manager)
    , Registry<Registrar, T...>(managers...)
  {
    this->registerDependencies<0, typename detail::AllDependencies<Registrar>::type>(
      manager, std::forward<const std::shared_ptr<T>&>(managers)...);
  }

  ~Registry() override = default;

  template<typename M>
  bool contains(const std::shared_ptr<M>& m)
  {
    auto tmp = dynamic_cast<
      detail::MaybeRegister<Registrar, M, typename detail::RegistersTo<Registrar, M>::type>*>(this);
    return tmp != nullptr && tmp->contains(m);
  }

  Registry(int, const std::shared_ptr<Manager>& manager, const std::shared_ptr<T>&... managers)
    : detail::
        MaybeRegister<Registrar, Manager, typename detail::RegistersTo<Registrar, Manager>::type>(
          manager)
    , Registry<Registrar, T...>(managers...)
  {
  }

private:
  template<std::size_t I, typename Tuple>
  typename std::enable_if<I != std::tuple_size<Tuple>::value>::type registerDependencies(
    const std::shared_ptr<Manager>& manager,
    const std::shared_ptr<T>&... managers)
  {
    m_registries.push_back(new Registry<typename std::tuple_element<I, Tuple>::type, Manager, T...>(
      0, manager, std::forward<const std::shared_ptr<T>&>(managers)...));
    return registerDependencies<I + 1, Tuple>(
      manager, std::forward<const std::shared_ptr<T>&>(managers)...);
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I == std::tuple_size<Tuple>::value>::type registerDependencies(
    const std::shared_ptr<Manager>&,
    const std::shared_ptr<T>&...)
  {
    return;
  }
};

template<typename Registrar>
class Registry<Registrar, detail::Sentinel> : public RegistryBase
{
public:
  Registry() = default;

  ~Registry() override = default;
};

#endif

template<typename Registrar, typename... Manager>
static Registry<Registrar, Manager...> addToManagers(const std::shared_ptr<Manager>&... manager)
{
  return Registry<Registrar, Manager...>(manager...);
}

} // namespace plugin
} // namespace smtk

#endif
