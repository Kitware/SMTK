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

#include "smtk/model/ModelDomainItem.h"
#include "smtk/model/GroupItem.h"

using namespace smtk::model; 

//----------------------------------------------------------------------------
Model::Model()
{
  this->m_modelDomain = smtk::ModelItemPtr(new ModelDomainItem(this, -1));
  this->m_items[-1] = this->m_modelDomain;
}

//----------------------------------------------------------------------------
Model::~Model()
{
  std::map<int, smtk::ModelItemPtr>::iterator it;
  for (it = this->m_items.begin(); it != this->m_items.end(); it++)
    {
    it->second->clearModel();
    }
}
//----------------------------------------------------------------------------
void Model::findGroupItems(unsigned int mask, 
  std::vector<smtk::ModelGroupItemPtr> &result) const
{
  std::map<int, smtk::ModelItemPtr>::iterator it;
  for (it = this->m_items.begin(); it != this->m_items.end(); it++)
    {
    if(it->second->type() == Item::GROUP)
      {
      smtk::ModelGroupItemPtr itemgrp = dynamicCastPointer<GroupItem>(it->second);
      if(itemgrp && (itemgrp->entityMask() & mask))
        {
        result.push_back(itemgrp);
        }
      }
    }
}
//----------------------------------------------------------------------------
void Model::removeGroupItems(unsigned int mask)
{
  std::vector<smtk::ModelGroupItemPtr> result;
  this->findGroupItems(mask, result);
  std::vector<smtk::ModelGroupItemPtr>::iterator it = result.begin();
  for(; it!=result.end(); ++it)
    {
    this->deleteModelGroup((*it)->id());
    }
}

//----------------------------------------------------------------------------
std::string Model::convertNodalTypeToString(ModelEntityNodalTypes t)
{
  switch(t)
    {
    case ModelEntityNodalTypes::AllNodesType:
      return "All Nodes";
    case ModelEntityNodalTypes::BoundaryNodesType:
      return "Boundary Nodes";
    case ModelEntityNodalTypes::InteriorNodesType:
      return "Interior Nodes";
    default:
      break;
    }
  return "Undefined";
}
