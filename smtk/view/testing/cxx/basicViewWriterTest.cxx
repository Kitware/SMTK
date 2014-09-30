//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Manager.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/model/EntityTypeBits.h" // for BitFlags type

#include "smtk/io/Logger.h"
#include "smtk/io/XmlV2StringWriter.h"
#include "smtk/io/XmlDocV2Parser.h"

#include "smtk/view/Attribute.h"
#include "smtk/view/Instanced.h"
#include "smtk/view/ModelEntity.h"
#include "smtk/view/Root.h"
#include "smtk/view/SimpleExpression.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.hpp"
#include "pugixml/src/pugixml.cpp"

#include <iostream>
#include <sstream>

using namespace smtk;
int main()
{
  int status=0;
  {
  attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create some attribute Definitions
  attribute::DefinitionPtr funcDef = manager.createDefinition("PolyLinearFunction");
  attribute::DefinitionPtr materialDef = manager.createDefinition("Material");
  materialDef->setAssociationMask(smtk::model::VOLUME); // belongs on regions for 3D problem
  attribute::DefinitionPtr boundaryConditionsDef = manager.createDefinition("BoundaryCondition");
  boundaryConditionsDef->setAssociationMask(smtk::model::FACE); // belongs on boundaries for 3D problem
  attribute::DefinitionPtr specifiedHeadDef = manager.createDefinition("SpecifiedHead", "BoundaryCondition");
  attribute::DefinitionPtr specifiedFluxDef = manager.createDefinition("SpecifiedFlux", "BoundaryCondition");
  attribute::DefinitionPtr injectionWellDef = manager.createDefinition("InjectionWell", "BoundaryCondition");
  attribute::DefinitionPtr timeParamDef = manager.createDefinition("TimeParameters");
  attribute::DefinitionPtr globalsDef = manager.createDefinition("GlobalParameters");
  // Lets add some analyses
  std::set<std::string> analysis;
  analysis.insert("Flow");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("CFD Flow", analysis);
  analysis.clear();

  analysis.insert("Flow");
  analysis.insert("Heat");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("CFD Flow with Heat Transfer", analysis);
  analysis.clear();

  analysis.insert("Constituent");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("Constituent Transport", analysis);
  analysis.clear();

  // Lets complete the definition for some boundary conditions
  attribute::DoubleItemDefinitionPtr ddef;
  ddef = specifiedHeadDef->addItemDefinition<attribute::DoubleItemDefinitionPtr>("Value");
  ddef->setExpressionDefinition(funcDef);
  ddef = specifiedFluxDef->addItemDefinition<attribute::DoubleItemDefinitionPtr>("Value");
  ddef->setExpressionDefinition(funcDef);

  attribute::IntItemDefinitionPtr idef;
  attribute::GroupItemDefinitionPtr gdef;
  gdef = timeParamDef->addItemDefinition<attribute::GroupItemDefinitionPtr>("StartTime");
  gdef->setCommonSubGroupLabel("Start Time");
  ddef = gdef->addItemDefinition<attribute::DoubleItemDefinitionPtr>("Value");
  ddef->addCategory("Time");
  ddef->setDefaultValue(0);
  ddef->setMinRange(0, true);
  idef = gdef->addItemDefinition<attribute::IntItemDefinitionPtr>("Units");
  idef->addDiscreteValue(0, "Seconds");
  idef->addDiscreteValue(1, "Minutes");
  idef->addDiscreteValue(2, "Hours");
  idef->addDiscreteValue(3, "Days");
  idef->setDefaultDiscreteIndex(0);
  idef->addCategory("Time");
  gdef = timeParamDef->addItemDefinition<attribute::GroupItemDefinitionPtr>("EndTime");
  gdef->setCommonSubGroupLabel("End Time");
  ddef = gdef->addItemDefinition<attribute::DoubleItemDefinitionPtr>("Value");
  ddef->addCategory("Time");
  ddef->setDefaultValue(162);
  ddef->setMinRange(0, true);
  idef = gdef->addItemDefinition<attribute::IntItemDefinitionPtr>("Units");
  idef->addDiscreteValue(0, "Seconds");
  idef->addDiscreteValue(1, "Minutes");
  idef->addDiscreteValue(2, "Hours");
  idef->addDiscreteValue(3, "Days");
  idef->setDefaultDiscreteIndex(0);
  idef->addCategory("Time");

  ddef = globalsDef->addItemDefinition<attribute::DoubleItemDefinitionPtr>("Gravity");
  ddef->setDefaultValue(1.272024e08);
  ddef->setUnits("m/hr^2");
  ddef->setAdvanceLevel(0, 1); //Set Read to require advance level 1 and write level 2
  ddef->setAdvanceLevel(1, 2);
  ddef->setMinRange(0, false);

  ddef = globalsDef->addItemDefinition<attribute::DoubleItemDefinitionPtr>("WaterSpecificHeat");
  ddef->setDefaultValue(0.00116);
  ddef->setUnits("W hr/g-K");
  ddef->setAdvanceLevel(1);
  ddef->setMinRange(0, false);

  ddef = globalsDef->addItemDefinition<attribute::DoubleItemDefinitionPtr>("AirSpecificHeat");
  ddef->setDefaultValue(0.000278);
  ddef->setUnits("W hr/g-K");
  ddef->setAdvanceLevel(1);
  ddef->setMinRange(0, false);

  ddef = globalsDef->addItemDefinition<attribute::DoubleItemDefinitionPtr>("SomethingWithNoValue");

  // Lets add some Views

  view::RootPtr root = manager.rootView();
  root->setTitle("SimBuilder");
  view::SimpleExpressionPtr expSec = root->addSubView<view::SimpleExpressionPtr>("Functions");
  expSec->setDefinition(funcDef);
  view::AttributePtr attSec;
  attSec = root->addSubView<view::AttributePtr>("Materials");
  attSec->addDefinition(materialDef);
  attSec->setModelEntityMask(smtk::model::VOLUME);
  attSec->setOkToCreateModelEntities(true);
  view::ModelEntityPtr modSec;
  modSec = root->addSubView<view::ModelEntityPtr>("Domains");
  modSec->setModelEntityMask(smtk::model::VOLUME); // Look at domains only for 3D
  modSec->setDefinition(materialDef); // use tabled view focusing on Material Attributes
  attSec = root->addSubView<view::AttributePtr>("BoundaryConditions");
  attSec->addDefinition(boundaryConditionsDef);
  modSec = root->addSubView<view::ModelEntityPtr>("Boundary View");
  modSec->setModelEntityMask(smtk::model::FACE); // Look at 3D boundary entities only

  manager.updateCategories();
  attribute::AttributePtr att = manager.createAttribute("TimeInformation", timeParamDef);
  view::InstancedPtr iSec;
  iSec = root->addSubView<view::InstancedPtr>("Time Parameters");
  iSec->addInstance(att);
  att = manager.createAttribute("Globals", globalsDef);
  iSec = root->addSubView<view::InstancedPtr>("Global Parameters");
  iSec->addInstance(att);
  smtk::io::Logger logger;
  smtk::io::XmlV2StringWriter writer(manager);
  std::string result = writer.convertToString(logger);
  std::cout << result << std::endl;
  if (logger.hasErrors())
    {
    std::cerr <<  "Errors encountered creating Attribute String:\n";
    std::cerr << logger.convertToString();
    }

  std::stringstream test(result);
  pugi::xml_document doc;
  doc.load(test);
  attribute::Manager manager1;
  smtk::io::XmlDocV2Parser reader(manager1);
  reader.process(doc);
  if (reader.messageLog().hasErrors())
    {
    std::cerr <<  "Errors encountered parsing Attribute String:\n";
    std::cerr << reader.messageLog().convertToString();
    status = -1;
    }

  smtk::io::XmlV2StringWriter writer1(manager1);
  std::cout << "Manager 1:\n";
  result = writer1.convertToString(logger);
  std::cout << result << std::endl;

  if (logger.hasErrors())
    {
    std::cerr <<  "Errors encountered creating Attribute String 2nd Pass:\n";
    std::cerr << logger.convertToString();
    }

  std::cout << "Manager destroyed\n";
  }
  return status;
}
