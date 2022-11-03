//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/AssociationBadge.h"

#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Links.h"

#include "smtk/io/Logger.h"

#include "smtk/common/Color.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace view
{

AssociationBadge::AssociationBadge() = default;

AssociationBadge::AssociationBadge(BadgeSet& parent, const Configuration::Component& config)
  : m_parent(&parent)
{
  // Extract a list of definitions that must be associated to objects.
  // NB: We should condition these on the object's type as well.
  std::size_t numChildren = config.numberOfChildren();
  for (std::size_t ii = 0; ii < numChildren; ++ii)
  {
    const auto& child = config.child(ii);
    if (child.name() == "Requires")
    {
      std::string def;
      if (child.attribute("Definition", def))
      {
        m_requiredDefinitions.insert(def);
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Badge Requires elements must have a Definition attribute.");
      }
    }
    else if (child.name() == "AppliesTo")
    {
      std::string val;
      if (child.attribute("Resource", val))
      {
        m_applyToResource = val;
      }
      else
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Badge AppliesTo elements must have a Resource attribute.");
      }
      if (child.attribute("Component", val))
      {
        // If unspecified, we assume the badge applies to resources.
        // Use "*" or "any" to apply to all components of a resource.
        m_applyToComponent = val;
      }
    }
  }
  if (m_requiredDefinitions.empty())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "AssociationBadge must be configured with a list of required Definitions.");
  }
  // NB: Could add option to show some other icon when associations are valid.
}

AssociationBadge::~AssociationBadge()
{
  m_parent = nullptr;
}

bool AssociationBadge::appliesToPhrase(const DescriptivePhrase* phrase) const
{
  if (!m_parent || !phrase)
  { // NB: The check on m_parent prevents color with no icon in some cases.
    return false;
  }
  auto obj = phrase->relatedObject();
  if (!this->appliesToObject(obj))
  {
    return false;
  }
  return !this->unmetRequirements(obj).empty();
}

std::string AssociationBadge::tooltip(const DescriptivePhrase* phrase) const
{
  std::string result;
  if (!m_parent || !phrase)
  { // NB: The check on m_parent prevents color with no icon in some cases.
    return result;
  }
  auto obj = phrase->relatedObject();
  if (!this->appliesToObject(obj))
  {
    return result;
  }
  result = "Needs association to";
  auto defs = this->unmetRequirements(obj);
  for (const auto& def : defs)
  {
    result += " " + def;
  }
  return result;
}

