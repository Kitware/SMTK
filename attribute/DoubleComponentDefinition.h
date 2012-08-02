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
// .NAME DoubleComponentDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_DoubleComponentDefinition_h
#define __slctk_attribute_DoubleComponentDefinition_h

#include "ValueComponentDefinitionTemplate.h"

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT DoubleComponentDefinition :
      public ValueComponentDefinitionTemplate<double>
    {
    public:
      DoubleComponentDefinition(const std::string &myName,
                                unsigned long myId);
      virtual ~DoubleComponentDefinition();
      virtual slctk::attribute::Component *buildComponent() const;

    protected:

    private:

    };
  };
};

#endif /* __slctk_attribute_DoubleComponentDefinition_h */
