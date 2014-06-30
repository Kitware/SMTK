/*=========================================================================

Copyright (c) 1998-2014 Kitware Inc. 28 Corporate Drive,
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
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME smtkResource.h - Abstract base class for SMTK resources
// .SECTION Description
//   An SMTK resource is one of: attribute manager, model, mesh
// .SECTION See Also

#ifndef __smtk_util_Resource_h
#define __smtk_util_Resource_h


#include "smtk/SMTKCoreExports.h"
#include <string>


namespace smtk
{
  namespace util
  {
    class SMTKCORE_EXPORT Resource
    {
    public:
      /// Identifies resource type
      enum Type
      {
        ATTRIBUTE = 0,
        MODEL,        // future
        MESH,         // future
        NUMBER_OF_TYPES
      };

      virtual Resource::Type resourceType() const = 0;

      static std::string type2String(Resource::Type t);
      static Resource::Type string2Type(const std::string &s);

    protected:
      Resource();
      virtual ~Resource();
    };
  }
}

#endif  /* __smtk_util_Resource_h */
