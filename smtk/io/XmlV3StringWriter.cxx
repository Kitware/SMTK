//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/XmlV3StringWriter.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/io/Logger.h"

using namespace pugi;
using namespace smtk;
using namespace smtk::attribute;

namespace smtk
{
namespace io
{

XmlV3StringWriter::XmlV3StringWriter(const attribute::SystemPtr mySystem)
  : XmlV2StringWriter(mySystem)
{
}

XmlV3StringWriter::~XmlV3StringWriter()
{
}

std::string XmlV3StringWriter::className() const
{
  return std::string("XmlV3StringWriter");
}

unsigned int XmlV3StringWriter::fileVersion() const
{
  return 3;
}

void XmlV3StringWriter::processItemDefinitionType(
  xml_node& node, smtk::attribute::ItemDefinitionPtr idef)
{
  switch (idef->type())
  {
    case Item::DATE_TIME:
      this->processDateTimeDef(node, smtk::dynamic_pointer_cast<DateTimeItemDefinition>(idef));
      break;

    default:
      XmlV2StringWriter::processItemDefinitionType(node, idef);
      break;
  }
}

void XmlV3StringWriter::processDateTimeDef(
  pugi::xml_node& node, smtk::attribute::DateTimeItemDefinitionPtr idef)
{
  node.append_attribute("NumberOfRequiredValues") =
    static_cast<unsigned int>(idef->numberOfRequiredValues());

  std::string format = idef->displayFormat();
  if (!format.empty())
  {
    node.append_attribute("DisplayFormat").set_value(format.c_str());
  }
  node.append_attribute("ShowTimeZone") = idef->useTimeZone();
  node.append_attribute("ShowCalendarPopup") = idef->useCalendarPopup();

  if (idef->hasDefault())
  {
    xml_node defnode = node.append_child("DefaultValue");
    ::smtk::common::DateTimeZonePair dtz = idef->defaultValue();
    defnode.text().set(dtz.serialize().c_str());
  }

  // if (idef->hasRange())
  //   {
  //   xml_node rnode = node.append_child("RangeInfo");
  //   xml_node r;
  //   bool inclusive;
  //   if (idef->hasMinRange())
  //     {
  //     r = rnode.append_child("Min");
  //     inclusive = idef->minRangeInclusive();
  //     r.append_attribute("Inclusive").set_value(inclusive);
  //     r.text().set(getValueForXMLElement(idef->minRange()));
  //     }
  //   if (idef->hasMaxRange())
  //     {
  //     r = rnode.append_child("Max");
  //     inclusive = idef->maxRangeInclusive();
  //     r.append_attribute("Inclusive").set_value(inclusive);
  //     r.text().set(getValueForXMLElement(idef->maxRange()));
  //     }
  //   }
}

void XmlV3StringWriter::processItemType(xml_node& node, smtk::attribute::ItemPtr item)
{
  switch (item->type())
  {
    case Item::DATE_TIME:
      this->processDateTimeItem(node, smtk::dynamic_pointer_cast<DateTimeItem>(item));
      break;

    default:
      XmlV2StringWriter::processItemType(node, item);
      break;
  }
}

void XmlV3StringWriter::processDateTimeItem(pugi::xml_node& node, attribute::DateTimeItemPtr item)
{
  size_t numValues = item->numberOfValues();
  xml_node val;
  if (numValues == 0)
  {
    return;
  }

  // (else)
  if (numValues == 1)
  {
    if (item->isSet())
    {
      ::smtk::common::DateTimeZonePair dtz = item->value();
      node.text().set(dtz.serialize().c_str());
    }
    else
    {
      node.append_child("UnsetVal");
    }
    return;
  }

  // (else)
  xml_node values = node.append_child("Values");
  for (std::size_t i = 0; i < numValues; ++i)
  {
    if (item->isSet(i))
    {
      val = values.append_child("Val");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
      ::smtk::common::DateTimeZonePair dtz = item->value();
      val.text().set(dtz.serialize().c_str());
    }
    else
    {
      val = values.append_child("UnsetVal");
      val.append_attribute("Ith").set_value(static_cast<unsigned int>(i));
    }
  }
}

} // namespace io
} // namespace smtk
