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
#include <iostream>

int main()
{
  int status = 0;
    {
    smtk::attribute::Manager manager;
    std::cout << "Manager Created\n";
    smtk::attribute::DefinitionPtr def = manager.createDefinition("testDef");
    if (def)
      {
      std::cout << "Definition testDef created\n";
      }
    else
      {
      std::cout << "ERROR: Definition testDef not created\n";
      status = -1;
      }

    smtk::attribute::AttributePtr att = manager.createAttribute("testDef");
    if (att)
      {
      std::cout << "Attribute " << att->name() << " created\n";
      }
    else
      {
      std::cout << "ERROR: 1st Attribute not created\n";
      status = -1;
      }

    att = manager.createAttribute("testDef");
    if (att)
      {
      std::cout << "Attribute " << att->name() << " created\n";
      }
    else
      {
      std::cout << "ERROR: 2nd Attribute not created\n";
      status = -1;
      }

    att = manager.createAttribute("testDef");
    if (att)
      {
      std::cout << "Attribute " << att->name() << " created\n";
      }
    else
      {
      std::cout << "ERROR: 3rd Attribute not created\n";
      status = -1;
      }
    std::cout << "Manager destroyed\n";
    }
    return status;
}
