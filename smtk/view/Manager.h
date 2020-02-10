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

#include "smtk/view/BaseView.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/IconFactory.h"
#include "smtk/view/Information.h"

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

  // ------ ViewWidget ------
  using ViewWidgetConstructor =
    std::function<smtk::view::BaseView*(const smtk::view::Information& info)>;

  /// Register a widget identified by its class type.
  template <typename ViewWidgetType>
  bool registerViewWidget();

  /// Register a widget identified by its class type and type index.
  template <typename ViewWidgetType>
  bool registerViewWidget(std::size_t);

  /// Register a tuple of views identified by their class types.
  template <typename Tuple>
  bool registerViewWidgets()
  {
    return Manager::registerViewWidgets<0, Tuple>();
  }

  /// Register a tuple of views identified by their class types and type
  /// indices.
  template <typename Tuple>
  bool registerViewWidgets(
    const std::array<std::size_t, std::tuple_size<Tuple>::value>& typeIndices)
  {
    return Manager::registerViewWidgets<0, Tuple>(typeIndices);
  }

  /// Unregister a widget identified by its class type.
  template <typename ViewWidgetType>
  bool unregisterViewWidget();

  /// Unregister a ViewWidget identified by its type index.
  bool unregisterViewWidget(std::size_t);

  // Unregister a tuple of ViewWidgets identified by their class types.
  template <typename Tuple>
  bool unregisterViewWidgets()
  {
    return Manager::unregisterViewWidgets<0, Tuple>();
  }

  /// Construct a ViewWidget identified by its class type.
  template <typename ViewWidgetType>
  smtk::view::BaseView* createViewWidget(const smtk::view::Information& info);

  /// Construct a ViewWidget identified by its type index.
  smtk::view::BaseView* createViewWidget(std::size_t, const smtk::view::Information& info);

  /// Construct a ViewWidget identified by its alias.
  smtk::view::BaseView* createViewWidget(
    const std::string& alias, const smtk::view::Information& info);

  /// Can we find a ViewWidget to construct?
  bool hasViewWidget(const std::string&) const;

  template <typename ViewWidgetType>
  void addWidgetAlias(const std::string& alias)
  {
    addWidgetAlias(smtk::view::typeIndex<ViewWidgetType>(), alias);
  }

  /// Add an alternative constructor name for a view widget.
  void addWidgetAlias(std::size_t, const std::string&);

