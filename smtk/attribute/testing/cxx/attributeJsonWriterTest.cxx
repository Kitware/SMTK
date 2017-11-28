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
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/model/EntityTypeBits.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

// nlohmann json related files
#include "nlohmann/json.hpp"
#include "smtk/attribute/json/jsonAttribute.h"
#include "smtk/attribute/json/jsonCollection.h"
#include "smtk/attribute/json/jsonDefinition.h"
#include "smtk/attribute/json/jsonItem.h"

#include <iostream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;
using json = nlohmann::json;

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
  {
    if (argc != 2)
    {
      std::cerr << "Usage: " << argv[0] << " ValidationAttributeFilename\n";
      return -1;
    }
    smtk::attribute::CollectionPtr collPtr = smtk::attribute::Collection::create();
    smtk::attribute::Collection& collection(*collPtr.get());
    std::cout << "Collection Created\n";
    // Lets add some analyses
    std::set<std::string> analysis;
    analysis.insert("Flow");
    analysis.insert("General");
    analysis.insert("Time");
    collection.defineAnalysis("CFD Flow", analysis);
    analysis.clear();

    analysis.insert("Flow");
    analysis.insert("Heat");
    analysis.insert("General");
    analysis.insert("Time");
    collection.defineAnalysis("CFD Flow with Heat Transfer", analysis);
    analysis.clear();

    analysis.insert("Constituent");
    analysis.insert("General");
    analysis.insert("Time");
    collection.defineAnalysis("Constituent Transport", analysis);
    analysis.clear();

    double lcolor1[] = { 1.0, 1.0, 0.0, 0.1 };
    double lcolor2[] = { 1.0, 0.0, 1.0, 0.2 };
    double lcolor3[] = { 0.0, 1.0, 1.0, 0.3 };
    collection.addAdvanceLevel(0, "Level 0", lcolor1);
    collection.addAdvanceLevel(1, "Level 1", lcolor2);
    collection.addAdvanceLevel(2, "Level 2", lcolor3);

    // Lets create an attribute to represent an expression
    smtk::attribute::DefinitionPtr expDef = collection.createDefinition("ExpDef");
    expDef->setIsAbstract(true);
    expDef->setNotApplicableColor(1, 0, 0, 1);
    expDef->setDefaultColor(0, 1, 0, 1);
    expDef->createLocalAssociationRule();
    expDef->setBriefDescription("Sample Expression");
    expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
    smtk::attribute::StringItemDefinitionPtr eitemdef =
      expDef->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("Expression String");
    smtk::attribute::StringItemDefinitionPtr eitemdef2 =
      expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
    eitemdef->setDefaultValue("sample");
    std::cout << "number of itemDefs: " << expDef->numberOfItemDefinitions() << std::endl;

    smtk::attribute::DefinitionPtr base = collection.createDefinition("BaseDef");
    // Lets add some item definitions
    smtk::attribute::IntItemDefinitionPtr iitemdef1 =
      base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("TEMPORAL");
    iitemdef1->setIsExtensible(true);
    iitemdef1->setCommonValueLabel("Time");
    iitemdef1->addDiscreteValue(0, "Seconds");
    iitemdef1->addDiscreteValue(1, "Minutes");
    iitemdef1->addDiscreteValue(2, "Hours");
    iitemdef1->addDiscreteValue(3, "Days");
    iitemdef1->setDefaultDiscreteIndex(0);
    iitemdef1->addCategory("Time");
    smtk::attribute::IntItemDefinitionPtr iitemdef2 =
      base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("IntItem2");
    iitemdef2->setDefaultValue(10);
    iitemdef2->addCategory("Heat");

    smtk::attribute::DefinitionPtr def1 = collection.createDefinition("Derived1", "BaseDef");
    def1->setLocalAssociationMask(smtk::model::MODEL_ENTITY); // belongs on model
    // Lets add some item definitions
    smtk::attribute::DoubleItemDefinitionPtr ditemdef =
      def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem1");
    // Allow this one to hold an expression
    ditemdef->addCategory("Veg");
    ditemdef->setExpressionDefinition(expDef);
    // Check to make sure we can use expressions
    if (!ditemdef->allowsExpressions())
    {
      std::cout << "ERROR - Item Def does not allow expressions\n";
      status = -1;
    }
    smtk::attribute::DoubleItemDefinitionPtr ditemdef2 =
      def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem2");
    ditemdef2->setDefaultValue(-35.2);
    ditemdef2->setMinRange(-100, true);
    ditemdef2->setMaxRange(125.0, false);
    ditemdef2->addCategory("Constituent");
    smtk::attribute::VoidItemDefinitionPtr vdef =
      def1->addItemDefinition<smtk::attribute::VoidItemDefinitionPtr>("VoidItem");
    vdef->setIsOptional(true);
    vdef->setLabel("Option 1");

    smtk::attribute::DefinitionPtr def2 = collection.createDefinition("Derived2", "Derived1");
    def2->setLocalAssociationMask(smtk::model::VOLUME);
    // Lets add some item definitions
    smtk::attribute::StringItemDefinitionPtr sitemdef =
      def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem1");
    sitemdef->setIsMultiline(true);
    sitemdef->addCategory("Flow");
    smtk::attribute::StringItemDefinitionPtr sitemdef2 =
      def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem2");
    sitemdef2->setDefaultValue("Default");
    sitemdef2->addCategory("General");
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
    smtk::attribute::StringItemDefinitionPtr sitemdef3 =
      gdef1->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("GroupString");
    sitemdef3->setDefaultValue("Something Cool");
    sitemdef3->addCategory("General");
    sitemdef3->addCategory("Flow");

    // Add in a Attribute definition with a reference to another attribute
    smtk::attribute::DefinitionPtr attrefdef = collection.createDefinition("AttributeReferenceDef");
    smtk::attribute::RefItemDefinitionPtr aritemdef =
      attrefdef->addItemDefinition<smtk::attribute::RefItemDefinitionPtr>("BaseDefItem");
    aritemdef->setCommonValueLabel("A reference to another attribute");
    aritemdef->setAttributeDefinition(base);

    // Process Categories
    collection.updateCategories();
    std::cout << "categories original size: " << collection.numberOfCategories() << std::endl;
    // Lets test creating an attribute by passing in the expression definition explicitly
    smtk::attribute::AttributePtr expAtt = collection.createAttribute("Exp1", expDef);
    smtk::attribute::AttributePtr att = collection.createAttribute("testAtt", "Derived2");
    if (!att)
    {
      std::cout << "ERROR: Attribute testAtt not created\n";
      status = -1;
    }

    smtk::view::ViewPtr rootView = collection.findTopLevelView();
    if (!rootView)
    {
      rootView = smtk::view::View::New("Group", "RootView");
      rootView->details().setAttribute("TopLevel", "true");
      collection.addView(rootView);
      smtk::view::View::Component& temp = rootView->details().addChild("Views");
      temp.setContents("fooContent");
      (void)temp;
      // Add a second view
      smtk::view::ViewPtr secondView = smtk::view::View::New("Group", "SecondView");
      collection.addView(secondView);
    }
    /**************************************************************/
    json j = collPtr;
    // create a valid shared ptr
    //std::cout << "Collection to Json:\n" <<j.dump(2) <<std::endl;

    // Test from_json functionality
    {
      auto inputColPtr = smtk::attribute::Collection::create();
      inputColPtr = j;
      inputColPtr->updateCategories();
      std::cout << "categories size: " << inputColPtr->numberOfCategories() << std::endl;
      std::cout << "number of itemDefs: "
                << inputColPtr->findDefinition("ExpDef")->numberOfItemDefinitions() << std::endl;

      json jTest = inputColPtr;
      std::cout << "Collection from Json:\n" << jTest.dump(2) << std::endl;
      bool toFromMatchFlag = (j == jTest);
      std::cout << "Json matches? " << toFromMatchFlag << std::endl;
    }
    // write json result into a file
    // std::ofstream outfile;
    // outfile.open(argv[1], std::ofstream::out | std::ofstream::trunc);
    // if (!outfile)
    // {
    //   std::cerr << "Error opening file for writing: " << argv[1] << std::endl;
    // }
    // else
    // {
    //   outfile << j.dump(2);
    // }
    // outfile.close();
    // Read a json file and do comparson
  }
  return status;
}
