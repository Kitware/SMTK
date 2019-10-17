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

#include "smtk/view/PhraseModel.h"
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
  smtkCreateMacro(Manager);

  using PhraseModelConstructor = std::function<PhraseModelPtr(void)>;

  virtual ~Manager();

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

  /// Construct an PhraseModel identified by its type name.
  std::shared_ptr<smtk::view::PhraseModel> create(const std::string&);

  /// Construct an PhraseModel identified by its class type.
  template <typename PhraseModelType>
  smtk::shared_ptr<PhraseModelType> create();

  /// Return a set of type names for all PhraseModels.
  std::set<std::string> availablePhraseModels() const;

private:
  Manager();

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
};

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
}
}

#endif // smtk_view_Manager_h
