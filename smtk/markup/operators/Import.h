//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Import_h
#define smtk_markup_Import_h

#include "smtk/markup/Exports.h"
#include "smtk/operation/XMLOperation.h"

#include <memory>
#include <vector>

namespace smtk
{
namespace resource
{
class Resource;
}
namespace markup
{

class Resource;

/**\brief Import geometric and ontological data into a resource.
  *
  * The import filter creates UnstructuredData, Ontology, and OntologyIdentifier nodes.
  *
  * For each UnstructuredData node, two URL nodes are created and attached: one links
  * the data to the filename/URL from which the data was imported, while the other
  * links the data to an unspecified location where the imported data will be
  * written in a native format. The latter location is set by the markup::Write
  * operation when the data is saved.
  *
  * For Ontology nodes, only the import URL is created (because ontology data is saved
  * as part of the graph resource rather than in an external file).
  */
class SMTKMARKUP_EXPORT Import : public smtk::operation::XMLOperation
{

public:
  smtkTypeMacro(smtk::markup::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  Specification createSpecification() override;
  const char* xmlDescription() const override;

  virtual bool importVTKImage(
    const std::shared_ptr<Resource>& resource,
    const std::string& filename);
  virtual bool importVTKMesh(
    const std::shared_ptr<Resource>& resource,
    const std::string& filename);
  virtual bool importOWL(const std::shared_ptr<Resource>& resource, const std::string& filename);

  static std::map<std::string, std::set<std::string>> supportedVTKFileFormats();

  Result m_result;
};

SMTKMARKUP_EXPORT std::shared_ptr<smtk::resource::Resource> importResource(
  const std::string& filename);
} // namespace markup
} // namespace smtk

#endif
