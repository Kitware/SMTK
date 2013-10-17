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
// .NAME SimpleExpression.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_view_SimpleExpression_h
#define __smtk_view_SimpleExpression_h

#include "smtk/view/Base.h"
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <string>

namespace smtk
{
  namespace view
  {
    class SMTKCORE_EXPORT SimpleExpression : public Base
    {
    public:
      static smtk::view::SimpleExpressionPtr New(const std::string &myName)
      { return smtk::view::SimpleExpressionPtr(new SimpleExpression(myName)); }

      SimpleExpression(const std::string &myTitle);
      virtual ~SimpleExpression();
      virtual Base::Type type() const;
      smtk::attribute::DefinitionPtr definition() const
      {return this->m_definition;}
      void setDefinition(smtk::attribute::DefinitionPtr def)
      {this->m_definition = def;}

    protected:
      smtk::attribute::DefinitionPtr m_definition;
    private:
    };
  }
}


#endif /* __smtk_view_SimpleExpression_h */