private:
  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  registerViewWidgets()
  {
    bool registered = this->registerViewWidget<typename std::tuple_element<I, Tuple>::type>();
    return registered && Manager::registerViewWidgets<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  registerViewWidgets()
  {
    return true;
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  unregisterViewWidgets()
  {
    bool unregistered = this->unregisterViewWidget<typename std::tuple_element<I, Tuple>::type>();
    return unregistered && Manager::unregisterViewWidgets<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  unregisterViewWidgets()
  {
    return true;
  }

  /// utility, retrieve a matching ViewWidgetConstructor
  ViewWidgetConstructor getViewWidgetConstructor(const std::string& alias) const;
  /// utility, retrieve a matching ViewWidgetConstructor
  ViewWidgetConstructor getViewWidgetConstructor(std::size_t typeIndex) const;
  /// A container for all registered ViewWidget constructors.
  std::map<std::size_t, ViewWidgetConstructor> m_viewWidgets;
  /// Alternate type names for the constructors.
  std::map<std::string, std::size_t> m_altViewWidgetNames;

public:
  // ------ PhraseModel ------
  using PhraseModelConstructor = std::function<PhraseModelPtr(void)>;

  /// Register a PhraseModel identified by its class type.
  template <typename PhraseModelType>
  bool registerPhraseModel();

  /// Register a PhraseModel identified by its class type and type name.
  template <typename PhraseModelType>
  bool registerPhraseModel(const std::string&);

  /// Register a tuple of PhraseModels identified by their class types.
  template <typename Tuple>
  bool registerPhraseModels()
  {
    return Manager::registerPhraseModels<0, Tuple>();
  }

  /// Register a tuple of PhraseModels identified by their class types and type
  /// names.
  template <typename Tuple>
  bool registerPhraseModels(const std::array<std::string, std::tuple_size<Tuple>::value>& typeNames)
  {
    return Manager::registerPhraseModels<0, Tuple>(typeNames);
  }

  /// Unregister a PhraseModel identified by its class type.
  template <typename PhraseModelType>
  bool unregisterPhraseModel();

  /// Unregister a PhraseModel identified by its type name.
  bool unregisterPhraseModel(const std::string&);

  // Unregister a tuple of PhraseModels identified by their class types.
  template <typename Tuple>
  bool unregisterPhraseModels()
  {
    return Manager::unregisterPhraseModels<0, Tuple>();
  }

  /// Construct a PhraseModel identified by its type name.
  std::shared_ptr<smtk::view::PhraseModel> create(const std::string&);

  /// Construct a PhraseModel identified by its class type.
  template <typename PhraseModelType>
  smtk::shared_ptr<PhraseModelType> create();

private:
  // ------ PhraseModel ------
  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  registerPhraseModels()
  {
    bool registered = this->registerPhraseModel<typename std::tuple_element<I, Tuple>::type>();
    return registered && Manager::registerPhraseModels<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  registerPhraseModels()
  {
    return true;
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  registerPhraseModels(const std::array<std::string, std::tuple_size<Tuple>::value>& typeNames)
  {
    bool registered =
      this->registerPhraseModel<typename std::tuple_element<I, Tuple>::type>(typeNames.at(I));
    return registered && Manager::registerPhraseModels<I + 1, Tuple>(typeNames);
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  registerPhraseModels(const std::array<std::string, std::tuple_size<Tuple>::value>&)
  {
    return true;
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type
  unregisterPhraseModels()
  {
    bool unregistered = this->unregisterPhraseModel<typename std::tuple_element<I, Tuple>::type>();
    return unregistered && Manager::unregisterPhraseModels<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type
  unregisterPhraseModels()
  {
    return true;
  }

  /// A container for all registered phrasemodel constructors.
  std::map<std::string, PhraseModelConstructor> m_phraseModels;

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
  IconFactory& iconFactory() { return m_iconFactory; }
  const IconFactory& iconFactory() const { return m_iconFactory; }

private:
  IconFactory m_iconFactory;
};

// ------ ViewWidget ------
template <typename ViewWidgetType>
bool Manager::unregisterViewWidget()
{
  return this->unregisterViewWidget(smtk::view::typeIndex<ViewWidgetType>());
}

template <typename ViewWidgetType>
smtk::view::BaseView* Manager::createViewWidget(const smtk::view::Information& info)
{
  return this->createViewWidget(smtk::view::typeIndex<ViewWidgetType>(), info);
}

template <typename ViewWidgetType>
bool Manager::registerViewWidget()
{
  return Manager::registerViewWidget<ViewWidgetType>(smtk::view::typeIndex<ViewWidgetType>());
}

template <typename ViewWidgetType>
bool Manager::registerViewWidget(std::size_t typeIndex)
{
  // see if already exists:
  if (m_viewWidgets.find(typeIndex) == m_viewWidgets.end())
  {
    m_viewWidgets[typeIndex] = [](
      const smtk::view::Information& info) { return ViewWidgetType::createViewWidget(info); };
    std::string alias = smtk::common::typeName<ViewWidgetType>();
    auto nameIter = m_altViewWidgetNames.find(alias);
    if (nameIter == m_altViewWidgetNames.end())
    {
      m_altViewWidgetNames[alias] = typeIndex;
    }
    return true;
  }
  return false;
}

// ------ PhraseModel ------
template <typename PhraseModelType>
bool Manager::unregisterPhraseModel()
{
  return this->unregisterPhraseModel(smtk::common::typeName<PhraseModelType>());
}

template <typename PhraseModelType>
smtk::shared_ptr<PhraseModelType> Manager::create()
{
  return smtk::static_pointer_cast<PhraseModelType>(
    this->create(smtk::common::typeName<PhraseModelType>()));
}

template <typename PhraseModelType>
bool Manager::registerPhraseModel()
{
  return Manager::registerPhraseModel<PhraseModelType>(smtk::common::typeName<PhraseModelType>());
}

template <typename PhraseModelType>
bool Manager::registerPhraseModel(const std::string& typeName)
{
  // see if already exists:
  if (m_phraseModels.find(typeName) == m_phraseModels.end())
  {
    // Must wrap: m_phraseModels[typeName] = PhraseModelType::create;
    m_phraseModels[typeName] = []() { return PhraseModelType::create(); };
    return true;
  }
  return false;
}

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
