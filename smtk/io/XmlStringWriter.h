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

#ifndef __smtk_io_XmlStringWriter_h
#define __smtk_io_XmlStringWriter_h
#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/Resource.h"
#include "smtk/io/Logger.h"

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
    , m_includeAnalyses(true)
    , m_includeAdvanceLevels(true)
    , m_includeAttributeAssociations(true)
    , m_includeDefinitions(true)
    , m_includeEvaluators(true)
    , m_includeInstances(true)
    , m_includeResourceAssociations(true)
    , m_includeUniqueRoles(true)
    , m_includeViews(true)
    , m_useDirectoryInfo(false)
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

protected:
  smtk::attribute::ResourcePtr m_resource;
  bool m_includeAnalyses;
  bool m_includeAdvanceLevels;
  bool m_includeAttributeAssociations;
  bool m_includeDefinitions;
  bool m_includeEvaluators;
  bool m_includeInstances;
  bool m_includeResourceAssociations;
  bool m_includeResourceID;
  bool m_includeUniqueRoles;
  bool m_includeViews;
  bool m_useDirectoryInfo;
  std::vector<smtk::attribute::DefinitionPtr> m_includedDefs;

  smtk::io::Logger& m_logger;
};
} // namespace io
} // namespace smtk

#endif // __smtk_io_XmlStringWriter_h
