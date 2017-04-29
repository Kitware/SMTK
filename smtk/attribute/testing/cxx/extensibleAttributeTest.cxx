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
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/VoidItemDefinition.h"

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

const char* d2items[] = { "IntItem1", "IntItem2", "DoubleItem1", "DoubleItem2", "StringItem1",
  "StringItem2", "StringItem3", "FileItem1", "FileItem2", "FileItem3", "DirectoryItem1",
  "DirectoryItem2", "DirectoryItem3" };

const char* d3items[] = { "IntItem1", "IntItem2", "DoubleItem1", "DoubleItem2", "GroupItem1",
  "GroupItem2", "GroupItem3" };

int checkGroupItemDef(const char* name, smtk::attribute::DefinitionPtr def, bool isExtensible)
{
  int pos = def->findItemPosition(name);
  if (pos < 0)
  {
    std::cerr << "Could not find " << name << "! - ERROR\n";
    return -1;
  }
  else
  {
    std::cout << "Pos of " << name << " = " << pos << std::endl;
  }
  smtk::attribute::GroupItemDefinitionPtr gdef =
    smtk::dynamic_pointer_cast<smtk::attribute::GroupItemDefinition>(def->itemDefinition(pos));
  if (!gdef)
  {
    std::cerr << name << " Def is not a group! at pos: " << pos << " - ERROR\n";
    return -1;
  }
  // Is this suppose to be extenisble
  // Is it extensible?
  if (isExtensible)
  {
    if (gdef->isExtensible())
    {
      std::cout << name
                << " is extensible!, NumOfRequired Groups: " << gdef->numberOfRequiredGroups()
                << " MaxNumberOfGroups: " << gdef->maxNumberOfGroups() << " - PASSED\n";
    }
    else
    {
      std::cerr << name << " Def is not extensible - ERROR\n";
      return -1;
    }
  }
  else
  {
    if (!gdef->isExtensible())
    {
      std::cout << name << " is not extensible - PASSED\n";
    }
    else
    {
      std::cerr << name << " Def is  extensible - ERROR\n";
      return -1;
    }
  }

  return pos;
}

