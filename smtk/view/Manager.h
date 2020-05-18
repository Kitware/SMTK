//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_Manager_h
#define smtk_view_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/TypeName.h"

#include "smtk/view/BadgeFactory.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/IconFactory.h"
#include "smtk/view/PhraseModelFactory.h"
#include "smtk/view/ViewWidgetFactory.h"

#include <array>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace smtk
{
namespace view
{
/// A view Manager is responsible for creating new views (eventually) as well as
/// view components such as PhraseModels and SubPhraseGenerators.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypeMacroBase(smtk::view::Manager);
  smtkCreateMacro(smtk::view::Manager);

  virtual ~Manager();

public:
  ViewWidgetFactory& viewWidgetFactory() { return m_viewWidgetFactory; }
  const ViewWidgetFactory& viewWidgetFactory() const { return m_viewWidgetFactory; }

  PhraseModelFactory& phraseModelFactory() { return m_phraseModelFactory; }
  const PhraseModelFactory& phraseModelFactory() const { return m_phraseModelFactory; }

private:
  PhraseModelFactory m_phraseModelFactory;
  ViewWidgetFactory m_viewWidgetFactory;

public:
  // ------ SubphraseGenerator ------
  using SubphraseGeneratorConstructor = std::function<SubphraseGeneratorPtr(void)>;

  /// Register a SubphraseGenerator identified by its class type.
  template <typename SubphraseGeneratorType>
  bool registerSubphraseGenerator();

  /// Register a SubphraseGenerator identified by its class type and type name.
  template <typename SubphraseGeneratorType>
  bool registerSubphraseGenerator(const std::string&);

  /// Register a tuple of views identified by their class types.
  template <typename Tuple>
  bool registerSubphraseGenerators()
  {
    return Manager::registerSubphraseGenerators<0, Tuple>();
  }

  /// Register a tuple of views identified by their class types and type
  /// names.
  template <typename Tuple>
  bool registerSubphraseGenerators(
    const std::array<std::string, std::tuple_size<Tuple>::value>& typeNames)
  {
    return Manager::registerSubphraseGenerators<0, Tuple>(typeNames);
  }

  /// Unregister a SubphraseGenerator identified by its class type.
  template <typename SubphraseGeneratorType>
  bool unregisterSubphraseGenerator();

  /// Unregister a SubphraseGenerator identified by its type name.
  bool unregisterSubphraseGenerator(const std::string&);

  // Unregister a tuple of SubphraseGenerators identified by their class types.
  template <typename Tuple>
  bool unregisterSubphraseGenerators()
  {
    return Manager::unregisterSubphraseGenerators<0, Tuple>();
  }

  /// Construct a SubphraseGenerator from a configuration component.
  std::shared_ptr<smtk::view::SubphraseGenerator> createSubphrase(const Configuration::Component*);

  /// Construct a SubphraseGenerator identified by its type name.
  std::shared_ptr<smtk::view::SubphraseGenerator> createSubphrase(const std::string&);

  /// Construct a SubphraseGenerator identified by its class type.
  template <typename SubphraseGeneratorType>
  smtk::shared_ptr<SubphraseGeneratorType> createSubphrase();

protected:
  Manager();

private:
  // ------ SubphraseGenerator ------
  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  registerSubphraseGenerators()
  {
    bool registered =
      this->registerSubphraseGenerator<typename std::tuple_element<I, Tuple>::type>();
    return registered && Manager::registerSubphraseGenerators<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  registerSubphraseGenerators()
  {
    return true;
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  registerSubphraseGenerators(
    const std::array<std::string, std::tuple_size<Tuple>::value>& typeNames)
  {
    bool registered = this->registerSubphraseGenerator<typename std::tuple_element<I, Tuple>::type>(
      typeNames.at(I));
    return registered && Manager::registerSubphraseGenerators<I + 1, Tuple>(typeNames);
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  registerSubphraseGenerators(const std::array<std::string, std::tuple_size<Tuple>::value>&)
  {
    return true;
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  unregisterSubphraseGenerators()
  {
    bool unregistered =
      this->unregisterSubphraseGenerator<typename std::tuple_element<I, Tuple>::type>();
    return unregistered && Manager::unregisterSubphraseGenerators<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  unregisterSubphraseGenerators()
  {
    return true;
  }

  /// A container for all registered SubphraseGenerator constructors.
  std::map<std::string, SubphraseGeneratorConstructor> m_subphraseGenerators;

public:
  // ------ Badges for phrase models ------
  BadgeFactory& badgeFactory() { return m_badgeFactory; }
  const BadgeFactory& badgeFactory() const { return m_badgeFactory; }

private:
  BadgeFactory m_badgeFactory;

public:
  // ------ Icons for persistent object classes ------
  IconFactory& iconFactory() { return m_iconFactory; }
  const IconFactory& iconFactory() const { return m_iconFactory; }

private:
  IconFactory m_iconFactory;
};

// ------ SubphraseGenerator ------
template <typename SubphraseGeneratorType>
bool Manager::unregisterSubphraseGenerator()
{
  return this->unregisterSubphraseGenerator(smtk::common::typeName<SubphraseGeneratorType>());
}

template <typename SubphraseGeneratorType>
smtk::shared_ptr<SubphraseGeneratorType> Manager::createSubphrase()
{
  return smtk::static_pointer_cast<SubphraseGeneratorType>(
    this->createSubphrase(smtk::common::typeName<SubphraseGeneratorType>()));
}

template <typename SubphraseGeneratorType>
bool Manager::registerSubphraseGenerator()
{
  return Manager::registerSubphraseGenerator<SubphraseGeneratorType>(
    smtk::common::typeName<SubphraseGeneratorType>());
}

template <typename SubphraseGeneratorType>
bool Manager::registerSubphraseGenerator(const std::string& typeName)
{
  // see if already exists:
  if (m_subphraseGenerators.find(typeName) == m_subphraseGenerators.end())
  {
    // Must wrap: m_subphraseGenerators[typeName] = SubphraseGeneratorType::create;
    m_subphraseGenerators[typeName] = []() { return SubphraseGeneratorType::create(); };
    return true;
  }
  return false;
}
}
}

#endif // smtk_view_Manager_h
