//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME XmlStringWriter.h - Abstract base class for internal attribute writers
// .SECTION Description - Header file only
// .SECTION See Also

#ifndef smtk_io_XmlStringWriter_h
#define smtk_io_XmlStringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/Resource.h"
#include "smtk/io/Logger.h"

#include <set>
#include <string>

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace io
{
class SMTKCORE_EXPORT XmlStringWriter
{
public:
  XmlStringWriter(smtk::attribute::ResourcePtr resource, smtk::io::Logger& logger)
    : m_resource(resource)
    , m_logger(logger)
  {
  }

  virtual ~XmlStringWriter() = default;

  // Subclass methods
  virtual std::string className() const = 0;
  virtual std::string rootNodeName() const = 0;
  virtual unsigned int fileVersion() const = 0;

  virtual std::string convertToString(bool no_declaration = false) = 0;
  // If using the resource's DirectoryInfo - return the ith string (NOTE - make use you have called
  // convertToString first!)
  virtual std::string getString(std::size_t ith, bool no_declaration = false) = 0;

  virtual void generateXml() = 0;

  //Control which sections of the attribute resource should be written out
  // By Default all sections are processed.  These are advance options!!

  // If val is false then Analyses will not be saved
  void includeAnalyses(bool val) { m_includeAnalyses = val; }

  // If val is false then Advance Levels will not be saved
  void includeAdvanceLevels(bool val) { m_includeAdvanceLevels = val; }

  // If val is false then Attribute Associations will not be saved
  void includeAttributeAssociations(bool val) { m_includeAttributeAssociations = val; }

  // If val is false then definitions will not be saved (default is true)
  void includeDefinitions(bool val) { m_includeDefinitions = val; }

  // If val is false then Evaluators will not be saved (default is true).
  void includeEvaluators(bool val) { m_includeEvaluators = val; }

  // If val is false then Attribute instances will not be saved (default is true)
  void includeInstances(bool val) { m_includeInstances = val; }

  // If val is false then the Resource's associations will not be saved
  void includeResourceAssociations(bool val) { m_includeResourceAssociations = val; }

  // If val is false then the Resource's ID will not be saved
  void includeResourceID(bool val) { m_includeResourceID = val; }

  // If val is false then UniqueRoles will not be saved
  void includeUniqueRoles(bool val) { m_includeUniqueRoles = val; }

  // If val is false then views will not be saved (default is true)
  void includeViews(bool val) { m_includeViews = val; }

  // If val is true then the resource's DirectoryInfo is used to construct several
  // XML representations
  void useDirectoryInfo(bool val) { m_useDirectoryInfo = val; }

  // Restricts the types of attribute instances written out to those derived from a
  // specified list.  If the list is empty then all attributes will be saved.
  void setIncludedDefinitions(const std::vector<smtk::attribute::DefinitionPtr>& includedDefs)
  {
    m_includedDefs = includedDefs;
  }

  // Restricts the types of attribute instances written out to those *not* derived
  // from a specified list.  If the list is empty then all attributes will be saved.
  void setExcludedDefinitions(const std::set<smtk::attribute::DefinitionPtr>& excludedDefs)
  {
    m_excludedDefs = excludedDefs;
  }

protected:
  smtk::attribute::ResourcePtr m_resource;
  bool m_includeAnalyses{ true };
  bool m_includeAdvanceLevels{ true };
  bool m_includeAttributeAssociations{ true };
  bool m_includeDefinitions{ true };
  bool m_includeEvaluators{ true };
  bool m_includeInstances{ true };
  bool m_includeResourceAssociations{ true };
  bool m_includeResourceID{ true };
  bool m_includeUniqueRoles{ true };
  bool m_includeViews{ true };
  bool m_useDirectoryInfo{ false };
  std::vector<smtk::attribute::DefinitionPtr> m_includedDefs;
  std::set<smtk::attribute::DefinitionPtr> m_excludedDefs;

  smtk::io::Logger& m_logger;
};
} // namespace io
} // namespace smtk

#endif // smtk_io_XmlStringWriter_h