int checkGroupItem(const char* name, smtk::attribute::AttributePtr att, bool isExtensible)
{
  int status = 0;
  smtk::attribute::ItemPtr item = att->find(name);
  if (!item)
  {
    std::cerr << name << " could not be found\n";
    return -1;
  }
  smtk::attribute::GroupItemPtr gitem =
    smtk::dynamic_pointer_cast<smtk::attribute::GroupItem>(item);
  if (!gitem)
  {
    std::cerr << name << " is not a group\n";
    return -1;
  }
  std::size_t minN = gitem->numberOfRequiredGroups(), maxN = gitem->maxNumberOfGroups();
  std::cout << name << ": NumOfRequired Groups: " << minN << " MaxNumberOfGroups: " << maxN << "\n";
  if (!isExtensible)
  {
    if (!gitem->appendGroup())
    {
      std::cout << name << " did not allow append op for a fixed length item - PASSED\n";
    }
    else
    {
      std::cout << name
                << " allowed appending of new value for a fixed length item. RequiredSize:" << minN
                << " Current Size: " << gitem->numberOfGroups() << " - ERROR\n";
      status = -1;
    }
    // Can we remove from the item?
    if (!gitem->removeGroup(0))
    {
      std::cout << name << " did not allow remove op for a fixed length item - PASSED\n";
    }
    else
    {
      std::cout << name
                << " allowed removing of  value for a fixed length item. RequiredSize:" << minN
                << " Current Size: " << gitem->numberOfGroups() << " - ERROR\n";
      status = -1;
    }
    return status;
  }

  // Is the number of initial values correct?
  if (minN != gitem->numberOfGroups())
  {
    std::cerr << name << "'s initial size is not correct RequiredSize:" << minN
              << " Initial Size: " << gitem->numberOfGroups() << " - ERROR\n";
    status = -1;
  }
  else
  {
    std::cout << name << " is have correct initial size - PASSED\n";
  }

  if (minN)
  {
    // Can we delete below the required number of values?
    if (gitem->removeGroup(0))
    {
      std::cerr << name << " allowed deleting below min size. RequiredSize:" << minN
                << " Current Size: " << gitem->numberOfGroups() << " - ERROR\n";
      status = -1;
    }
    else
    {
      std::cout << name << " - attempting to delete below required size test - PASSED\n";
    }
  }
  else
  {
    std::cout << name << " had no min size\n";
  }
  // Does resizing work?
  gitem->setNumberOfGroups(minN + 1);
  if (gitem->numberOfGroups() != (minN + 1))
  {
    std::cerr << name << " did not resize correctly. RequiredSize:" << minN
              << " Current Size: " << gitem->numberOfGroups() << " - ERROR\n";
    status = -1;
  }
  else
  {
    std::cout << name << "'s resize test - PASSED\n";
  }

  // Can we add to the item?
  if (gitem->appendGroup())
  {
    std::cout << name << "'s attempt of appending of new value. RequiredSize:" << minN
              << " Current Size: " << gitem->numberOfGroups() << " - PASSED\n";
  }
  else
  {
    std::cout << name << "'s append test - ERROR\n";
    status = -1;
  }
  // Can we remove from the item?
  if (gitem->removeGroup(0))
  {
    std::cout << name << " allowed removing of 0th val. RequiredSize:" << minN
              << " Current Size: " << gitem->numberOfGroups() << "- PASSED\n";
  }
  else
  {
    std::cout << name << "'s remove test - ERROR\n";
    status = -1;
  }
  if (maxN)
  {
    // add enough values to reach max number
    while (gitem->numberOfGroups() != maxN)
    {
      gitem->appendGroup();
    }
    // Now try to append past it
    if (gitem->appendGroup())
    {
      std::cerr << name << " allowed appending above max size. MaxSize:" << maxN
                << " Current Size: " << gitem->numberOfGroups() << " - FAILED\n";
      status = -1;
    }
    else
    {
      std::cout << name << "'s appending above max size test  MaxSize:" << maxN
                << " Current Size: " << gitem->numberOfGroups() << " - PASSED\n";
    }
  }
  else
  {
    std::cout << name << " had no max size (IGNORED)\n";
  }
  return status;
}

int checkStringItemDef(const char* name, smtk::attribute::DefinitionPtr def, bool isExtensible)
{
  int pos = def->findItemPosition(name);
  if (pos < 0)
  {
    std::cerr << "Could not find " << name << "! - ERROR\n";
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
    std::cerr << name << " Def is not a string! at pos: " << pos << " - ERROR\n";
    return -1;
  }
  // Is this suppose to be extenisble
  // Is it extensible?
  if (isExtensible)
  {
    if (sdef->isExtensible())
    {
      std::cout << name
                << " is extensible!, NumOfRequired Values: " << sdef->numberOfRequiredValues()
                << " MaxNumberOfValues: " << sdef->maxNumberOfValues() << " - PASSED\n";
    }
    else
    {
      std::cerr << name << " Def is not extensible - ERROR\n";
      return -1;
    }
  }
  else
  {
    if (!sdef->isExtensible())
    {
      std::cout << name << " is not extensible - PASSED\n";
    }
    else
    {
      std::cerr << name << " Def is  extensible - ERROR\n";
      return -1;
    }
  }

  return pos;
}

int checkFileItemDef(const char* name, smtk::attribute::DefinitionPtr def, bool isExtensible)
{
  int pos = def->findItemPosition(name);
  if (pos < 0)
  {
    std::cerr << "Could not find " << name << "! - ERROR\n";
    return -1;
  }
  else
  {
    std::cout << "Pos of " << name << " = " << pos << std::endl;
  }
  auto sdef =
    smtk::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(def->itemDefinition(pos));
  if (!sdef)
  {
    std::cerr << name << " Def is not a file! at pos: " << pos << " - ERROR\n";
    return -1;
  }
  // Is this suppose to be extenisble
  // Is it extensible?
  if (isExtensible)
  {
    if (sdef->isExtensible())
    {
      std::cout << name
                << " is extensible!, NumOfRequired Values: " << sdef->numberOfRequiredValues()
                << " MaxNumberOfValues: " << sdef->maxNumberOfValues() << " - PASSED\n";
    }
    else
    {
      std::cerr << name << " Def is not extensible - ERROR\n";
      return -1;
    }
  }
  else
  {
    if (!sdef->isExtensible())
    {
      std::cout << name << " is not extensible - PASSED\n";
    }
    else
    {
      std::cerr << name << " Def is  extensible - ERROR\n";
      return -1;
    }
  }

  return pos;
}

