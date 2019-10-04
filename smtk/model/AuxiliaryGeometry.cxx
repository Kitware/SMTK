//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/AuxiliaryGeometry.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Resource.h"

namespace smtk
{
namespace model
{

bool AuxiliaryGeometry::reparent(const Model& m)
{
  if (!m.isValid() || this->owningModel() != m)
  {
    return false;
  }
  EntityRef immediateParent = this->embeddedIn();
  if (immediateParent == m)
  {
    return false;
  }

  immediateParent.unembedEntity(*this);
  m.as<Model>().addAuxiliaryGeometry(*this);
  return true;
}

bool AuxiliaryGeometry::reparent(const AuxiliaryGeometry& a)
{
  if (!a.isValid() || this->owningModel() != a.owningModel())
  {
    return false;
  }
  EntityRef immediateParent = this->embeddedIn();
  if (immediateParent == a)
  {
    return false;
  }

  immediateParent.unembedEntity(*this);
  a.as<AuxiliaryGeometry>().embedEntity(*this);
  return true;
}

/**\brief Indicate whether the entity has a URL to external geometry.
  *
  */
bool AuxiliaryGeometry::hasURL() const
{
  if (!this->hasStringProperties())
  {
    return false;
  }

  auto comp = this->component();
  if (comp == nullptr)
  {
    return false;
  }

  return comp->properties().has<std::vector<std::string> >("url");
}

/**\brief Return the URL to external geometry if any exists, and an empty string otherwise.
  *
  * Note that only the first string (in an array of strings that may be held) is returned.
  */
std::string AuxiliaryGeometry::url() const
{
  if (!this->hasStringProperties())
  {
    return std::string();
  }
  auto comp = this->component();
  if (comp == nullptr || !comp->properties().has<std::vector<std::string> >("url") ||
    comp->properties().at<std::vector<std::string> >("url").empty())
  {
    return std::string();
  }
  return comp->properties().at<std::vector<std::string> >("url").at(0);
}

/**\brief Set the URL of external geometry referenced by this auxiliary geometry.
  *
  */
void AuxiliaryGeometry::setURL(const std::string& url)
{
  this->setStringProperty("url", url);
}

bool AuxiliaryGeometry::isModified() const
{
  smtk::model::Resource::Ptr resource = this->resource();
  if (!resource || !this->hasIntegerProperty("clean"))
  {
    return false;
  }

  const IntegerList& prop(this->integerProperty("clean"));
  return (!prop.empty() && (prop[0] != 1));
}

void AuxiliaryGeometry::setIsModified(bool isModified)
{
  smtk::model::Resource::Ptr resource = this->resource();
  if (!resource)
  {
    return;
  }

  this->setIntegerProperty("clean", isModified ? 0 : 1);
}

AuxiliaryGeometries AuxiliaryGeometry::auxiliaryGeometries() const
{
  return this->embeddedEntities<AuxiliaryGeometries>();
}

} // namespace model
} // namespace smtk
