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


#include "attribute/Manager.h"
#include "attribute/Attribute.h"
#include "attribute/Definition.h"
#include "attribute/Cluster.h"

#include <sstream>

using namespace slctk::attribute; 

//----------------------------------------------------------------------------
Manager::Manager(): m_nextAttributeId(0)
{
}

//----------------------------------------------------------------------------
Manager::~Manager()
{
}

//----------------------------------------------------------------------------
slctk::AttributeDefinitionPtr
Manager::createDefinition(const std::string &typeName, 
                          const std::string &baseTypeName)
{
  slctk::AttributeClusterPtr newCluster, c = this->findCluster(typeName);
  // Does this cluster already exist
  if (c != NULL)
    {
    return slctk::AttributeDefinitionPtr();
    }

  // If baseTypeName is not empty then it better exist
  if (baseTypeName != "")
    {
    c = this->findCluster(baseTypeName);
    if (c == NULL)
      {
      return slctk::AttributeDefinitionPtr();
      }
    }
  newCluster = slctk::AttributeClusterPtr(new Cluster(this));
  if (c != NULL)
    {
    c->addChild(newCluster);
    newCluster->setParent(c);
    }
  this->m_clusters[typeName] = newCluster;
  slctk::AttributeDefinitionPtr def(new Definition(typeName, newCluster));
  newCluster->setDefinition(def);
  return def;
}

//----------------------------------------------------------------------------
slctk::AttributePtr Manager::createAttribute(const std::string &name,
                                             slctk::AttributeDefinitionPtr def)
{
  // Make sure the definition belongs to this manager!
  if (def->manager() != this)
    {
    return slctk::AttributePtr();
    }
  // Next we need to check to see if an attribute exists by the same name
  slctk::AttributePtr a = this->findAttribute(name);
  if (a != NULL)
    {
    return slctk::AttributePtr();
    }

  slctk::AttributeClusterPtr c = def->cluster();
  a = slctk::AttributePtr(new Attribute(name, c, this->m_nextAttributeId++));
  c->definition()->buildAttribute(a);
  c->addAttribute(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[a->id()] = a;
  return a;
}

//----------------------------------------------------------------------------
slctk::AttributePtr Manager::createAttribute(const std::string &typeName)
{
  slctk::AttributePtr att =
    this->createAttribute(this->createUniqueName(typeName), typeName,
                          this->m_nextAttributeId);
  if (att != NULL)
    {
    this->m_nextAttributeId++;
    }
  return att;
}

//----------------------------------------------------------------------------
slctk::AttributePtr Manager::createAttribute(const std::string &name,
                                             const std::string &typeName)
{
  slctk::AttributePtr att = this->createAttribute(name, typeName,
                                                  this->m_nextAttributeId);
  if (att != NULL)
    {
    this->m_nextAttributeId++;
    }
  return att;
}

//----------------------------------------------------------------------------
// For Reader classes
//----------------------------------------------------------------------------
slctk::AttributePtr Manager::createAttribute(const std::string &name,
                                             const std::string &typeName,
                                             unsigned long id)
{
  // First we need to check to see if an attribute exists by the same name
  slctk::AttributePtr a = this->findAttribute(name);
  if (a != NULL)
    {
    return slctk::AttributePtr();
    }

  // Second we need to find the cluster that corresponds to the type
  slctk::AttributeClusterPtr c = this->findCluster(typeName);
  if (c == NULL)
    {
    return slctk::AttributePtr();
    }
  a = slctk::AttributePtr(new Attribute(name, c, id));
  c->definition()->buildAttribute(a);
  c->addAttribute(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[id] = a;
  return a;
}
//----------------------------------------------------------------------------
bool Manager::removeAttribute(slctk::AttributePtr att)
{
  // Make sure that this manager is managing this attribute
  if (att->manager() != this)
    {
    return false;
    }
  slctk::AttributeClusterPtr c = att->cluster();
  this->m_attributes.erase(att->name());
  this->m_attributeIdMap.erase(att->id());
  c->removeAttribute(att);
  return true;
}
//----------------------------------------------------------------------------
void Manager::findClusters(long mask, std::vector<slctk::AttributeClusterPtr> &result) const
{
  slctk::AttributeDefinitionPtr def;
  result.clear();
  std::map<std::string,  slctk::AttributeClusterPtr>::const_iterator it;
  for (it = this->m_clusters.begin(); it != this->m_clusters.end(); it++)
    {
    def =  (*it).second->definition();
    if (def)
      {
      if ((def->associationMask() & mask) == mask)
        {
        result.push_back((*it).second);
        }
      }
    }
}
//----------------------------------------------------------------------------
bool Manager::rename(slctk::AttributePtr att, const std::string &newName)
{
  // Make sure that this manager is managing this attribute
  if (att->manager() != this)
    {
    return false;
    }
  // Make sure that the new name doesn't exists
  slctk::AttributePtr a = this->findAttribute(newName);
  if (a != NULL)
    {
    return false;
    }
  this->m_attributes.erase(att->name());
  att->cluster()->rename(att, newName);
  this->m_attributes[newName] = att;
  return true;
}
//----------------------------------------------------------------------------
std::string Manager::createUniqueName(const std::string &type) const
{
  int i = 0;
  std::string base = type, newName;
  base.append("-");
  while (1)
    {
    std::ostringstream n;
    n << i++;
    newName = base + n.str();
    // Make sure that the new name doesn't exists
    slctk::AttributePtr a = this->findAttribute(newName);
    if (a == NULL)
      {
      return newName;
      }
    }
  return "";
}
//----------------------------------------------------------------------------
