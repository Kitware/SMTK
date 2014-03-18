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

const char *itemNames[] = {
  "IntItem1", "IntItem2", "DoubleItem1", "DoubleItem2", "StringItem1", "StringItem2"
};

int checkStringItemDef(const char *name, smtk::attribute::DefinitionPtr def)
{
  int pos = def->findItemPosition(name);
  if (pos < 0)
    {
    std::cerr << "Could not find " << name << "!\n";
    return -1;
    }
  else
    {
    std::cout << "Pos of " << name << " = " << pos << std::endl;
    }
  smtk::attribute::StringItemDefinitionPtr sdef =
    smtk::dynamic_pointer_cast<smtk::attribute::StringItemDefinition>(def->itemDefinition(pos));
  if (!sdef)
    {
    std::cerr << name << " Def is not a string! at pos: " << pos << "\n";
    return -1;
    }

  // Is it extensible?
  if (sdef->isExtensible())
    {
    std::cout << name << " is extensible!, NumOfRequired Values: " << sdef->numberOfRequiredValues()
              << " MaxNumberOfValues: " << sdef->maxNumberOfValues() << "\n";
    }
  else
    {
    std::cerr << name << " Def is not extensible\n";
    return -1;
    }
  return pos;
}

int checkStringItem(const char *name, smtk::attribute::AttributePtr att)
{
  int status = 0;
  smtk::attribute::ItemPtr item = att->find(name);
  if (!item)
    {
    std::cerr << name << " could not be found\n";
    return -1;
    }
  smtk::attribute::StringItemPtr sitem =
    smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(item);
  if (!sitem)
    {
    std::cerr << name << " is not a string\n";
    return -1;
    }
  std::size_t minN = sitem->numberOfRequiredValues(), maxN = sitem->maxNumberOfValues();
  std::cout << name << ": NumOfRequired Values: " << minN
              << " MaxNumberOfValues: " << maxN << "\n";
  // Is the number of initial values correct?
  if (minN != sitem->numberOfValues())
    {
    std::cerr << name << "'s initial size is not correct RequiredSize:" << minN
              << " Initial Size: " << sitem->numberOfValues() << "\n";
    status = -1;
    }
  else
    {
    std::cout << name << " is have correct initial size\n";
    }

  if (minN)
    {
    // Can we delete below the required number of values?
    if (sitem->removeValue(0))
      {
      std::cerr << name << " allowed deleting below min size. RequiredSize:" << minN
                << " Current Size: " << sitem->numberOfValues() << "\n";
      status = -1;
      }
    else
      {
      std::cout << name << " passed deleting below required size test\n";
      }
    }
  else
    {
    std::cout << name << " had no min size\n";
    }
  // Can we add to the item?
  if (sitem->appendValue("New Val"))
    {
    std::cout << name << " allowed appending of new value. RequiredSize:" << minN
              << " Current Size: " << sitem->numberOfValues() << "\n";
    }
  else
    {
    std::cout << name << " failed append test!\n";
    status = -1;
    }
  // Can we remove from the item?
  if (sitem->removeValue(0))
    {
    std::cout << name << " allowed removing of 0th val. RequiredSize:" << minN
              << " Current Size: " << sitem->numberOfValues() << "\n";
    }
  else
    {
    std::cout << name << " failed remove test!\n";
    status = -1;
    }
  if (maxN)
    {
    // add enough values to reach max number
    while (sitem->numberOfValues() != maxN)
      {
      sitem->appendValue("Max N Test");
      }
    // Now try to append past it
    if (sitem->appendValue("Problem Val"))
      {
      std::cerr << name << " allowed appending above max size. MaxSize:" << maxN
                << " Current Size: " << sitem->numberOfValues() << "\n";
      status = -1;
      }
    else
      {
      std::cout << name  << " passed appending above max size test  MaxSize:" << maxN
                << " Current Size: " << sitem->numberOfValues() << "\n";
      }
    }
  else
    {
    std::cout << name << " had no max size\n";
    }
  return status;
}

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

  smtk::attribute::DefinitionPtr def = manager.findDefinition("Derived2");
  if (!def)
    {
    std::cerr << "Could not find Derived 2 Def!\n";
    return -2;
    }

  int i, n = def->numberOfItemDefinitions();
  if (n != 6)
    {
    std::cerr << "Derived 2 has incorrect number of items!\n";
    return -2;
    }

  for (i = 0; i < n; i++)
    {
    std::cout << i << ":" << def->itemDefinition(i)->name();
    if (def->itemDefinition(i)->name() == itemNames[i])
      {
      std::cout << "- Correct!" << std::endl;
      }
    else
      {
      std::cout << "- Error!" << std::endl;
      status = -2;
      }
    }
  if (status)
    {
    return status;
    }

  // Lets find the extensible definitions
  int pos1 = checkStringItemDef("StringItem1", def);
  if (pos1 < 0)
    {
    std::cerr << "Problem with StringItem1 Def!\n";
    return -3;
    }

  int pos2 = checkStringItemDef("StringItem2", def);
  if (pos2 < 0)
    {
    std::cerr << "Problem with StringItem2 Def!\n";
    return -4;
    }

  // Create an attribute
  smtk::attribute::AttributePtr att = manager.createAttribute("Derived2Att", def);
  if (!att)
    {
    std::cerr << "Could not create Attribute!\n";
    return -5;
    }

  if (checkStringItem("StringItem1", att))
    {
    std::cerr << "Problem with StringItem1!\n";
    return -6;
    }

  if (checkStringItem("StringItem2", att))
    {
    std::cerr << "Problem with StringItem2!\n";
    return -7;
    }


  std::cout << "Manager destroyed\n";
  }
  return status;
}