int checkDirectoryItemDef(const char* name, smtk::attribute::DefinitionPtr def, bool isExtensible)
{
  int pos = def->findItemPosition(name);
  if (pos < 0)
  {
    std::cerr << "Could not find " << name << "! - ERROR\n";
    return -1;
  }
  else
  {
    std::cout << "Pos of " << name << " = " << pos << std::endl;
  }
  auto sdef =
    smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItemDefinition>(def->itemDefinition(pos));
  if (!sdef)
  {
    std::cerr << name << " Def is not a directory! at pos: " << pos << " - ERROR\n";
    return -1;
  }
  // Is this suppose to be extenisble
  // Is it extensible?
  if (isExtensible)
  {
    if (sdef->isExtensible())
    {
      std::cout << name
                << " is extensible!, NumOfRequired Values: " << sdef->numberOfRequiredValues()
                << " MaxNumberOfValues: " << sdef->maxNumberOfValues() << " - PASSED\n";
    }
    else
    {
      std::cerr << name << " Def is not extensible - ERROR\n";
      return -1;
    }
  }
  else
  {
    if (!sdef->isExtensible())
    {
      std::cout << name << " is not extensible - PASSED\n";
    }
    else
    {
      std::cerr << name << " Def is  extensible - ERROR\n";
      return -1;
    }
  }

  return pos;
}

template <typename T>
int checkDefaults(T sitem)
{
  std::size_t n = sitem->numberOfValues();
  // If there is a default value is it correct?
  if (!(sitem->hasDefault() && (n > 0)))
  {
    return 0; // Nothing to check
  }
  std::cout << sitem->name() << "'s Default Value is " << sitem->defaultValue();
  if (!sitem->isUsingDefault())
  {
    std::cout << " - Not all values are set to default! - ERROR\n";
    for (std::size_t i = 0; i < n; i++)
    {
      std::cout << "\t" << i << ":" << sitem->value(i) << "\n";
    }
    return -1;
  }
  std::cout << " and all values are set to it - PASSED\n";
  return 0;
}

