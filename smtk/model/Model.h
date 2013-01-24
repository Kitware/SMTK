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
// .NAME Model.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_model_Model_h
#define __smtk_model_Model_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <string>
#include <map>
#include <vector>

namespace smtk
{
  namespace model
  {
    class SMTKCORE_EXPORT Model
    {
    public:
      Model();
      virtual ~Model();
      virtual smtk::ModelItemPtr getModelItem(int id);
      virtual smtk::ModelGroupItemPtr createModelGroup(
        const std::string &name, int myid, unsigned int mask) = 0;
      virtual bool deleteModelGroup(int id) = 0;
      smtk::ModelItemPtr modelDomain() const
      {return this->m_modelDomain;}

      virtual unsigned int convertGroupTypeToMask(int grouptype) = 0;
      virtual void loadGroupItems(int grouptype){;}
      virtual void removeGroupItems(int grouptype)
      { return this->removeGroupItems(this->convertGroupTypeToMask(grouptype));}
      virtual void removeGroupItems(unsigned int mask);
      virtual void findGroupItems(unsigned int mask, 
        std::vector<smtk::ModelGroupItemPtr> &result) const;
      
      virtual std::size_t numberOfItems()
      { return this->m_items.size(); }
      std::map<int, smtk::ModelItemPtr>::const_iterator itemIterator() const
        {return this->m_items.begin();}

      typedef enum
        {
        AllNodesType = 0,
        BoundaryNodesType,
        InteriorNodesType
        } ModelEntityNodalTypes;
      static std::string convertNodalTypeToString(ModelEntityNodalTypes t);

    protected:
      smtk::ModelItemPtr m_modelDomain;
      mutable std::map<int, smtk::ModelItemPtr> m_items;
    private:
    };

    inline smtk::ModelItemPtr Model::getModelItem(int id)
    {
      std::map<int, smtk::ModelItemPtr>::iterator it = this->m_items.find(id);
      if (it == this->m_items.end())
        {
        return smtk::ModelItemPtr();
        }
      return it->second;
    }
  };
};


#endif /* __smtk_model_Model_h */
