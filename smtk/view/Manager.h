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

#include "smtk/view/View.h"

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
  smtkTypedefs(smtk::view::Manager);
  smtkCreateMacro(smtk::view::Manager);

  using PhraseModelConstructor = std::function<PhraseModelPtr(void)>;
  using SubphraseGeneratorConstructor = std::function<SubphraseGeneratorPtr(void)>;

  virtual ~Manager();

  // ------ PhraseModel ------
  /// Register a resource identified by its class type.
  template <typename ResourceType>
  bool registerPhraseModel();

  /// Register a resource identified by its class type and type name.
  template <typename ResourceType>
  bool registerPhraseModel(const std::string&);

  /// Register a tuple of views identified by their class types.
  template <typename Tuple>
  bool registerPhraseModels()
  {
    return Manager::registerPhraseModels<0, Tuple>();
  }

  /// Register a tuple of views identified by their class types and type
  /// names.
  template <typename Tuple>
  bool registerPhraseModels(const std::array<std::string, std::tuple_size<Tuple>::value>& typeNames)
  {
    return Manager::registerPhraseModels<0, Tuple>(typeNames);
  }

  /// Unregister a resource identified by its class type.
  template <typename ResourceType>
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

  /// Return a set of type names for all PhraseModels.
  // std::set<std::string> availablePhraseModels() const;

  // ------ SubphraseGenerator ------
  /// Register a resource identified by its class type.
  template <typename ResourceType>
  bool registerSubphraseGenerator();

  /// Register a resource identified by its class type and type name.
  template <typename ResourceType>
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

  /// Unregister a resource identified by its class type.
  template <typename ResourceType>
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

private:
  Manager();

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
};

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
