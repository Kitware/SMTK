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


#include "attribute/Attribute.h"
#include "attribute/AttributeReferenceComponent.h"
#include "attribute/Cluster.h"
#include "attribute/Component.h"
#include "attribute/Definition.h"
using namespace slctk::attribute; 
//----------------------------------------------------------------------------
Attribute::Attribute(const std::string &myName, Cluster *myCluster, 
                     unsigned long myId):
  m_name(myName), m_cluster(myCluster), m_id(myId)
{
}

//----------------------------------------------------------------------------
Attribute::~Attribute()
{
  this->removeAllAssociations();
  this->removeAllComponents();
  // Tell all references this attribute is going away.  Since setting the reference
  // to NULL will cause the references set to be modified we need to copy the set first
  // and iterate over the copy
  std::set<AttributeReferenceComponent *> comps = this->m_references; 
  std::set<AttributeReferenceComponent *>::iterator it;
  for (it = comps.begin(); it != comps.end(); it++)
    {
    (*it)->setValue(NULL);
    }
}
//----------------------------------------------------------------------------
void Attribute::removeAllComponents()
{
  std::size_t i, n = this->m_components.size();
  for (i = 0; i < n; i++)
    {
    delete this->m_components[i];
    }

}
//----------------------------------------------------------------------------
const std::string &Attribute::type() const
{
  return this->m_cluster->type();
}
//----------------------------------------------------------------------------
std::vector<std::string> Attribute::types() const
{
  std::vector<std::string> tvec;
  Cluster *c = this->m_cluster;
  while (c != NULL)
    {
    tvec.push_back(c->type());
    c = c->parent();
    }
  return tvec;
}
//----------------------------------------------------------------------------
bool Attribute::isA(Definition *def) const
{
  return this->m_cluster->definition()->isA(def);
}
//----------------------------------------------------------------------------
const Definition *Attribute::definition() const
{
  return this->m_cluster->definition();
}
//----------------------------------------------------------------------------
bool Attribute::isMemberOf(const std::string &catagory) const
{
  return this->m_cluster->definition()->isMemberOf(catagory);
}
//----------------------------------------------------------------------------
bool Attribute::isMemberOf(const std::vector<std::string> &catagories) const
{
  return this->m_cluster->definition()->isMemberOf(catagories);
}
//----------------------------------------------------------------------------
Manager *Attribute::manager() const
{
  return this->m_cluster->manager();
}
//----------------------------------------------------------------------------
void Attribute::associateEntity(slctk::ModelEntity *entity)
{
  if (this->isEntityAssociated(entity))
    {
    // Nothing to be done
    return;
    }
  this->m_entities.insert(entity);
  //TODO Need to attach attribute to the entity!
}
//----------------------------------------------------------------------------
void Attribute::disassociateEntity(slctk::ModelEntity *entity)
{
  if (this->m_entities.erase(entity))
    {
    // TODO Need to detatch the attribute from the entity
    }
}
//----------------------------------------------------------------------------
void Attribute::removeAllAssociations()
{
  std::set<slctk::ModelEntity *>::iterator it;
  for (it = this->m_entities.begin(); it != this->m_entities.end(); it++)
    {
    // TODO Need to detatch the attribute from the entity
    }
}
//----------------------------------------------------------------------------
void Attribute::addComponent(Component *component)
{
  this->m_components.push_back(component);
  this->m_componentLookUp[component->name()] = component;
}
//----------------------------------------------------------------------------
