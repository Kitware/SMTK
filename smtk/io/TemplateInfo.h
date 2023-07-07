//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_io_TemplateInfo_h
#define smtk_io_TemplateInfo_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.

#include <map>
#include <set>
#include <string>

namespace pugi
{
class xml_document;
class xml_node;
} // namespace pugi

namespace smtk
{
namespace io
{
class Logger;

/**\brief A collection of methods to aid I/O.
  *
  */
class SMTKCORE_EXPORT TemplateInfo
{
public:
  TemplateInfo() = default;
  ~TemplateInfo() = default;
  bool define(
    const std::string& globalNameSpace,
    pugi::xml_node& node,
    bool& isToBeExported,
    smtk::io::Logger& log);
  const std::string& name() const { return m_name; }
  const std::string& nameSpace() const { return m_nameSpace; }
  pugi::xml_node
  instantiate(pugi::xml_node& instancedNode, pugi::xml_node& storageNode, smtk::io::Logger& log);

protected:
  std::string m_name;
  std::string m_nameSpace;
  std::string m_contents;
  std::set<std::string> m_parameters;
  std::map<std::string, std::string> m_paramValues;
};

} // namespace io
} // namespace smtk

#endif // smtk_io_TemplateInfo_h
