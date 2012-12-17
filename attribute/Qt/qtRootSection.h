/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME qtRootSection - a Root Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __slctk_attribute_qtRootSection_h
#define __slctk_attribute_qtRootSection_h

#include "qtSection.h"
#include "attribute/Section.h"

class qtRootSectionInternals;
class QScrollArea;

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT qtRootSection : public qtSection
    {
      Q_OBJECT

    public:
      qtRootSection(slctk::RootSectionPtr, QWidget* p);
      virtual ~qtRootSection();
      qtSection* getSection(slctk::attribute::Section::Type secType);

    public slots:
      virtual void showAdvanced(int show);

    protected:
      virtual void createWidget( );
      virtual void initRootTabGroup( );
      QScrollArea *ScrollArea;

    private:

      qtRootSectionInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace slctk


#endif
