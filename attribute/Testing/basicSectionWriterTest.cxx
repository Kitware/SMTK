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

#include "attribute/Manager.h"
#include "attribute/Definition.h"
#include "attribute/Attribute.h"
#include "attribute/AttributeSection.h"
#include "attribute/InstancedSection.h"
#include "attribute/IntItem.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/DirectoryItemDefinition.h"
#include "attribute/FileItemDefinition.h"
#include "attribute/GroupItemDefinition.h"
#include "attribute/ModelEntitySection.h"
#include "attribute/RootSection.h"
#include "attribute/SimpleExpressionSection.h"
#include "attribute/StringItem.h"
#include "attribute/StringItemDefinition.h"
#include "attribute/VoidItemDefinition.h"
#include "attribute/XmlV1StringWriter.h"
#include "attribute/XmlDocV1Parser.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml-1.2/src/pugixml.hpp"
#include "pugixml-1.2/src/pugixml.cpp"

#include <iostream>
#include <sstream>

using namespace slctk;
int main()
{
  int status;
  {
  attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create some attribute Definitions
  AttributeDefinitionPtr funcDef = manager.createDefinition("PolyLinearFunction");
  AttributeDefinitionPtr materialDef = manager.createDefinition("Material");
  materialDef->setAssociationMask(0x40); // belongs on domains
  AttributeDefinitionPtr boundaryConditionsDef = manager.createDefinition("BoundaryCondition");
  boundaryConditionsDef->setAssociationMask(0x20); // belongs on boundaries
  AttributeDefinitionPtr specifiedHeadDef = manager.createDefinition("SpecifiedHead", "BoundaryCondition");
  AttributeDefinitionPtr specifiedFluxDef = manager.createDefinition("SpecifiedFlux", "BoundaryCondition");
  AttributeDefinitionPtr injectionWellDef = manager.createDefinition("InjectionWell", "BoundaryCondition");
  AttributeDefinitionPtr timeParamDef = manager.createDefinition("TimeParameters");
  AttributeDefinitionPtr globalsDef = manager.createDefinition("GlobalParameters");
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

  // Lets add some sections
  
  RootSectionPtr root = manager.rootSection();
  root->setTitle("SimBuilder");
  SimpleExpressionSectionPtr expSec = root->addSubsection<SimpleExpressionSectionPtr>("Functions");
  expSec->setDefinition(funcDef);
  AttributeSectionPtr attSec;
  attSec = root->addSubsection<AttributeSectionPtr>("Materials");
  attSec->addDefinition(materialDef);
  attSec->setModelEntityMask(0x40);
  attSec->setOkToCreateModelEntities(true);
  ModelEntitySectionPtr modSec;
  modSec = root->addSubsection<ModelEntitySectionPtr>("Domains");
  modSec->setModelEntityMask(0x40); // Look at domains only
  modSec->setDefinition(materialDef); // use tabled view focusing on Material Attributes
  attSec = root->addSubsection<AttributeSectionPtr>("BoundaryConditions");
  attSec->addDefinition(boundaryConditionsDef);
  modSec = root->addSubsection<ModelEntitySectionPtr>("Boundary View");
  modSec->setModelEntityMask(0x20); // Look at boundary entities only

  manager.updateCategories();
  AttributePtr att = manager.createAttribute("TimeInfomation", timeParamDef);
  InstancedSectionPtr iSec;
  iSec = root->addSubsection<InstancedSectionPtr>("Time Parameters");
  iSec->addInstance(att);
  att = manager.createAttribute("Globals", globalsDef);
  iSec = root->addSubsection<InstancedSectionPtr>("Global Parameters");
  iSec->addInstance(att);
  attribute::XmlV1StringWriter writer(manager);
  std::string result = writer.convertToString();
  std::cout << result << std::endl;
  std::stringstream test(result);
  pugi::xml_document doc;
  doc.load(test);
  attribute::Manager manager1;  
  slctk::attribute::XmlDocV1Parser reader(manager1);
  reader.process(doc);
  std::string readErrors = reader.errorStatus();
  if (readErrors != "")
    {
    std::cerr << "READ ERRORS ENCOUNTERED!!!\n";
    std::cerr << readErrors << "\n";
    }
  attribute::XmlV1StringWriter writer1(manager1);
  std::cout << "Manager 1:\n";
  result = writer1.convertToString();
  std::cout << result << std::endl;

  std::cout << "Manager destroyed\n";
  }
  return status;
}
