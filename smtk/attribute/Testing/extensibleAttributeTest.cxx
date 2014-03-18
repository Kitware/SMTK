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
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/RefItemDefinition.h"
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
#include "smtk/model/Item.h" // just needed for enum
#include "smtk/util/AttributeWriter.h"
#include "smtk/util/AttributeReader.h"
#include "smtk/util/Logger.h"

#include <iostream>

int main(int argc, char *argv[])
{
  int status = 0;
  {
  if (argc != 3)
    {
    std::cerr << "Usage: " << argv[0] << " ExtensibleTestTemplate OutputFileName\n";
    return -1;
    }
  smtk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  smtk::util::AttributeReader reader;
  smtk::util::Logger logger;
  if (reader.read(manager, argv[1], logger))
    {
    std::cerr << "Errors encountered reading Attribute File: " << argv[1] << "\n";
    std::cerr << logger.convertToString();
    status = -1;
    }
  else
    {
    std::cout << "Read in template!\n";
    }

  // Lets find the extensible definitions
  smtk::attribute::DefinitionPtr def = manager.findDefinition("Derived2");
  if (!def)
    {
    std::cerr << "Could not find Derived 2 Def!\n";
    return -2;
    }
  int pos = def->findItemPosition("StringItem1");
  if (pos < 0)
    {
    std::cerr << "Could not find StringItem1!\n";
    return -3;
    }
  else
    {
    std::cout << "Pos = " << pos << std::endl;
    }
  std::cout << "Offset = " << def->itemOffset() << std::endl;

  int i, n = def->numberOfItemDefinitions();
  for (i = 0; i < n; i++)
    {
    std::cout << i << ":" << def->itemDefinition(i)->name() << std::endl;
    }
  if (pos < 0)
    {
    std::cerr << "Could not find StringItem1!\n";
    return -3;
    }
  smtk::attribute::ItemDefinitionPtr idef = def->itemDefinition(pos);
  smtk::attribute::StringItemDefinitionPtr sdef =
    smtk::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(def->itemDefinition(pos));
  if (!sdef)
    {
    std::cerr << "StringItem1 Def is not a string! at pos: " << pos << " Name: "
              << idef->name() << "\n";
    return -4;
    }

  std::cout << "Manager destroyed\n";
  }
  return status;
}
