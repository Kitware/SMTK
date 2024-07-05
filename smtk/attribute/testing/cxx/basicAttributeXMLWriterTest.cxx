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
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/model/EntityTypeBits.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include <iostream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

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

int main(int argc, char* argv[])
{
  int status = 0;
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " FullAttributeFilename InstanceOnlyFileName\n";
    return -1;
  }
  auto resptr = smtk::attribute::Resource::create();
  smtk::attribute::Resource& resource(*resptr);
  std::cout << "Resource Created\n";
  // Lets add some analyses
  auto& analyses = resptr->analyses();
  std::set<std::string> cats;
  cats.insert("Flow");
  cats.insert("General");
  cats.insert("Time");
  auto* analysis = analyses.create("CFD Flow");
  analysis->setLocalCategories(cats);
  cats.clear();

  cats.insert("Flow");
  cats.insert("Heat");
  cats.insert("General");
  cats.insert("Time");
  analysis = analyses.create("CFD Flow with Heat Transfer");
  analysis->setLocalCategories(cats);
  cats.clear();

  cats.insert("Constituent");
  cats.insert("General");
  cats.insert("Time");
  analysis = analyses.create("Constituent Transport");
  analysis->setLocalCategories(cats);
  cats.clear();

  double lcolor1[] = { 1.0, 1.0, 0.0, 0.1 };
  double lcolor2[] = { 1.0, 0.0, 1.0, 0.2 };
  double lcolor3[] = { 0.0, 1.0, 1.0, 0.3 };
  resource.addAdvanceLevel(0, "Level 0", lcolor1);
  resource.addAdvanceLevel(1, "Level 1", lcolor2);
  resource.addAdvanceLevel(2, "Level 2", lcolor3);

  // Lets create an attribute to represent an expression
  smtk::attribute::DefinitionPtr expDef = resource.createDefinition("ExpDef");
  expDef->setBriefDescription("Sample Expression");
  expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
  expDef->addTag(smtk::attribute::Tag("My Tag"));
  expDef->addTag(smtk::attribute::Tag("My Tag with Values", { "value1", "value2", "value3" }));
  smtk::attribute::StringItemDefinitionPtr eitemdef =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("Expression String");
  smtk::attribute::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
  eitemdef->setDefaultValue("sample");

  smtk::attribute::DefinitionPtr base = resource.createDefinition("BaseDef");
  // Lets add some item definitions
  smtk::attribute::IntItemDefinitionPtr iitemdef =
    base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("TEMPORAL");
  iitemdef->setIsExtensible(true);
  iitemdef->setCommonValueLabel("Time");
  iitemdef->addDiscreteValue(0, "Seconds");
  iitemdef->addDiscreteValue(1, "Minutes");
  iitemdef->addDiscreteValue(2, "Hours");
  iitemdef->addDiscreteValue(3, "Days");
  iitemdef->setDefaultDiscreteIndex(0);
  iitemdef->localCategories().insertInclusion("Time");
  iitemdef = base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("IntItem2");
  iitemdef->setDefaultValue(10);
  iitemdef->localCategories().insertInclusion("Heat");

  smtk::attribute::DefinitionPtr def1 = resource.createDefinition("Derived1", "BaseDef");
  def1->setLocalAssociationMask(smtk::model::MODEL_ENTITY); // belongs on model
  // Lets add some item definitions
  smtk::attribute::DoubleItemDefinitionPtr ditemdef =
    def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem1");
  // Allow this one to hold an expression
  ditemdef->localCategories().insertInclusion("Veg");
  ditemdef->setExpressionDefinition(expDef);
  // Check to make sure we can use expressions
  if (!ditemdef->allowsExpressions())
  {
    std::cout << "ERROR - Item Def does not allow expressions\n";
    status = -1;
  }
  ditemdef = def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem2");
  ditemdef->setDefaultValue(-35.2);
  ditemdef->setMinRange(-100, true);
  ditemdef->setMaxRange(125.0, false);
  ditemdef->localCategories().insertInclusion("Constituent");
  smtk::attribute::VoidItemDefinitionPtr vdef =
    def1->addItemDefinition<smtk::attribute::VoidItemDefinitionPtr>("VoidItem");
  vdef->setIsOptional(true);
  vdef->setLabel("Option 1");

  smtk::attribute::DefinitionPtr def2 = resource.createDefinition("Derived2", "Derived1");
  def2->setLocalAssociationMask(smtk::model::VOLUME);
  // Lets add some item definitions
  smtk::attribute::StringItemDefinitionPtr sitemdef =
    def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem1");
  sitemdef->setIsMultiline(true);
  sitemdef->localCategories().insertInclusion("Flow");
  sitemdef = def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem2");
  sitemdef->setDefaultValue("Default");
  sitemdef->localCategories().insertInclusion("General");
  smtk::attribute::DirectoryItemDefinitionPtr dirdef =
    def2->addItemDefinition<smtk::attribute::DirectoryItemDefinitionPtr>("DirectoryItem");
  dirdef->setShouldExist(true);
  dirdef->setShouldBeRelative(true);
  smtk::attribute::FileItemDefinitionPtr fdef =
    def2->addItemDefinition<smtk::attribute::FileItemDefinitionPtr>("FileItem");
  fdef->setShouldBeRelative(true);
  smtk::attribute::GroupItemDefinitionPtr gdef1,
    gdef = def2->addItemDefinition<smtk::attribute::GroupItemDefinitionPtr>("GroupItem");
  gdef->addItemDefinition<smtk::attribute::FileItemDefinitionPtr>("File1");
  gdef1 = gdef->addItemDefinition<smtk::attribute::GroupItemDefinitionPtr>("SubGroup");
  sitemdef = gdef1->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("GroupString");
  sitemdef->setDefaultValue("Something Cool");
  sitemdef->localCategories().insertInclusion("General");
  sitemdef->localCategories().insertInclusion("Flow");

  // Add in a Attribute definition with a component item referencing to another attribute
  smtk::attribute::DefinitionPtr attcompdef = resource.createDefinition("AttributeComponentDef");
  smtk::attribute::ComponentItemDefinitionPtr acitemdef =
    attcompdef->addItemDefinition<smtk::attribute::ComponentItemDefinitionPtr>("BaseDefItem");
  acitemdef->setCommonValueLabel("A reference to another attribute");
  std::string attQuery = smtk::attribute::Resource::createAttributeQuery(base);
  acitemdef->setAcceptsEntries(smtk::common::typeName<smtk::attribute::Resource>(), attQuery, true);

  // Process Definition Information
  resource.finalizeDefinitions();
  // Lets test creating an attribute by passing in the expression definition explicitly
  smtk::attribute::AttributePtr expAtt = resource.createAttribute("Exp1", expDef);
  smtk::attribute::AttributePtr att = resource.createAttribute("testAtt", "Derived2");
  if (!att)
  {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
  }

  smtk::attribute::ValueItemPtr vitem;
  smtk::attribute::ItemPtr item;

  // Find the expression enabled item
  item = att->item(2);
  vitem = smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);
  smtk::io::AttributeWriter writer;
  writer.setFileVersion(3);
  smtk::io::Logger logger;
  if (writer.write(resptr, argv[1], logger))
  {
    std::cerr << "Errors encountered creating Attribute File:\n";
    std::cerr << logger.convertToString();
    status = -1;
  }

  // Sanity check readback
  auto inputResptr = smtk::attribute::Resource::create();
  smtk::io::AttributeReader reader;
  if (reader.read(inputResptr, argv[1], logger))
  {
    std::cerr << "Errors encountered reading back Attribute File:\n";
    std::cerr << logger.convertToString();
    status = -1;
  }

  // Now repeat but only save the instance section
  writer.includeDefinitions(false);
  writer.includeViews(false);
  if (writer.write(resptr, argv[2], logger))
  {
    std::cerr << "Errors encountered creating Instance Only Attribute File:\n";
    std::cerr << logger.convertToString();
    status = -1;
  }

  std::cout << "Resource destroyed\n";

  return status;
}