template <typename T>
int checkItem(T sitem, bool isExtensible)
{
  std::size_t minN = sitem->numberOfRequiredValues(), maxN = sitem->maxNumberOfValues();
  std::string name = sitem->name();
  int status = 0;
  std::cout << name << ": NumOfRequired Values: " << minN << " MaxNumberOfValues: " << maxN << "\n";
  if (checkDefaults(sitem))
  {
    std::cout << "Problem with default for " << sitem->name() << " - ERROR\n";
    status = -1;
  }
  if (!isExtensible)
  {
    if (!sitem->appendValue("New Val"))
    {
      std::cout << name << " did not allow append op for a fixed length item - PASSED\n";
    }
    else
    {
      std::cout << name
                << " allowed appending of new value for a fixed length item. RequiredSize:" << minN
                << " Current Size: " << sitem->numberOfValues() << " - ERROR\n";
      status = -1;
    }
    // Can we remove from the item?
    if (!sitem->removeValue(0))
    {
      std::cout << name << " did not allow remove op for a fixed length item - PASSED\n";
    }
    else
    {
      std::cout << name
                << " allowed removing of  value for a fixed length item. RequiredSize:" << minN
                << " Current Size: " << sitem->numberOfValues() << " - ERROR\n";
      status = -1;
    }
    return status;
  }

  // Is the number of initial values correct?
  if (minN != sitem->numberOfValues())
  {
    std::cerr << name << "'s initial size is not correct RequiredSize:" << minN
              << " Initial Size: " << sitem->numberOfValues() << " - ERROR\n";
    status = -1;
  }
  else
  {
    std::cout << name << " is have correct initial size - PASSED\n";
  }

  if (minN)
  {
    // Can we delete below the required number of values?
    if (sitem->removeValue(0))
    {
      std::cerr << name << " allowed deleting below min size. RequiredSize:" << minN
                << " Current Size: " << sitem->numberOfValues() << " - ERROR\n";
      status = -1;
    }
    else
    {
      std::cout << name << " - attempting to delete below required size test - PASSED\n";
    }
  }
  else
  {
    std::cout << name << " had no min size\n";
  }
  // Does resizing work?
  sitem->setNumberOfValues(minN + 1);
  if (sitem->numberOfValues() != (minN + 1))
  {
    std::cerr << name << " did not resize correctly. RequiredSize:" << minN
              << " Current Size: " << sitem->numberOfValues() << " - ERROR\n";
    status = -1;
  }
  else
  {
    std::cout << name << "'s resize test - PASSED\n";
  }

  if (checkDefaults<T>(sitem))
  {
    std::cout << "Problem with default for resized " << sitem->name() << "- ERROR\n";
    status = -1;
  }
  // Can we add to the item?
  if (sitem->appendValue("New Val"))
  {
    std::cout << name << "'s attempt of appending of new value. RequiredSize:" << minN
              << " Current Size: " << sitem->numberOfValues() << " - PASSED\n";
  }
  else
  {
    std::cout << name << "'s append test - ERROR\n";
    status = -1;
  }
  // Can we remove from the item?
  if (sitem->removeValue(0))
  {
    std::cout << name << " allowed removing of 0th val. RequiredSize:" << minN
              << " Current Size: " << sitem->numberOfValues() << "- PASSED\n";
  }
  else
  {
    std::cout << name << "'s remove test - ERROR\n";
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
                << " Current Size: " << sitem->numberOfValues() << " - FAILED\n";
      status = -1;
    }
    else
    {
      std::cout << name << "'s appending above max size test  MaxSize:" << maxN
                << " Current Size: " << sitem->numberOfValues() << " - PASSED\n";
    }
  }
  else
  {
    std::cout << name << " had no max size (IGNORED)\n";
  }
  return status;
}
int checkStringItem(const char* name, smtk::attribute::AttributePtr att, bool isExtensible)
{
  smtk::attribute::ItemPtr item = att->find(name);
  if (!item)
  {
    std::cerr << name << " could not be found\n";
    return -1;
  }
  auto sitem = smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(item);
  if (!sitem)
  {
    std::cerr << name << " is not a string\n";
    return -1;
  }
  return checkItem<>(sitem, isExtensible);
}

int checkFileItem(const char* name, smtk::attribute::AttributePtr att, bool isExtensible)
{
  smtk::attribute::ItemPtr item = att->find(name);
  if (!item)
  {
    std::cerr << name << " could not be found\n";
    return -1;
  }
  auto sitem = smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(item);
  if (!sitem)
  {
    std::cerr << name << " is not a file\n";
    return -1;
  }
  return checkItem<>(sitem, isExtensible);
}

int checkDirectoryItem(const char* name, smtk::attribute::AttributePtr att, bool isExtensible)
{
  smtk::attribute::ItemPtr item = att->find(name);
  if (!item)
  {
    std::cerr << name << " could not be found\n";
    return -1;
  }
  auto sitem = smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItem>(item);
  if (!sitem)
  {
    std::cerr << name << " is not a file\n";
    return -1;
  }
  return checkItem<>(sitem, isExtensible);
}

