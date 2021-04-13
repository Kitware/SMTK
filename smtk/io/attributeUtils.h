//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_attributeUtils_h
#define __smtk_io_attributeUtils_h

#include "smtk/CoreExports.h"

#include <string>

///\file This file contains a set of functions that provide additional I/O capabilities for
/// smtk::attribute related entities.
namespace smtk
{
namespace attribute
{
class GroupItem;
}

namespace io
{
class Logger;
///\brief Populates a GroupItem from a CSV file.
///
/// This function can be used on an extensible GroupItem that only contains non-option,
/// non-extensible ValueItems.  Any active children associated with these items are ignored.
///
/// The function will either overwrite or append groups to the item depending on the appendToGroup
/// parameter.  Any line that starts with the comment character will be skipped but no logged.
/// Any line in the CSV file that does not contain the proper number of values (based
/// on the item's definition's children) will be  logged and skipped.  Empty values will result in
/// in the corresponding value being unset.  Any value  in the CSV file that can not be converted
/// into the appropriate data type or is invalid based on the corresponding item's definition will
/// be ignored and logged.
/// See groupItemCVSTest.cxx for an example of using the function.
SMTKCORE_EXPORT bool importFromCSV(
  smtk::attribute::GroupItem& item,
  const std::string& filename,
  Logger& logger,
  bool appendToGroup = false,
  const std::string& sep = ",",
  const std::string& comment = "");

///\brief Populates a GroupItem from a file of Doubles.
///
/// This function can be used on an extensible GroupItem that only contains non-option,
/// non-extensible DoubleItems.  Any active children associated with these items are ignored.
/// The file is assume to contain doubles that are separated by either whitespace and/or
/// by an optional separator character.  If an invalid character is found, the line will be skipped
/// and logged.  Any line that starts with the comment character will be skipped but no logged.
/// Any line in the file that contain less than the proper number of values (based
/// on the item's definition's children) will be  logged and skipped.
///
/// The function will either overwrite or append groups to the item depending on the appendToGroup
/// parameter.
/// See groupItemDoubleFileTest.cxx for an example of using the function.
SMTKCORE_EXPORT bool importFromDoubleFile(
  smtk::attribute::GroupItem& item,
  const std::string& filename,
  Logger& logger,
  bool appendToGroup = false,
  const std::string& optionalSep = ",",
  const std::string& comment = "");
} // namespace io
} // namespace smtk
#endif
