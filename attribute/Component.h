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
// .NAME Component.h -
// .SECTION Description
// .SECTION See Also

#ifndef __slctk_attribute_Component_h
#define __slctk_attribute_Component_h

#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"
#include <string>
#include <vector>


namespace slctk
{
  namespace attribute
  {
    class ComponentDefinition;
    class SLCTKATTRIBUTE_EXPORT Component
    {
    public:
     enum Type
     {
       ATTRIBUTE_REFERENCE,
       DOUBLE,
       GROUP,
       INTEGER,
       STRING,
       VOID,
       NUMBER_OF_TYPES
     };
       
      Component();
      virtual ~Component();
      const std::string &name() const;
      virtual Component::Type type() const = 0;
      virtual bool setDefinition(slctk::ConstAttributeComponentDefinitionPtr def);
      slctk::ConstAttributeComponentDefinitionPtr definition() const
      {return this->m_definition;}

      bool isOptional() const;

      // isEnabled only matters for optional components.  All non-optional
      // components will return true for isEnabled regardless of the value 
      // of m_isEnabled
      bool isEnabled() const;
      void setIsEnabled(bool isEnabledValue)
      {this->m_isEnabled = isEnabledValue;}

      bool isMemberOf(const std::string &catagory) const;
      bool isMemberOf(const std::vector<std::string> &catagories) const;

      static std::string type2String(Component::Type t);
      static Component::Type string2Type(const std::string &s);

     protected:
      // This method allows any Component to delete another - USE WITH CARE!
      void deleteComponent();
      bool m_isEnabled;
      mutable std::string m_tempString;
      slctk::ConstAttributeComponentDefinitionPtr m_definition;
    private:
      
    };
  };
};

#endif /* __slctk_attribute_Component_h */
