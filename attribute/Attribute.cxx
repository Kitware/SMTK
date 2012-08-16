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
#include "attribute/AttributeRefItem.h"
#include "attribute/Cluster.h"
#include "attribute/Item.h"
#include "attribute/Definition.h"
#include <iostream>
using namespace slctk::attribute; 
//----------------------------------------------------------------------------
Attribute::Attribute(const std::string &myName, slctk::AttributeClusterPtr myCluster, 
                     unsigned long myId):
  m_name(myName), m_cluster(myCluster), m_id(myId)
{
}

//----------------------------------------------------------------------------
Attribute::~Attribute()
{
  std::cout << "Deleting Attribute " << this->name() << "\n";
  this->removeAllAssociations();
  this->removeAllItems();
 }
//----------------------------------------------------------------------------
void Attribute::removeAllItems()
{
  this->m_items.clear();
}
//----------------------------------------------------------------------------
const std::string &Attribute::type() const
{
  return this->m_cluster.lock()->type();
}
//----------------------------------------------------------------------------
std::vector<std::string> Attribute::types() const
{
  std::vector<std::string> tvec;
  slctk::AttributeClusterPtr c = this->m_cluster.lock();
  while (c != NULL)
    {
    tvec.push_back(c->type());
    c = c->parent();
    }
  return tvec;
}
//----------------------------------------------------------------------------
bool Attribute::isA(slctk::AttributeDefinitionPtr def) const
{
  return this->m_cluster.lock()->definition()->isA(def);
}
//----------------------------------------------------------------------------
slctk::AttributeDefinitionPtr Attribute::definition() const
{
  return this->m_cluster.lock()->definition();
}
//----------------------------------------------------------------------------
bool Attribute::isMemberOf(const std::string &catagory) const
{
  return this->m_cluster.lock()->definition()->isMemberOf(catagory);
}
//----------------------------------------------------------------------------
bool Attribute::isMemberOf(const std::vector<std::string> &catagories) const
{
  return this->m_cluster.lock()->definition()->isMemberOf(catagories);
}
//----------------------------------------------------------------------------
Manager *Attribute::manager() const
{
  return this->m_cluster.lock()->manager();
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
slctk::ConstAttributeItemPtr Attribute::find(const std::string &name) const
{
  int i = this->definition()->findItemPosition(name);
  if (i < 0)
    {
    return slctk::ConstAttributeItemPtr();
    }
  return this->m_items[i];
}

//----------------------------------------------------------------------------
slctk::AttributeItemPtr Attribute::find(const std::string &name)
{
  int i = this->definition()->findItemPosition(name);
  return (i < 0) ? slctk::AttributeItemPtr() : this->m_items[i];
}
//----------------------------------------------------------------------------
