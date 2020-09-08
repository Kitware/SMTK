//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/PythonRule.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/json/jsonResource.h"
#include "smtk/attribute/operators/Read.h"
#include "smtk/attribute/operators/Write.h"
#include "smtk/common/UUID.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/utility/Create.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <algorithm>
#include <iostream>

namespace
{
std::string write_root = SMTK_SCRATCH_DIR;

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

int testAssociationRule(const smtk::attribute::AttributePtr& attribute)
{
  // Test if the attribute instance with a custom association rule can be
  // associated with itself (it should not).
  if (attribute->canBeAssociated(attribute))
  {
    std::cerr << "Custom Python rule should reject attribute components\n";
    std::cerr << smtk::io::Logger::instance().convertToString();
    return -2;
  }

  // Test if the attribute instance with a custom association rule can be
  // associated with a mesh component (it should).
  {
    // Create a new mesh mesh resource
    smtk::mesh::ResourcePtr meshResource = smtk::mesh::Resource::create();

    // Construct a uniform grid
    std::array<std::size_t, 3> discretization = { { 2, 2, 2 } };
    std::function<std::array<double, 3>(std::array<double, 3>)> transform = [](
      std::array<double, 3> x) { return x; };
    auto meshes = smtk::mesh::utility::createUniformGrid(meshResource, discretization, transform);
    auto meshComponent = smtk::mesh::Component::create(meshes[0]);

    // Test whether the attribute resource can be associated to the newly
    // created mesh component.
    if (!attribute->canBeAssociated(meshComponent))
    {
      std::cerr << "Custom Python rule should accept mesh components\n";
      std::cerr << smtk::io::Logger::instance().convertToString();
      return -2;
    }
  }

  return 0;
}

const char* testInput = R"xml(
<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="4">
  <Definitions>
    <AttDef Type="att1" BaseType="">
      <AssociationRule Name="myPythonRule"/>
      <ItemDefinitions>
        <String Name="normalString" Extensible="0"
               NumberOfRequiredValues="1">
         <DefaultValue>normal</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <AssociationRules>
    <PythonRule Name="myPythonRule"><![CDATA[
def canBeAssociated(attribute, object):
    import smtk.mesh
    meshComponent = smtk.mesh.Component.CastTo(object)
    return meshComponent != None
      ]]></PythonRule>
  </AssociationRules>
  <Attributes>
    <Att Name="att" Type="att1"/>
  </Attributes>
</SMTK_AttributeResource>)xml";
}

int unitAssociationRulesTest(int argc, char** argv)
{
  (void)argc;
  (void)argv;

  // Construct an attribute resource from the test input, test its association
  // properties, and write it to disk.
  std::string sbiFileName;
  std::string smtkFileName;
  {
    // Construct a new attribute resource
    smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();

    // Associate the smtk::attribute::PythonRule association type to it, and give
    // it the alias "PythonRule".
    resource->associationRules()
      .associationRuleFactory()
      .registerType<smtk::attribute::PythonRule>();
    resource->associationRules().associationRuleFactory().addAlias<smtk::attribute::PythonRule>(
      "PythonRule");

    // Read the input xml to construct an attribute instance.
    smtk::io::Logger logger;
    smtk::io::AttributeReader reader;
    if (reader.readContents(resource, testInput, logger))
    {
      std::cerr << "Encountered Errors while reading input data\n";
      std::cerr << logger.convertToString();
      return -2;
    }

    // Access the newly created attribute instance.
    std::vector<smtk::attribute::AttributePtr> atts;
    resource->attributes(atts);
    if (atts.size() != 1)
    {
      std::cerr << "Unexpected number of attributes: " << atts.size() << "\n";
      std::cerr << logger.convertToString();
      return -2;
    }
    smtk::attribute::AttributePtr att = atts[0];

    // Test the custom association rule.
    int testValue = testAssociationRule(att);
    if (testValue)
    {
      return testValue;
    }

    // Write the resource out to a .sbi file
    {
      sbiFileName = write_root + "/associationRulesTest.sbi";
      // sbiFileName = write_root + "/" + smtk::common::UUID::random().toString() + ".sbi";

      smtk::io::AttributeWriter writer;
      if (writer.write(resource, sbiFileName, logger))
      {
        std::cerr << "Encountered Errors while writing output data\n";
        std::cerr << logger.convertToString();
        return -2;
      }
    }

    // Write the resource out to a .smtk file
    {
      smtkFileName = write_root + "/associationRulesTest.json";
      // smtkFileName = write_root + "/" + smtk::common::UUID::random().toString() + ".json";
      resource->setLocation(smtkFileName);

      smtk::attribute::Write::Ptr writeOp = smtk::attribute::Write::create();
      writeOp->parameters()->associate(resource);
      auto result = writeOp->operate();

      if (result->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        std::cerr << "Write operation failed\n";
        std::cerr << writeOp->log().convertToString();
        return -2;
      }
    }
  }

  {
    // Read the .sbi file into a new resource
    smtk::attribute::Resource::Ptr resource = smtk::attribute::Resource::create();
    resource->associationRules()
      .associationRuleFactory()
      .registerType<smtk::attribute::PythonRule>();
    resource->associationRules().associationRuleFactory().addAlias<smtk::attribute::PythonRule>(
      "PythonRule");

    smtk::io::AttributeReader reader;

    std::ifstream file(smtkFileName);
    nlohmann::json j = nlohmann::json::parse(file);
    smtk::attribute::from_json(j, resource);
    resource->setLocation(smtkFileName);

    std::vector<smtk::attribute::AttributePtr> atts;
    resource->attributes(atts);
    if (atts.size() != 1)
    {
      std::cerr << "Unexpected number of attributes: " << atts.size() << "\n";
      return -2;
    }
    smtk::attribute::AttributePtr att = atts[0];

    // Test the custom association rule.
    int testValue = testAssociationRule(att);
    if (testValue)
    {
      return testValue;
    }
  }

  {
    // Read the .smtk file into a new resource
    smtk::attribute::Resource::Ptr resource = smtk::attribute::Resource::create();
    resource->associationRules()
      .associationRuleFactory()
      .registerType<smtk::attribute::PythonRule>();
    resource->associationRules().associationRuleFactory().addAlias<smtk::attribute::PythonRule>(
      "PythonRule");

    // Read the generated sbi file to construct an attribute instance.
    smtk::io::Logger logger;
    smtk::io::AttributeReader reader;
    if (reader.read(resource, sbiFileName, logger))
    {
      std::cerr << "Encountered Errors while reading input data\n";
      std::cerr << logger.convertToString();
      return -2;
    }

    // Access the newly created attribute instance.
    std::vector<smtk::attribute::AttributePtr> atts;
    resource->attributes(atts);
    if (atts.size() != 1)
    {
      std::cerr << "Unexpected number of attributes: " << atts.size() << "\n";
      std::cerr << logger.convertToString();
      return -2;
    }
    smtk::attribute::AttributePtr att = atts[0];

    // Test the custom association rule.
    int testValue = testAssociationRule(att);
    if (testValue)
    {
      return testValue;
    }
  }

  // cleanup(smtkFileName);
  // cleanup(sbiFileName);

  return 0;
}