std::string AssociationBadge::icon(const DescriptivePhrase*, const std::array<float, 4>& background)
  const
{
  std::string icon = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<svg data-name=\"Layer 1\" version=\"1.1\" viewBox=\"0 0 64 64\" "
                     "xmlns=\"http://www.w3.org/2000/svg\"><title>Exclamation</title>"
                     "<path d=\"m31.57 2.998c-15.94 0-28.863 12.921-28.863 28.861 7e-7 15.94 "
                     "12.923 28.863 28.863 28.863 15.94-1e-6 28.861-12.923 28.861-28.863 0"
                     "-15.94-12.921-28.861-28.861-28.861zm-0.013671 8.6816c1.4401 0 2.6888 "
                     "0.5013 3.7461 1.5039 1.0573 0.98438 1.5859 2.1784 1.5859 3.582 0 0.7474"
                     "-0.13672 1.7409-0.41016 2.9805-0.25521 1.2214-0.58333 2.6615-0.98438 "
                     "4.3203-0.38281 1.5677-0.82031 3.5273-1.3125 5.8789-0.49219 2.3333-1.0117 "
                     "5.2227-1.5586 8.668h-2.1055c-0.54688-3.4818-1.0664-6.3802-1.5586-8.6953s"
                     "-0.92969-4.2565-1.3125-5.8242c-0.34635-1.4401-0.66536-2.8438-0.95703"
                     "-4.2109-0.29167-1.3854-0.4375-2.4245-0.4375-3.1172 0-1.4036 0.51953-2.5977 "
                     "1.5586-3.582 1.0391-1.0026 2.2878-1.5039 3.7461-1.5039zm0.027343 30.188c"
                     "1.4036 0 2.625 0.51042 3.6641 1.5312 1.0573 1.0026 1.5859 2.1966 1.5859 "
                     "3.582 0 1.3854-0.52864 2.5794-1.5859 3.582-1.0391 0.98438-2.2604 1.4766"
                     "-3.6641 1.4766-1.4219 0-2.6615-0.49219-3.7188-1.4766-1.0391-1.0026-1.5586"
                     "-2.1966-1.5586-3.582 0-1.3854 0.51953-2.5794 1.5586-3.582 1.0573-1.0208 "
                     "2.2969-1.5313 3.7188-1.5312z\" color=\"#000000\" color-rendering=\"auto\" "
                     "dominant-baseline=\"auto\" fill=\"#ff2a2a\" image-rendering=\"auto\" "
                     "shape-rendering=\"auto\" solid-color=\"#000000\" stop-color=\"#000000\" "
                     "style=\"font-feature-settings:normal;font-variant-alternates:normal;"
                     "font-variant-caps:normal;font-variant-east-asian:normal;"
                     "font-variant-ligatures:normal;font-variant-numeric:normal;"
                     "font-variant-position:normal;font-variation-settings:normal;inline-size:0;"
                     "isolation:auto;mix-blend-mode:normal;shape-margin:0;shape-padding:0;"
                     "text-decoration-color:#000000;text-decoration-line:none;"
                     "text-decoration-style:solid;text-indent:0;text-orientation:mixed;"
                     "text-transform:none;white-space:normal\"/>\" "
                     "d=\"M59.94,31.86S47.49,14.37,31.31,14.37c-16.81,0-28.1,17.49-28.1,17.49S"
                     "14.5,49.35, 31.31,49.35C47.49,49.35,59.94,31.86,59.94,31.86Z\" "
                     "style=\"fill:#fff\"/&gt;</svg>";

  if (smtk::common::Color::floatRGBToLightness(background.data()) < 0.5)
  {
    icon = smtk::regex_replace(icon, smtk::regex("ff2a2a"), "ffaaaa");
  }
  return icon;
}

bool AssociationBadge::appliesToObject(const smtk::resource::PersistentObjectPtr& obj) const
{
  if (!obj)
  {
    return false;
  }
  if (m_applyToResource.empty())
  {
    return true;
  }
  auto* rsrc = dynamic_cast<smtk::resource::Resource*>(obj.get());
  if (rsrc)
  {
    if (m_applyToComponent.empty())
    {
      return rsrc->isOfType(m_applyToResource);
    }
    return false;
  }
  auto* comp = dynamic_cast<smtk::resource::Component*>(obj.get());
  if (comp)
  {
    rsrc = comp->resource().get();
    if (rsrc->isOfType(m_applyToResource))
    {
      auto queryOp = rsrc->queryOperation(m_applyToComponent);
      return queryOp(*comp);
    }
  }
  return false;
}

std::set<std::string> AssociationBadge::unmetRequirements(
  const smtk::resource::PersistentObjectPtr& obj) const
{
  std::set<std::string> defs = m_requiredDefinitions;
  // This call (linkedFrom) is slow. If we can configure ourselves to key on a
  // specific attribute resource, then we could use it to call the linkedFrom()
  // variant that accepts a resource pointer as well as a role (which is fast).
  auto poset = obj->links().linkedFrom(smtk::attribute::Resource::AssociationRole);

  // See if the set covers all the definitions needed
  // Subtract requirements as they are met.
  for (const auto& candidate : poset)
  {
    auto* att = dynamic_cast<smtk::attribute::Attribute*>(candidate.get());
    if (!att)
    {
      continue;
    }
    auto def = att->definition();
    while (def)
    {
      std::string name = def->type();
      for (std::set<std::string>::iterator it = defs.begin(); it != defs.end(); ++it)
      {
        if (name == *it)
        {
          defs.erase(name);
          break;
        }
      }
      def = def->baseDefinition();
    }
    if (defs.empty())
    {
      return defs; // All requirements met. Terminate early.
    }
  }
  return defs; // Some requirements remain.
}
} // namespace view
} // namespace smtk
