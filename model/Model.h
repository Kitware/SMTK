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

#ifndef __slctk_model_Model_h
#define __slctk_model_Model_h

#include "ModelExports.h"
#include "PublicPointerDefs.h"
#include <string>
#include <map>

namespace slctk
{
  namespace model
  {
    class SLCTKMODEL_EXPORT Model
    {
    public:
      Model();
      virtual ~Model();
      virtual slctk::ModelItemPtr findModelItem(int id) const;
      virtual slctk::ModelGroupItemPtr createModelGroup(const std::string &name, unsigned int mask) = 0;
      virtual bool deleteModelGroup(int id) = 0;
      slctk::ModelItemPtr modelDomain() const
      {return this->m_modelDomain;}
      
    protected:
      slctk::ModelItemPtr m_modelDomain;
      mutable std::map<int, slctk::ModelItemPtr> m_items;
    private:
    };

    inline slctk::ModelItemPtr Model::findModelItem(int id) const
    {
      std::map<int, slctk::ModelItemPtr>::const_iterator it = this->m_items.find(id);
      if (it == this->m_items.end())
        {
        return slctk::ModelItemPtr();
        }
      return it->second;
    }
  };
};


#endif /* __slctk_model_Model_h */
