//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/Export.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/mesh/MeshIO.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/Export_xml.h"

#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

#include <sstream>
#include <string>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
} // namespace

namespace smtk
{
namespace mesh
{

bool Export::ableToOperate()
{
  return this->Superclass::ableToOperate();
}

Export::Result Export::operateInternal()
{
  std::string outputfile = this->parameters()->findFile("filename")->value();

  smtk::attribute::ReferenceItem::Ptr meshItem = this->parameters()->associations();

  // for multiple meshes, we suffix the file name root with ascending integers
  std::string root = outputfile.substr(0, outputfile.find_last_of('.'));
  std::string ext = outputfile.substr(outputfile.find_last_of('.'));
  int index = 0;

  std::vector<std::string> generatedFiles;

  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::ResourcePtr resource = meshItem->valueAs<smtk::mesh::Resource>(i);
    bool fileExportSuccess = false;

    if (resource)
    {
      if (meshItem->numberOfValues() > 1)
      {
        std::stringstream s;
        s << root << "_" << index << ext;
        outputfile = s.str();
      }

      smtk::io::ExportMesh exportMesh;
      fileExportSuccess = exportMesh(outputfile, resource);

      if (fileExportSuccess)
      {
        ++index;
        generatedFiles.push_back(outputfile);
      }
    }

    if (!fileExportSuccess)
    {
      for (auto&& file : generatedFiles)
      {
        cleanup(file);
      }
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
  }

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

Export::Specification Export::createSpecification()
{
  Specification spec = this->smtk::operation::XMLOperation::createSpecification();
  auto exportDef = spec->findDefinition("export");

  std::vector<smtk::attribute::FileItemDefinition::Ptr> fileItemDefinitions;
  auto fileItemDefinitionFilter = [](smtk::attribute::FileItemDefinition::Ptr ptr) {
    return ptr->name() == "filename";
  };
  exportDef->filterItemDefinitions(fileItemDefinitions, fileItemDefinitionFilter);

  assert(fileItemDefinitions.size() == 1);

  std::stringstream fileFilters;
  bool firstFormat = true;
  for (auto& ioType : smtk::io::ExportMesh::SupportedIOTypes())
  {
    for (const auto& format : ioType->FileFormats())
    {
      if (format.CanExport())
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

const char* Export::xmlDescription() const
{
  return Export_xml;
}
} // namespace mesh
} // namespace smtk
