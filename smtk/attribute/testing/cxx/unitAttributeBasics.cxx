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
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItemDefinition.h"

#include <iostream>

#include "smtk/common/testing/cxx/helpers.h"

int unitAttributeBasics(int, char* [])
{
  int status = 0;
  smtk::attribute::ResourcePtr resptr = smtk::attribute::Resource::create();
  smtk::attribute::WeakResourcePtr wresptr = resptr;
  smtk::attribute::Resource& resource(*resptr.get());
  std::cout << "Resource Created\n";

  smtk::attribute::DefinitionPtr def = resource.createDefinition("testDef");
  smtkTest(!!def, "Definition testDef not created.");
  std::cout << "Definition testDef created\n";

  smtk::attribute::DefinitionPtr def1 = resource.createDefinition("testDef");
  smtkTest(!def1, "Duplicated definition testDef created");
  std::cout << "Duplicated definition testDef not created\n";

  smtk::attribute::AttributePtr att = resource.createAttribute("testAtt", "testDef");
  smtkTest(!!att, "Attribute testAtt not created.");
  std::cout << "Attribute testAtt created\n";

  smtk::attribute::AttributePtr att1 = resource.createAttribute("testAtt", "testDef");
  smtkTest(!att1, "Duplicate Attribute testAtt created.");
  std::cout << "Duplicate Attribute testAtt not created\n";

  std::vector<smtk::attribute::AttributePtr> atts;
  std::vector<smtk::attribute::DefinitionPtr> defs;

  // Check to see how many atts and defs are in the resource
  resource.definitions(defs);
  resource.attributes(atts);

  smtkTest(defs.size() == 1, "Incorrect number of definitions reported - definitions returned: "
      << defs.size() << " but should have returned 1.");
  // Is testDef in the list?
  smtkTest(defs[0] == def, "testDef is not in the list!");

  smtkTest(atts.size() == 1, "Incorrect number of attributes reported - attributes returned: "
      << atts.size() << " but should have returned 1.");
  // Is testAtt in the list?
  smtkTest(atts[0] == att, "testAtt is not in the list!");

  smtkTest(!att->isColorSet(), "Color should not be set.");
  double const* tcol = att->color();
  smtkTest(tcol[0] == 1 && tcol[1] == 1 && tcol[2] == 1 && tcol[3] == 1,
    "wrong default color values: " << tcol[0] << " " << tcol[1] << " " << tcol[2] << ' '
                                   << tcol[3]);

  double color[] = { 3, 24, 12, 6 };
  att->setColor(color);
  tcol = att->color();
  smtkTest(tcol[0] == color[0] && tcol[1] == color[1] && tcol[2] == color[2] && tcol[3] == color[3],
    "Wrong set color values: " << tcol[0] << " " << tcol[1] << " " << tcol[2] << ' ' << tcol[3]);

  smtkTest(att->isColorSet(), "Color should be set.");
  att->unsetColor();
  smtkTest(!att->isColorSet(), "Color should not be set.");
  tcol = att->color();
  smtkTest(tcol[0] == 1 && tcol[1] == 1 && tcol[2] == 1 && tcol[3] == 1,
    "Wrong default color values: " << tcol[0] << " " << tcol[1] << " " << tcol[2] << ' '
                                   << tcol[3]);

  smtkTest(att->associatedModelEntityIds().size() == 0, "Should not have associated entity IDs.");
  smtkTest(!att->associatedObjects(), "Should not have associated components.");

  smtkTest(!att->appliesToBoundaryNodes(), "Should not be applied to boundry node.");
  att->setAppliesToBoundaryNodes(true);
  smtkTest(att->appliesToBoundaryNodes(), "Should be applied to boundry node.");
  att->setAppliesToBoundaryNodes(false);
  smtkTest(!att->appliesToBoundaryNodes(), "Should not be applied to boundry node.");
  smtkTest(!att->appliesToInteriorNodes(), "Should not be applied to interior node.");
  att->setAppliesToInteriorNodes(true);
  smtkTest(att->appliesToInteriorNodes(), "Should be applied to interior node.");
  att->setAppliesToInteriorNodes(false);
  smtkTest(!att->appliesToInteriorNodes(), "Should not applied to interior node.");
  smtkTest(att->resource() == resptr, "Should be this resource.");
  resptr = nullptr;
  smtkTest(wresptr.lock() == nullptr, "Resource was not destroyed") return status;
}
