//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/Import.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/operators/Import_xml.h"

namespace
{
class AddMeshToResult : public smtk::mesh::MeshForEach
{
public:
  AddMeshToResult(smtk::mesh::Import::Result& result)
    : m_result(result)
  {
  }

  void forMesh(smtk::mesh::MeshSet& mesh) override
  {
    m_result->findComponent("created")->appendValue(smtk::mesh::Component::create(mesh));
  }

private:
  smtk::mesh::Import::Result& m_result;
};
} // namespace

namespace smtk
{
namespace mesh
{

Import::Result Import::operateInternal()
{
  // Get the import file name
  smtk::attribute::FileItem::Ptr filePathItem = this->parameters()->findFile("filename");
  std::string filePath = filePathItem->value();

  // Get the subset value
  smtk::attribute::StringItem::Ptr labelItem = this->parameters()->findString("label");
  std::string label = labelItem->value();

  auto resource = smtk::mesh::Resource::create();
  bool success = smtk::io::importMesh(filePath, resource, label);

  if (!success)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  AddMeshToResult addMeshToResult(result);

  smtk::mesh::for_each(resource->meshes(), addMeshToResult);

  auto assignNames = this->parameters()->findVoid("assign default names");
  if (assignNames && assignNames->isEnabled())
  {
    resource->assignDefaultNames();
  }

  result->findResource("resource")->appendValue(resource);

  return result;
}

Import::Specification Import::createSpecification()
{
  Specification spec = this->smtk::operation::XMLOperation::createSpecification();
  auto importDef = spec->findDefinition("import");

  std::vector<smtk::attribute::FileItemDefinition::Ptr> fileItemDefinitions;
  auto fileItemDefinitionFilter = [](smtk::attribute::FileItemDefinition::Ptr ptr) {
    return ptr->name() == "filename";
  };
  importDef->filterItemDefinitions(fileItemDefinitions, fileItemDefinitionFilter);

  assert(fileItemDefinitions.size() == 1);

  std::stringstream fileFilters;
  bool firstFormat = true;
  for (auto& ioType : smtk::io::ImportMesh::SupportedIOTypes())
  {
    for (const auto& format : ioType->FileFormats())
    {
      if (format.CanImport())
      {
        if (firstFormat)
        {
          firstFormat = false;
        }
        else
        {
          fileFilters << ";;";
        }

        fileFilters << format.Name << "(";
        bool first = true;
        for (const auto& ext : format.Extensions)
        {
          if (first)
          {
            first = false;
          }
          else
          {
            fileFilters << " ";
          }
          fileFilters << "*" << ext;
        }
        fileFilters << ")";
      }
    }
  }
  fileItemDefinitions[0]->setFileFilters(fileFilters.str());
  return spec;
}

const char* Import::xmlDescription() const
{
  return Import_xml;
}
} // namespace mesh
} // namespace smtk
