/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/

#include "smtk/attribute/Manager.h"
#include "smtk/attribute/AttributeDefinition.h"
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
#include "smtk/util/Logger.h"
#include "smtk/util/XmlV1StringWriter.h"
#include "smtk/util/XmlDocV1Parser.h"
#include "smtk/view/Attribute.h"
#include "smtk/view/Instanced.h"
#include "smtk/view/ModelEntity.h"
#include "smtk/view/Root.h"
#include "smtk/view/SimpleExpression.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml-1.2/src/pugixml.hpp"
#include "pugixml-1.2/src/pugixml.cpp"

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
  AttributeDefinitionPtr funcDef = manager.createAttributeDefinition("PolyLinearFunction");
  AttributeDefinitionPtr materialDef = manager.createAttributeDefinition("Material");
  materialDef->setAssociationMask(0x40); // belongs on domains
  AttributeDefinitionPtr boundaryConditionsDef = manager.createAttributeDefinition("BoundaryCondition");
  boundaryConditionsDef->setAssociationMask(0x20); // belongs on boundaries
  AttributeDefinitionPtr specifiedHeadDef = manager.createAttributeDefinition("SpecifiedHead", "BoundaryCondition");
  AttributeDefinitionPtr specifiedFluxDef = manager.createAttributeDefinition("SpecifiedFlux", "BoundaryCondition");
  AttributeDefinitionPtr injectionWellDef = manager.createAttributeDefinition("InjectionWell", "BoundaryCondition");
  AttributeDefinitionPtr timeParamDef = manager.createAttributeDefinition("TimeParameters");
  AttributeDefinitionPtr globalsDef = manager.createAttributeDefinition("GlobalParameters");
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
  DoubleItemDefinitionPtr ddef;
  ddef = specifiedHeadDef->addItemDefinition<DoubleItemDefinitionPtr>("Value");
  ddef->setExpressionDefinition(funcDef);
  ddef = specifiedFluxDef->addItemDefinition<DoubleItemDefinitionPtr>("Value");
  ddef->setExpressionDefinition(funcDef);

  IntItemDefinitionPtr idef;
  GroupItemDefinitionPtr gdef;
  gdef = timeParamDef->addItemDefinition<GroupItemDefinitionPtr>("StartTime");
  gdef->setCommonSubGroupLabel("Start Time");
  ddef = gdef->addItemDefinition<DoubleItemDefinitionPtr>("Value");
  ddef->addCategory("Time");
  ddef->setDefaultValue(0);
  ddef->setMinRange(0, true);
  idef = gdef->addItemDefinition<IntItemDefinitionPtr>("Units");
  idef->addDiscreteValue(0, "Seconds");
  idef->addDiscreteValue(1, "Minutes");
  idef->addDiscreteValue(2, "Hours");
  idef->addDiscreteValue(3, "Days");
  idef->setDefaultDiscreteIndex(0);
  idef->addCategory("Time");
  gdef = timeParamDef->addItemDefinition<GroupItemDefinitionPtr>("EndTime");
  gdef->setCommonSubGroupLabel("End Time");
  ddef = gdef->addItemDefinition<DoubleItemDefinitionPtr>("Value");
  ddef->addCategory("Time");
  ddef->setDefaultValue(162);
  ddef->setMinRange(0, true);
  idef = gdef->addItemDefinition<IntItemDefinitionPtr>("Units");
  idef->addDiscreteValue(0, "Seconds");
  idef->addDiscreteValue(1, "Minutes");
  idef->addDiscreteValue(2, "Hours");
  idef->addDiscreteValue(3, "Days");
  idef->setDefaultDiscreteIndex(0);
  idef->addCategory("Time");

  ddef = globalsDef->addItemDefinition<DoubleItemDefinitionPtr>("Gravity");
  ddef->setDefaultValue(1.272024e08);
  ddef->setUnits("m/hr^2");
  ddef->setAdvanceLevel(1);
  ddef->setMinRange(0, false);

  ddef = globalsDef->addItemDefinition<DoubleItemDefinitionPtr>("WaterSpecificHeat");
  ddef->setDefaultValue(0.00116);
  ddef->setUnits("W hr/g-K");
  ddef->setAdvanceLevel(1);
  ddef->setMinRange(0, false);

  ddef = globalsDef->addItemDefinition<DoubleItemDefinitionPtr>("AirSpecificHeat");
  ddef->setDefaultValue(0.000278);
  ddef->setUnits("W hr/g-K");
  ddef->setAdvanceLevel(1);
  ddef->setMinRange(0, false);

  // Lets add some Views

  view::RootPtr root = manager.rootView();
  root->setTitle("SimBuilder");
  view::SimpleExpressionPtr expSec = root->addSubView<view::SimpleExpressionPtr>("Functions");
  expSec->setDefinition(funcDef);
  view::AttributePtr attSec;
  attSec = root->addSubView<view::AttributePtr>("Materials");
  attSec->addDefinition(materialDef);
  attSec->setModelEntityMask(0x40);
  attSec->setOkToCreateModelEntities(true);
  view::ModelEntityPtr modSec;
  modSec = root->addSubView<view::ModelEntityPtr>("Domains");
  modSec->setModelEntityMask(0x40); // Look at domains only
  modSec->setDefinition(materialDef); // use tabled view focusing on Material Attributes
  attSec = root->addSubView<view::AttributePtr>("BoundaryConditions");
  attSec->addDefinition(boundaryConditionsDef);
  modSec = root->addSubView<view::ModelEntityPtr>("Boundary View");
  modSec->setModelEntityMask(0x20); // Look at boundary entities only

  manager.updateCategories();
  AttributePtr att = manager.createAttribute("TimeInfomation", timeParamDef);
  view::InstancedPtr iSec;
  iSec = root->addSubView<view::InstancedPtr>("Time Parameters");
  iSec->addInstance(att);
  att = manager.createAttribute("Globals", globalsDef);
  iSec = root->addSubView<view::InstancedPtr>("Global Parameters");
  iSec->addInstance(att);
  smtk::util::Logger logger;
  smtk::util::XmlV1StringWriter writer(manager);
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
  smtk::util::XmlDocV1Parser reader(manager1);
  reader.process(doc);
  if (reader.messageLog().hasErrors())
    {
    std::cerr <<  "Errors encountered parsing Attribute String:\n";
    std::cerr << reader.messageLog().convertToString();
    status = -1;
    }

  smtk::util::XmlV1StringWriter writer1(manager1);
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
