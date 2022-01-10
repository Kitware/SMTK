//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_xml_xmlConfiguration_h
#define smtk_view_xml_xmlConfiguration_h

#include "smtk/view/Configuration.h"

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace view
{

void SMTKCORE_EXPORT
from_xml(const pugi::xml_node& node, std::shared_ptr<smtk::view::Configuration>& comp);

void SMTKCORE_EXPORT
from_xml(const pugi::xml_node& node, smtk::view::Configuration::Component& comp, bool isTopComp);

} // namespace view
} // namespace smtk
#endif // smtk_view_xml_xmlConfiguration_h
