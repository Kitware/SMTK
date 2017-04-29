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
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/System.h"
#include <iostream>

int main()
{
  int status = 0;
  smtk::attribute::SystemPtr sysptr = smtk::attribute::System::create();
  smtk::attribute::System& system(*sysptr.get());
  std::cout << "System Created\n";
  smtk::common::Resource::Type t = system.resourceType();
  if (t != smtk::common::Resource::ATTRIBUTE)
  {
    std::cout << "ERROR: Returned wrong resource type";
    status++;
  }
  std::cout << "Resource type: " << smtk::common::Resource::type2String(t) << "\n";
  smtk::attribute::DefinitionPtr def = system.createDefinition("testDef");
  if (def)
  {
    std::cout << "Definition testDef created\n";
  }
  else
  {
    std::cout << "ERROR: Definition testDef not created\n";
    status++;
  }
  smtk::attribute::DefinitionPtr def1 = system.createDefinition("testDef");
  if (!def1)
  {
    std::cout << "Duplicated definition testDef not created\n";
  }
  else
  {
    std::cout << "ERROR: Duplicated definition testDef created\n";
    status++;
  }
  smtk::attribute::AttributePtr att = system.createAttribute("testAtt", "testDef");
  if (att)
  {
    std::cout << "Attribute testAtt created\n";
  }
  else
  {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status++;
  }

  smtk::attribute::AttributePtr att1 = system.createAttribute("testAtt", "testDef");
  if (!att1)
  {
    std::cout << "Duplicate Attribute testAtt not created\n";
  }
  else
  {
    std::cout << "ERROR: Duplicate Attribute testAtt  created\n";
    status++;
  }

  std::vector<smtk::attribute::AttributePtr> atts;
  std::vector<smtk::attribute::DefinitionPtr> defs;

  // Check to see how many atts and defs are in the system
  system.definitions(defs);
  system.attributes(atts);

  if (defs.size() != 1)
  {
    std::cout << "Incorrect number of definitions reported - definitions returned: " << defs.size()
              << " but should have returned 1\n";
    status++;
  }
  else if (defs[0] != def) // Is testDef in the list?
  {
    std::cout << "testDef is not in the list!\n";
    ++status;
  }

  if (atts.size() != 1)
  {
    std::cout << "Incorrect number of attributes reported - attributes returned: " << atts.size()
              << " but should have returned 1\n";
    status++;
  }
  else if (atts[0] != att) // Is testAtt in the list?
  {
    std::cout << "testAtt is not in the list!\n";
    ++status;
  }

  if (att)
  {
    double color[] = { 3, 24, 12, 6 };
    if (att->isColorSet())
    {
      std::cout << "Color should not be set.\n";
      status++;
    }
    double const* tcol = att->color();
    if (tcol[0] != 1 || tcol[1] != 1 || tcol[2] != 1 || tcol[3] != 1)
    {
      std::cout << "wrong default color values: " << tcol[0] << " " << tcol[1] << " " << tcol[2]
                << ' ' << tcol[3] << std::endl;
      status++;
    }
    att->setColor(color);
    tcol = att->color();
    if (tcol[0] != 3 || tcol[1] != 24 || tcol[2] != 12 || tcol[3] != 6)
    {
      std::cout << "wrong set color values: " << tcol[0] << " " << tcol[1] << " " << tcol[2] << ' '
                << tcol[3] << std::endl;
      status++;
    }
    if (!att->isColorSet())
    {
      std::cout << "Color should be set.\n";
      status++;
    }
    att->unsetColor();
    if (att->isColorSet())
    {
      std::cout << "Color should not be set.\n";
      status++;
    }
    tcol = att->color();
    if (tcol[0] != 1 || tcol[1] != 1 || tcol[2] != 1 || tcol[3] != 1)
    {
      std::cout << "wrong default color values: " << tcol[0] << " " << tcol[1] << " " << tcol[2]
                << ' ' << tcol[3] << std::endl;
      status++;
    }
    if (att->associatedModelEntityIds().size() != 0)
    {
      std::cout << "Should not have associated entity IDs.\n";
      status++;
    }
    if (att->appliesToBoundaryNodes())
    {
      std::cout << "Should not be applies to boundry node.\n";
      status++;
    }
    att->setAppliesToBoundaryNodes(true);
    if (!att->appliesToBoundaryNodes())
    {
      std::cout << "Should be applies to boundry node.\n";
      status++;
    }
    att->setAppliesToBoundaryNodes(false);
    if (att->appliesToBoundaryNodes())
    {
      std::cout << "Should not be applies to boundry node.\n";
      status++;
    }
    if (att->appliesToInteriorNodes())
    {
      std::cout << "Should not be applies to interior node.\n";
      status++;
    }
    att->setAppliesToInteriorNodes(true);
    if (!att->appliesToInteriorNodes())
    {
      std::cout << "Should be applies to interior node.\n";
      status++;
    }
    att->setAppliesToInteriorNodes(false);
    if (att->appliesToInteriorNodes())
    {
      std::cout << "Should not applies to interior node.\n";
      status++;
    }
    if (att->system() != sysptr)
    {
      std::cout << "Should be this system.\n";
      status++;
    }
  }

  std::cout << "System destroyed\n";
  return status;
}
