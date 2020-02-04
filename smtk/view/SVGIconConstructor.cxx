//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/SVGIconConstructor.h"

#include "smtk/attribute/Resource.h"

#include "smtk/common/Color.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityTypeBits.h"

#include "smtk/resource/PersistentObject.h"

#include "smtk/view/icons/attributeResource_svg.h"
#include "smtk/view/icons/attribute_svg.h"
#include "smtk/view/icons/edge_svg.h"
#include "smtk/view/icons/face_svg.h"
#include "smtk/view/icons/mesh_svg.h"
#include "smtk/view/icons/model_svg.h"
#include "smtk/view/icons/vertex_svg.h"
#include "smtk/view/icons/volume_svg.h"

#include <regex>
#include <vector>

namespace
{
// PersistentObjects' colors are held as a std::vector<double> property using
// the following keyword.
static const std::string colorProperty = "color";

// If there is no color property on the object, default to coloring it gray.
static const std::string defaultFillColor = "#808080";
}

namespace smtk
{
namespace view
{
std::string SVGIconConstructor::operator()(
  const smtk::resource::PersistentObject& object, const std::string& secondaryColor) const
{
  std::string fill = defaultFillColor;

  if (object.properties().contains<std::vector<double> >(colorProperty))
  {
    fill = smtk::common::Color::floatRGBToString(
      &object.properties().at<std::vector<double> >(colorProperty)[0]);
  }

  std::string svg =
    std::regex_replace(std::regex_replace(this->svg(object), std::regex(m_defaultColor), fill),
      std::regex(m_secondaryColor), secondaryColor);

  return svg;
}

std::string DefaultIconConstructor::operator()(
  const smtk::resource::PersistentObject&, const std::string& secondaryColor) const
{
  return "";
}

std::string AttributeIconConstructor::svg(const smtk::resource::PersistentObject& object) const
{
  if (dynamic_cast<const smtk::attribute::Resource*>(&object) != nullptr)
  {
    return attributeResource_svg;
  }
  else
  {
    return attribute_svg;
  }
}

std::string MeshIconConstructor::svg(const smtk::resource::PersistentObject&) const
{
  return mesh_svg;
}

std::string ModelIconConstructor::svg(const smtk::resource::PersistentObject& object) const
{
  if (auto entity = dynamic_cast<const smtk::model::Entity*>(&object))
  {
    smtk::model::BitFlags flags = entity->entityFlags();

    bool dimBits = true;
    switch (flags & smtk::model::ENTITY_MASK)
    {
      case smtk::model::CELL_ENTITY:
        switch (flags & smtk::model::ANY_ENTITY)
        {
          case smtk::model::VERTEX:
            return vertex_svg;
          case smtk::model::EDGE:
            return edge_svg;
          case smtk::model::FACE:
            return face_svg;
          case smtk::model::VOLUME:
            return volume_svg;
          default:
            break;
        }
      case smtk::model::USE_ENTITY:
      case smtk::model::SHELL_ENTITY:
      case smtk::model::GROUP_ENTITY:
      case smtk::model::MODEL_ENTITY:
        return model_svg;
      case smtk::model::INSTANCE_ENTITY:
      case smtk::model::AUX_GEOM_ENTITY:
      case smtk::model::SESSION:
      default:
        return "";
    }
  }
  else
  {
    return model_svg;
  }
}
}
}
