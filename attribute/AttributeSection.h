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
// .NAME AttributeSection.h -
// .SECTION Description
// .SECTION See Also

#ifndef __AttributeSection_h
#define __AttributeSection_h

#include "attribute/Section.h"
#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"
#include <string>
#include <vector>

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT AttributeSection : public Section
    {
    public:
      AttributeSection(const std::string &myTitle);
      virtual ~AttributeSection();
      virtual Section::Type type() const;
      void addDefinition(slctk::AttributeDefinitionPtr def)
      {this->m_definitions.push_back(def);}
      std::size_t numberOfDefinitions() const
      { return this->m_definitions.size();}
      slctk::AttributeDefinitionPtr definition(int ith) const
      {return this->m_definitions[ith];}
      unsigned long modelEntityMask() const
      {return this->m_modelEntityMask;}
      void setModelEntityMask(unsigned long mask)
      {this->m_modelEntityMask = mask;}
      bool okToCreateModelEntities() const
      { return this->m_okToCreateModelEntities;}
      void setOkToCreateModelEntities(bool val)
      { this->m_okToCreateModelEntities = val;}

    protected:
      std::vector<slctk::AttributeDefinitionPtr> m_definitions;
      unsigned long m_modelEntityMask;
      bool m_okToCreateModelEntities;

    private:
    };
  };
};


#endif /* __AttributeSection_h */