int checkSystem(smtk::attribute::SystemPtr system)
{
  int status = 0;

  std::cout << "Checking Extensible Value Items........\n";
  smtk::attribute::DefinitionPtr def = system->findDefinition("Derived2");
  if (!def)
  {
    std::cerr << "Could not find Derived 2 Def! - ERROR\n";
    return -2;
  }

  int i, n = static_cast<int>(def->numberOfItemDefinitions());
  if (n != 13)
  {
    std::cerr << "Derived 2 has incorrect number of items! Reported:" << n
              << " should have 13!- ERROR\n";
    return -2;
  }

  for (i = 0; i < n; i++)
  {
    std::cout << i << ":" << def->itemDefinition(i)->name();
    if (def->itemDefinition(i)->name() == d2items[i])
    {
      std::cout << "- PASSED!" << std::endl;
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
  int pos1 = checkStringItemDef("StringItem1", def, true);
  if (pos1 < 0)
  {
    std::cerr << "Problem with StringItem1 Def - ERROR\n";
    return -3;
  }

  int pos2 = checkStringItemDef("StringItem2", def, true);
  if (pos2 < 0)
  {
    std::cerr << "Problem with StringItem2 Def - ERROR\n";
    return -4;
  }

  int pos3 = checkStringItemDef("StringItem3", def, false);
  if (pos3 < 0)
  {
    std::cerr << "Problem with StringItem3 Def - ERROR\n";
    return -4;
  }

  pos1 = checkFileItemDef("FileItem1", def, true);
  if (pos1 < 0)
  {
    std::cerr << "Problem with FileItem1 Def - ERROR\n";
    return -3;
  }

  pos2 = checkFileItemDef("FileItem2", def, true);
  if (pos2 < 0)
  {
    std::cerr << "Problem with FileItem2 Def - ERROR\n";
    return -4;
  }

  pos3 = checkFileItemDef("FileItem3", def, false);
  if (pos3 < 0)
  {
    std::cerr << "Problem with FileItem3 Def - ERROR\n";
    return -4;
  }

  pos1 = checkDirectoryItemDef("DirectoryItem1", def, true);
  if (pos1 < 0)
  {
    std::cerr << "Problem with DirectoryItem1 Def - ERROR\n";
    return -3;
  }

  pos2 = checkDirectoryItemDef("DirectoryItem2", def, true);
  if (pos2 < 0)
  {
    std::cerr << "Problem with DirectoryItem2 Def - ERROR\n";
    return -4;
  }

  pos3 = checkDirectoryItemDef("DirectoryItem3", def, false);
  if (pos3 < 0)
  {
    std::cerr << "Problem with DirectoryItem3 Def - ERROR\n";
    return -4;
  }

  // Find or Create an attribute
  smtk::attribute::AttributePtr att = system->findAttribute("Derived2Att");
  if (!att)
  {
    att = system->createAttribute("Derived2Att", def);
    if (!att)
    {
      std::cerr << "Could not create Attribute - ERROR\n";
      return -5;
    }
    std::cout << "Created Derived2Att\n";
  }
  else
  {
    std::cout << "Found Derived2Att\n";
  }

  if (checkStringItem("StringItem1", att, true))
  {
    std::cerr << "Problem with StringItem1- ERROR\n";
    status = -6;
  }

  if (checkStringItem("StringItem2", att, true))
  {
    std::cerr << "Problem with StringItem2- ERROR\n";
    status = -7;
  }

  if (checkStringItem("StringItem3", att, false))
  {
    std::cerr << "Problem with StringItem3- ERROR\n";
    status = -8;
  }

  if (checkFileItem("FileItem1", att, true))
  {
    std::cerr << "Problem with FileItem1- ERROR\n";
    status = -6;
  }

  if (checkFileItem("FileItem2", att, true))
  {
    std::cerr << "Problem with FileItem2- ERROR\n";
    status = -7;
  }

  if (checkFileItem("FileItem3", att, false))
  {
    std::cerr << "Problem with FileItem3- ERROR\n";
    status = -8;
  }

  if (checkDirectoryItem("DirectoryItem1", att, true))
  {
    std::cerr << "Problem with DirectoryItem1- ERROR\n";
    status = -6;
  }

  if (checkDirectoryItem("DirectoryItem2", att, true))
  {
    std::cerr << "Problem with DirectoryItem2- ERROR\n";
    status = -7;
  }

  if (checkDirectoryItem("DirectoryItem3", att, false))
  {
    std::cerr << "Problem with DirectoryItem3- ERROR\n";
    status = -8;
  }

  std::cout << "Checking Extensible Group Items........\n";
  def = system->findDefinition("Derived3");
  if (!def)
  {
    std::cerr << "Could not find Derived 3 Def! - ERROR\n";
    return -2;
  }

  n = static_cast<int>(def->numberOfItemDefinitions());
  if (n != 7)
  {
    std::cerr << "Derived 3 has incorrect number of items! - ERROR\n";
    return -2;
  }

  for (i = 0; i < n; i++)
  {
    std::cout << i << ":" << def->itemDefinition(i)->name();
    if (def->itemDefinition(i)->name() == d3items[i])
    {
      std::cout << "- PASSED!" << std::endl;
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
  pos1 = checkGroupItemDef("GroupItem1", def, true);
  if (pos1 < 0)
  {
    std::cerr << "Problem with GroupItem1 Def - ERROR\n";
    return -3;
  }

  pos2 = checkGroupItemDef("GroupItem2", def, true);
  if (pos2 < 0)
  {
    std::cerr << "Problem with GroupItem2 Def - ERROR\n";
    return -4;
  }

  pos3 = checkGroupItemDef("GroupItem3", def, false);
  if (pos3 < 0)
  {
    std::cerr << "Problem with GroupItem3 Def - ERROR\n";
    return -4;
  }

  // Find or Create an attribute
  att = system->findAttribute("Derived3Att");
  if (!att)
  {
    att = system->createAttribute("Derived3Att", def);
    if (!att)
    {
      std::cerr << "Could not create Attribute - ERROR\n";
      return -5;
    }
    std::cout << "Created Derived3Att\n";
  }
  else
  {
    std::cout << "Found Derived3Att\n";
  }

  if (checkGroupItem("GroupItem1", att, true))
  {
    std::cerr << "Problem with GroupItem1- ERROR\n";
    status = -6;
  }

  if (checkGroupItem("GroupItem2", att, true))
  {
    std::cerr << "Problem with GroupItem2- ERROR\n";
    status = -7;
  }

  if (checkGroupItem("GroupItem3", att, false))
  {
    std::cerr << "Problem with GroupItem3- ERROR\n";
    status = -8;
  }

  return status;
}

int main(int argc, char* argv[])
{
  int status = 0;

  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " ExtensibleTestTemplate OutputFileName\n";
    return -1;
  }
  std::string outputFilename = argv[2];

  {
    smtk::attribute::SystemPtr system = smtk::attribute::System::create();
    std::cout << "System Created\n";
    smtk::io::AttributeReader reader;
    smtk::io::Logger logger;
    if (reader.read(system, argv[1], true, logger))
    {
      std::cerr << "Errors encountered reading Attribute File: " << argv[1] << "\n";
      std::cerr << logger.convertToString();
      return -1;
    }
    else
    {
      std::cout << "Read in template - PASSED\n";
    }

    // Write output file *before* checking system (checking changes system)
    smtk::io::AttributeWriter writer;
    smtk::io::Logger logger1;
    if (writer.write(system, outputFilename, logger1))
    {
      std::cerr << "Errors encountered creating Attribute File:\n";
      std::cerr << logger1.convertToString();
      status = -1;
    }
    else
    {
      std::cout << "Wrote " << outputFilename << std::endl;
    }

    // Check system
    status = checkSystem(system);
    if (status < 0)
    {
      return status;
    }

    std::cout << "System destroyed\n";
  }

  //Use separate scope to read attribute system back in
  {
    smtk::attribute::SystemPtr readbackSystem = smtk::attribute::System::create();
    std::cout << "Readback System Created\n";
    smtk::io::AttributeReader reader2;
    smtk::io::Logger logger2;
    if (reader2.read(readbackSystem, outputFilename, true, logger2))
    {
      std::cerr << "Errors encountered reading Attribute File: " << outputFilename << "\n";
      std::cerr << logger2.convertToString();
      return -1;
    }

    status = checkSystem(readbackSystem);
  }
  if (status == 0)
  {
    std::cout << "All Test PASSED!\n";
    cleanup(outputFilename);
  }
  else
  {
    std::cerr << "FAILURES Detected!  - Status = " << status << "\n";
  }
  return status;
}
