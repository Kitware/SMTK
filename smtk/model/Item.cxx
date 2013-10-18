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


#include "smtk/model/Item.h"

#include "smtk/model/Model.h"
#include "smtk/attribute/Attribute.h"

using namespace smtk::model;

//----------------------------------------------------------------------------
Item::Item(Model *model, int myid, unsigned long mask):
  m_model(model), m_id(myid), m_entityMask(mask)
{
  this->m_UserName = "";
}

//----------------------------------------------------------------------------
Item::~Item()
{
  this->detachAllAttributes();
}
//----------------------------------------------------------------------------
smtk::model::ItemPtr Item::pointer() const
{
  if (this->m_model)
    {
    return this->m_model->getModelItem(this->m_id);
    }
  return smtk::model::ItemPtr();
}
//----------------------------------------------------------------------------
bool Item::isAttributeAssociated(smtk::attribute::AttributePtr anAtt) const
{
  return (this->m_attributes.find(anAtt) != this->m_attributes.end());
}

//----------------------------------------------------------------------------
void Item::attachAttribute(smtk::attribute::AttributePtr anAtt)
{
  if (this->isAttributeAssociated(anAtt))
    {
    // Nothing to be done
    return;
    }
  this->m_attributes.insert(anAtt);
  anAtt->associateEntity(this->pointer());
}
//----------------------------------------------------------------------------
void Item::detachAttribute(smtk::attribute::AttributePtr anAtt, bool reverse)
{
  if (this->isAttributeAssociated(anAtt))
    {
    this->m_attributes.erase(anAtt);
    if(reverse)
      {
      anAtt->disassociateEntity(this->pointer());
      }
    }
}
//----------------------------------------------------------------------------
void Item::detachAllAttributes()
{
  if(this->m_attributes.size() == 0)
    {
    return;
    }

  for (const_iterator associatedAtt = this->m_attributes.begin();
       associatedAtt != this->m_attributes.end();
       ++associatedAtt)
    {
    (*associatedAtt)->disassociateEntity(this->pointer(), false);
    }
  this->m_attributes.clear();
}
