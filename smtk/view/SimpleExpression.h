//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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
