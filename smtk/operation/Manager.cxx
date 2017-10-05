//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PublicPointerDefs.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

namespace smtk
{
namespace operation
{
Manager::Dictionary Manager::s_dictionary;

Manager::Manager()
{
  this->m_operatorCollection = smtk::attribute::Collection::create();

  // Create the "base" definitions that all operators and results will inherit.
  smtk::attribute::Definition::Ptr opdefn =
    this->m_operatorCollection->createDefinition("operator");

  smtk::attribute::IntItemDefinition::Ptr debugLevelDefn =
    smtk::attribute::IntItemDefinition::New("debug level");
  debugLevelDefn->setIsOptional(true);
  debugLevelDefn->setDefaultValue(0);
  debugLevelDefn->setAdvanceLevel(10);

  opdefn->addItemDefinition(debugLevelDefn);

  smtk::attribute::Definition::Ptr resultdefn =
    this->m_operatorCollection->createDefinition("result");

  smtk::attribute::IntItemDefinition::Ptr outcomeDefn =
    smtk::attribute::IntItemDefinition::New("outcome");
  outcomeDefn->setNumberOfRequiredValues(1);
  outcomeDefn->setIsOptional(false);
  resultdefn->addItemDefinition(outcomeDefn);

  smtk::attribute::StringItemDefinition::Ptr logDefn =
    smtk::attribute::StringItemDefinition::New("log");
  logDefn->setNumberOfRequiredValues(0);
  logDefn->setIsExtensible(1);
  logDefn->setIsOptional(true);
  resultdefn->addItemDefinition(logDefn);

  for (auto it = this->s_dictionary.begin(); it != this->s_dictionary.end(); ++it)
  {
    Operator::Info& info = it->second;
    if (info.xml.empty())
      continue;

    this->importOperatorXML(info.classname, info.nickname, info.xml);
  }
}

Manager::~Manager()
{
}

namespace
{
void replace(std::string& str, const std::string& from, const std::string& to)
{
  if (from.empty())
  {
    return;
  }
  std::size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos)
  {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}
}

bool Manager::importOperatorXML(
  const std::string& opClassName, const std::string& opNickName, const std::string& opDescrXML)
{
  // For now, we replace all instances of <opNickName> in <opDescrXML> with
  // <opClassName>. This is a nefarious kludge that must go away with a
  // refactoring of the registration system, but doing so may require that
  // every operator's XML template be updated.

  std::string xml(opDescrXML);
  replace(xml, opNickName, opClassName);

  smtk::io::Logger tmpLog;
  smtk::io::AttributeReader rdr;
  bool ok = true;

  ok &= !rdr.readContents(this->m_operatorCollection, xml.c_str(), xml.size(), tmpLog);

  if (!ok)
  {
    std::cerr << "Error. Log follows:\n---\n" << tmpLog.convertToString() << "\n---\n";
  }

  return ok;
}

smtk::operation::OperatorPtr Manager::create(Operator::Index index)
{
  auto info = this->s_dictionary.find(index);
  return info != this->s_dictionary.end() ? info->second.constructor()
                                          : smtk::operation::OperatorPtr();
}

bool Manager::registerOperator(Operator::Index index, const std::string& opClassName,
  const std::string& opNickName, const char* opDescrXML, const Operator::Constructor& constructor)
{
  bool registered =
    Manager::registerStaticOperator(index, opClassName, opNickName, opDescrXML, constructor);

  if (registered && opDescrXML)
  {
    this->importOperatorXML(opClassName, opNickName, opDescrXML);
  }

  return registered;
}

bool Manager::registerStaticOperator(Operator::Index index, const std::string& opClassName,
  const std::string& opNickName, const char* opDescrXML, const Operator::Constructor& constructor)
{
  if (s_dictionary.find(index) == s_dictionary.end())
  {
    s_dictionary.insert(
      std::make_pair(index, Operator::Info(opClassName, opNickName,
                              (opDescrXML ? std::string(opDescrXML) : ""), constructor)));
    return true;
  }

  return false;
}

bool Manager::unregisterStaticOperator(Operator::Index index)
{
  return s_dictionary.erase(index) != 0;
}
}
}
