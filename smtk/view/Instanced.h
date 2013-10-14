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
// .NAME InstancedSection.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_Instanced_h
#define __smtk_view_Instanced_h

#include "smtk/view/Base.h"
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <vector>

//NOTE THAT WE ASSUME THAT THE READER Takes care of creating the instances if they are
// not found!!
namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT Instanced : public Base
    {
    public:
      static smtk::view::InstancedPtr New(const std::string &myName)
      { return smtk::view::InstancedPtr(new Instanced(myName)); }

      Instanced(const std::string &myTitle);
      virtual ~Instanced();
      virtual Base::Type type() const;
      void addInstance(smtk::AttributePtr att)
        { if(att.get() != NULL)
            {this->m_instances.push_back(att);}
        }
      std::size_t numberOfInstances() const
      {return this->m_instances.size();}
      smtk::AttributePtr instance(int ith) const
      {return this->m_instances[ith];}

    protected:
      std::vector<smtk::AttributePtr> m_instances;
    private:

    };
  }
}


#endif /* __smtk_view_Instanced_h */
