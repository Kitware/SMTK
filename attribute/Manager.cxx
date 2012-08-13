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
using namespace slctk::attribute; 

//----------------------------------------------------------------------------
Manager::Manager(): m_nextAttributeId(0), m_nextDefinitionId(0)
{
}

//----------------------------------------------------------------------------
Manager::~Manager()
{
  // Deleting the clusters delete the attributes and their definitions
  std::map<std::string,  Cluster*>::iterator it;
  for (it = this->m_clusters.begin(); it != this->m_clusters.end(); it++)
    {
    delete (*it).second;
    }
}
//----------------------------------------------------------------------------
slctk::AttributeDefinitionPtr 
Manager::createDefinition(const std::string &typeName,
                          const std::string &baseTypeName)
{
  slctk::AttributeDefinitionPtr def = 
    this->createDefinition(typeName, baseTypeName, this->m_nextDefinitionId);
  if (def != NULL)
    {
    this->m_nextDefinitionId++;
    }
  return def;
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
slctk::AttributeDefinitionPtr
Manager::createDefinition(const std::string &typeName, 
                          const std::string &baseTypeName,
                          unsigned long id)
{
  Cluster *newCluster, *c = this->findCluster(typeName);
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
  newCluster = new Cluster(this, c);
  this->m_clusters[typeName] = newCluster;
  return newCluster->generateDefinition(typeName, id);
}
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
  Cluster *c = this->findCluster(typeName);
  if (c == NULL)
    {
    return slctk::AttributePtr();
    }
  a = c->generateAttribute(name, id);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[id] = a;
  return a;
}
//----------------------------------------------------------------------------
bool Manager::deleteAttribute(slctk::AttributePtr att)
{
  // Make sure that this manager is managing this attribute
  if (att->manager() != this)
    {
    return false;
    }
  Cluster *c = att->cluster();
  this->m_attributes.erase(att->name());
  this->m_attributeIdMap.erase(att->id());
  c->deleteAttribute(att);
  return true;
}
//----------------------------------------------------------------------------
void Manager::findClusters(long mask, std::vector<Cluster *> &result) const
{
  slctk::AttributeDefinitionPtr def;
  result.clear();
  std::map<std::string,  Cluster*>::const_iterator it;
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
