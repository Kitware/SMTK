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

#include "smtk/mesh/core/Resource.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityTypeBits.h"

#include "smtk/resource/PersistentObject.h"

#include "smtk/view/icons/attributeResource_cpp.h"
#include "smtk/view/icons/attribute_cpp.h"
#include "smtk/view/icons/edge_cpp.h"
#include "smtk/view/icons/face_cpp.h"
#include "smtk/view/icons/loop_cpp.h"
#include "smtk/view/icons/meshResource_cpp.h"
#include "smtk/view/icons/mesh_cpp.h"
#include "smtk/view/icons/modelResource_cpp.h"
#include "smtk/view/icons/model_cpp.h"
#include "smtk/view/icons/resource_cpp.h"
#include "smtk/view/icons/vertex_cpp.h"
#include "smtk/view/icons/volume_cpp.h"

#include "smtk/Regex.h"
#include <vector>

#include <fstream>

namespace
{
// PersistentObjects' colors are held as a std::vector<double> property using
// the following keyword.
const std::string colorProperty = "color";

// If there is no color property on the object, default to coloring it gray.
const std::string defaultFillColor = "#808080";
} // namespace

namespace smtk
{
namespace view
{
std::string SVGIconConstructor::operator()(
  const smtk::resource::PersistentObject& object,
  const std::string& secondaryColor) const
{
  std::string fill = defaultFillColor;

  if (object.properties().contains<std::vector<double>>(colorProperty))
  {
    const auto& vec = object.properties().at<std::vector<double>>(colorProperty);
    if (vec.size() >= 3)
    {
      fill = smtk::common::Color::floatRGBToString(vec.data());
    }
  }

  std::string svg = smtk::regex_replace(
    smtk::regex_replace(this->svg(object), smtk::regex(m_defaultColor), fill),
    smtk::regex(m_secondaryColor),
    secondaryColor);

  return svg;
}

std::string DefaultIconConstructor::operator()(
  const smtk::resource::PersistentObject&,
  const std::string&) const
{
  return "";
}

std::string ResourceIconConstructor::svg(const smtk::resource::PersistentObject&) const
{
  return resource_svg();
}

std::string AttributeIconConstructor::svg(const smtk::resource::PersistentObject& object) const
{
  if (dynamic_cast<const smtk::attribute::Resource*>(&object) != nullptr)
  {
    return attributeResource_svg();
  }
  else
  {
    return attribute_svg();
  }
}

std::string MeshIconConstructor::svg(const smtk::resource::PersistentObject& object) const
{
  if (const auto* resource = dynamic_cast<const smtk::mesh::Resource*>(&object))
  {
    (void)resource;
    return meshResource_svg();
  }
  else
  {
    return mesh_svg();
  }
}

std::string ModelIconConstructor::svg(const smtk::resource::PersistentObject& object) const
{
  if (const auto* resource = dynamic_cast<const smtk::model::Resource*>(&object))
  {
    (void)resource;
    return modelResource_svg();
  }
  else if (const auto* entity = dynamic_cast<const smtk::model::Entity*>(&object))
  {
    smtk::model::BitFlags flags = entity->entityFlags();

    switch (flags & smtk::model::ENTITY_MASK)
    {
      case smtk::model::CELL_ENTITY:
        switch (flags & smtk::model::ANY_ENTITY)
        {
          case smtk::model::VERTEX:
            return vertex_svg();
          case smtk::model::EDGE:
            return edge_svg();
          case smtk::model::SHELL_1D:
            return loop_svg();
          case smtk::model::FACE:
            return face_svg();
          case smtk::model::VOLUME:
            return volume_svg();
          default:
            return "";
            break;
        }
        break;
      case smtk::model::USE_ENTITY:
      case smtk::model::SHELL_ENTITY:
      case smtk::model::GROUP_ENTITY:
      case smtk::model::MODEL_ENTITY:
        return model_svg();
      case smtk::model::INSTANCE_ENTITY:
      case smtk::model::AUX_GEOM_ENTITY:
      case smtk::model::SESSION:
      default:
        return "";
    }
  }
  else
  {
    return model_svg();
  }
}
} // namespace view
} // namespace smtk
