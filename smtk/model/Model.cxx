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


#include "smtk/model/Model.h"

#include "smtk/model/GridInfo.h"
#include "smtk/model/ModelDomainItem.h"
#include "smtk/model/GroupItem.h"

using namespace smtk::model;

//----------------------------------------------------------------------------
Model::Model()
{
  this->m_modelDomain = smtk::model::ItemPtr(new ModelDomainItem(this, -1));
  this->m_items[-1] = this->m_modelDomain;
}

//----------------------------------------------------------------------------
Model::~Model()
{
  std::map<int, smtk::model::ItemPtr>::iterator it;
  for (it = this->m_items.begin(); it != this->m_items.end(); it++)
    {
    it->second->clearModel();
    }
}

//----------------------------------------------------------------------------
smtk::model::GroupItemPtr Model::createModelGroup(
  const std::string &name, int groupid,unsigned long mask)
{
  smtk::model::GroupItemPtr aGroup =
    smtk::model::GroupItemPtr(new GroupItem(this, groupid, mask));
  this->m_items[groupid] = aGroup;
  aGroup->setName(name);
  return aGroup;
}

//----------------------------------------------------------------------------
std::vector<smtk::model::GroupItemPtr> Model::findGroupItems(
                                                    unsigned int mask) const
{
  std::vector<smtk::model::GroupItemPtr> result;
  result.reserve(this->m_items.size()/2); //guess half the items
  std::map<int, smtk::model::ItemPtr>::iterator it;
  for (it = this->m_items.begin(); it != this->m_items.end(); it++)
    {
    if(it->second->type() == Item::BOUNDARY_GROUP || it->second->type() == Item::DOMAIN_SET)
      {
      smtk::model::GroupItemPtr itemgrp = dynamic_pointer_cast<GroupItem>(it->second);
      if(itemgrp && ((itemgrp->entityMask() & mask) == mask))
        {
        result.push_back(itemgrp);
        }
      }
    }
  result.reserve(result.size());
  return result;
}
//----------------------------------------------------------------------------
void Model::removeGroupItemsByMask(unsigned int mask)
{
  std::vector<smtk::model::GroupItemPtr> result = this->findGroupItems(mask);
  typedef std::vector<smtk::model::GroupItemPtr>::iterator iter;
  for(iter it=result.begin(); it!=result.end(); ++it)
    {
    this->deleteModelGroup((*it)->id());
    }
}

//----------------------------------------------------------------------------
std::string Model::convertNodalTypeToString(ModelEntityNodalTypes t)
{
  switch(t)
    {
    case AllNodesType:
      return "All Nodes";
    case BoundaryNodesType:
      return "Boundary Nodes";
    case InteriorNodesType:
      return "Interior Nodes";
    default:
      break;
    }
  return "Undefined";
}
