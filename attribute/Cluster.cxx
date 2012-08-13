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
Cluster::Cluster(Manager *myManager):
  m_manager(myManager), m_parent(), m_definition()
{
}

//----------------------------------------------------------------------------
Cluster::~Cluster()
{
}
//----------------------------------------------------------------------------
const std::string &Cluster::type() const
{
  return this->m_definition->type();
}
//----------------------------------------------------------------------------
void Cluster::removeAttribute(slctk::AttributePtr att)
{
  this->m_attributes.erase(att->name());
}
//----------------------------------------------------------------------------
void Cluster::addAttribute(slctk::AttributePtr att)
{
  this->m_attributes[att->name()] = att;
}
//----------------------------------------------------------------------------
bool Cluster::rename(slctk::AttributePtr att, const std::string &newName)
{
  // Can only remove attributes that are owned by this cluster!
  if (att->cluster().get() != this)
    {
    return false;
    }
  this->m_attributes.erase(att->name());
  att->setName(newName);
  this->m_attributes[att->name()] = att;
  return true;
}
  
