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
// .NAME AttributeWriter.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_AttributeWriter_h
#define __slctk_attribute_AttributeWriter_h

#include "AttributeExports.h"
#include <string>

namespace slctk
{
  namespace attribute
  {
    class Manager;
    class SLCTKATTRIBUTE_EXPORT AttributeWriter
    {
    public:
      // Returns true if there was a problem with writing the file
      bool write(const Manager &manager, const std::string &filename);
      const std::string &errorMessages() const
      {return this->m_errorMessages;}

    protected:
      std::string m_errorMessages;
    private:
    };
  };
};


#endif /* __slctk_attribute_AttributeWriter_h */
