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
// .NAME SimpleExpressionSection.h -
// .SECTION Description
// .SECTION See Also

#ifndef __SimpleExpressionSection_h
#define __SimpleExpressionSection_h

#include "smtk/attribute/Section.h"
#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <string>

namespace smtk
{
  namespace attribute
  {
    class SMTKCORE_EXPORT SimpleExpressionSection : public Section
    {
    public:
      SimpleExpressionSection(const std::string &myTitle);
      virtual ~SimpleExpressionSection();
      virtual Section::Type type() const;
      smtk::AttributeDefinitionPtr definition() const
      {return this->m_definition;}
      void setDefinition(smtk::AttributeDefinitionPtr def)
      {this->m_definition = def;}

    protected:
      smtk::AttributeDefinitionPtr m_definition;
    private:
    };
  };
};


#endif /* __SimpleExpressionSection_h */
