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

#include "attribute/Cluster.h"
#include "attribute/Attribute.h"
#include "attribute/Definition.h"
using namespace slctk::attribute; 
//----------------------------------------------------------------------------
Cluster::Cluster(Manager *myManager, Cluster *myParent):
  m_manager(myManager), m_parent(myParent), m_definition(NULL)
{
}

//----------------------------------------------------------------------------
Cluster::~Cluster()
{
  std::map<std::string,  Attribute*>::iterator it;
  for (it = this->m_attributes.begin(); it != this->m_attributes.end(); it++)
    {
    delete (*it).second;
    }
  if (this->m_definition)
    {
    delete this->m_definition;
    }
}
//----------------------------------------------------------------------------
Definition *Cluster::generateDefinition(const std::string &typeName,
                                        unsigned long defId)
{
  if (this->m_definition != NULL)
    {
    return NULL;
    }
  this->m_definition = new Definition(typeName, this, defId);
  return this->m_definition;
}
//----------------------------------------------------------------------------
Attribute *Cluster::generateAttribute(const std::string &name,
                                        unsigned long attId)
{
  //Do we already have an attribute with the same name
  if (this->find(name) != NULL)
    {
    return NULL;
    }
  Attribute *a = new Attribute(name, this, attId);
  if (this->m_definition)
    {
    this->m_definition->buildAttribute(a);
    }
  this->m_attributes[name] = a;
  return a;
}
//----------------------------------------------------------------------------
const std::string &Cluster::type() const
{
  return this->m_definition->type();
}
//----------------------------------------------------------------------------
void Cluster::deleteAttribute(Attribute *att)
{
  this->m_attributes.erase(att->name());
  delete att;
}
//----------------------------------------------------------------------------
bool Cluster::rename(Attribute *att, const std::string &newName)
{
  // Can only remove attributes that are owned by this cluster!
  if (att->cluster() != this)
    {
    return false;
    }
  this->m_attributes.erase(att->name());
  att->setName(newName);
  this->m_attributes[att->name()] = att;
}
  
