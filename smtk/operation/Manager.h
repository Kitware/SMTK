//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Manager.h - Class for managing SMTK operators
// .SECTION Description
// .SECTION See Also

#ifndef smtk_operation_Manager_h
#define smtk_operation_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

#include "smtk/operation/Operator.h"

#include <string>
#include <typeinfo>
#include <unordered_map>

namespace smtk
{
namespace operation
{
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
  typedef std::unordered_map<Operator::Index, Operator::Info> Dictionary;

public:
  smtkTypeMacroBase(Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager();

  // Construct a resource identified by its type index.
  smtk::operation::OperatorPtr create(Operator::Index);

  // Construct a resource identified by its class type.
  template <class OperatorType>
  smtk::shared_ptr<OperatorType> create();

  // TODO: All of the registration methods have way too much boilerplate info.
  // This much information about each operator cannot be required (not to
  // mention the implicit requirement that names in the xml description match
  // compiled string descriptions).

  // Register a resource identified by its type index.
  bool registerOperator(Operator::Index, const std::string&, const std::string&, const char*,
    const Operator::Constructor&);

  // Register a resource identified by its class type.
  template <class OperatorType>
  bool registerOperator(
    const std::string&, const std::string&, const char*, const typename OperatorType::Constructor&);

  // Register a resource identified by its type index.
  static bool registerStaticOperator(Operator::Index, const std::string&, const std::string&,
    const char*, const Operator::Constructor&);

  // Register a resource identified by its class type.
  template <class OperatorType>
  static bool registerStaticOperator(
    const std::string&, const std::string&, const char*, const typename OperatorType::Constructor&);

  // Unregister a resource identified by its type index.
  static bool unregisterStaticOperator(Operator::Index);

  // Unregister a resource identified by its class type.
  template <class OperatorType>
  static bool unregisterStaticOperator();

private:
  Manager();

  // Import XML describing an operator into the operator collection.
  bool importOperatorXML(
    const std::string& opClassName, const std::string& opNickName, const std::string& opDescrXML);

  // A map between the Operator's type_index and its constructor.
  static Dictionary s_dictionary;
  smtk::attribute::CollectionPtr m_operatorCollection;
};

template <class OperatorType>
smtk::shared_ptr<OperatorType> Manager::create()
{
  return smtk::dynamic_pointer_cast<OperatorType>(
    this->create(Operator::Index(typeid(OperatorType))));
}

template <class OperatorType>
bool Manager::registerOperator(const std::string& opClassName, const std::string& opNickName,
  const char* opDescrXML, const typename OperatorType::Constructor& constructor)
{
  return Manager::registerOperator(
    Operator::Index(typeid(OperatorType)), opClassName, opNickName, opDescrXML, constructor);
}

template <class OperatorType>
bool Manager::registerStaticOperator(const std::string& opClassName, const std::string& opNickName,
  const char* opDescrXML, const typename OperatorType::Constructor& constructor)
{
  return Manager::registerStaticOperator(
    Operator::Index(typeid(OperatorType)), opClassName, opNickName, opDescrXML, constructor);
}

template <class OperatorType>
bool Manager::unregisterStaticOperator()
{
  return Manager::unregisterStaticOperator(Operator::Index(typeid(OperatorType)));
}
}
}

#endif // smtk_operation_Manager_h
