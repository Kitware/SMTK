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
// .NAME Item.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_model_Item_h
#define __smtk_model_Item_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <string>
#include <set>

namespace smtk
{
  namespace model
  {
    class Model;
    class SMTKCORE_EXPORT Item
    {
      friend class Model;
    public:
      enum Type
      {
        VERTEX = 1,
        EDGE = 2,
        FACE=4,
        REGION=8,
        MODEL_DOMAIN=16,
        GROUP=32
      };
      Item(Model *model, int myid);
      virtual ~Item();
      int id() const
      {return this->m_id;}
      virtual Item::Type type() const = 0;

      virtual std::string name() const
      { return this->m_UserName; }
      virtual void setName(const std::string & strname)
      { this->m_UserName = strname; }


      virtual void attachAttribute(smtk::attribute::AttributePtr);
      virtual void detachAttribute(smtk::attribute::AttributePtr, bool reverse=true);

      virtual void detachAllAttributes();
      virtual bool isAttributeAssociated(smtk::attribute::AttributePtr) const;

      typedef std::set<smtk::attribute::AttributePtr>::const_iterator const_iterator;

      std::set<smtk::attribute::AttributePtr> AssociatedAttributes() const
      { return this->m_attributes;  }

      const_iterator beginAssociatedAttributes() const
        {return this->m_attributes.begin();}

      const_iterator endAssociatedAttributes() const
        {return this->m_attributes.end();}

      std::set<smtk::attribute::AttributePtr> attributes() const
        {return this->m_attributes;}

      std::size_t numberOfAssociatedAttributes() const
        { return this->m_attributes.size();}

      Model *model() const
      {return this->m_model;}
      // Return the public pointer for this model item.
      smtk::model::ItemPtr pointer() const;

    protected:
      void clearModel()
      {this->m_model = NULL;}

      Model *m_model;
      int m_id;
      std::string m_UserName;
      std::set<smtk::attribute::AttributePtr> m_attributes;

    private:
    };
  };
};


#endif /* __smtk_model_Item_h */
