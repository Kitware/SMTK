//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/Read.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ReadMesh.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Component.h"

#include "smtk/mesh/Read_xml.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>

namespace
{
class AddMeshToResult : public smtk::mesh::MeshForEach
{
public:
  AddMeshToResult(smtk::mesh::Read::Result& result)
    : smtk::mesh::MeshForEach()
    , m_result(result)
  {
  }

  void forMesh(smtk::mesh::MeshSet& mesh) override
  {
    m_result->findComponent("created")->appendValue(smtk::mesh::Component::create(mesh));
  }

private:
  smtk::mesh::Read::Result& m_result;
};
}

namespace smtk
{
namespace mesh
{

Read::Result Read::operateInternal()
{
  // Get the read file name
  smtk::attribute::FileItem::Ptr filePathItem = this->parameters()->findFile("filename");
  std::string filePath = filePathItem->value();

  // Get the subset value
  smtk::attribute::IntItem::Ptr subsetItem = this->parameters()->findInt("subset");
  smtk::io::mesh::Subset subset = static_cast<smtk::io::mesh::Subset>(subsetItem->value());

  auto collection = smtk::mesh::Collection::create();
  bool success = smtk::io::readMesh(filePath, collection, subset);

  if (success == false)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  AddMeshToResult addMeshToResult(result);

  smtk::mesh::for_each(collection->meshes(), addMeshToResult);

  result->findResource("resource")->appendValue(collection);

  return result;
}

Read::Specification Read::createSpecification()
{
  Specification spec = this->smtk::operation::XMLOperation::createSpecification();
  auto importDef = spec->findDefinition("read");

  std::vector<smtk::attribute::FileItemDefinition::Ptr> fileItemDefinitions;
  auto fileItemDefinitionFilter = [](
    smtk::attribute::FileItemDefinition::Ptr ptr) { return ptr->name() == "filename"; };
  importDef->filterItemDefinitions(fileItemDefinitions, fileItemDefinitionFilter);

  assert(fileItemDefinitions.size() == 1);

  std::stringstream fileFilters;
  for (auto& ioType : smtk::io::ReadMesh::SupportedIOTypes())
  {
    bool firstFormat = true;
    for (auto& format : ioType->FileFormats())
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
        for (auto& ext : format.Extensions)
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

const char* Read::xmlDescription() const
{
  return Read_xml;
}
}
}
