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
// .NAME qtSection - a class that encapsulates the UI of an Attribute 
// .SECTION Description

#ifndef __smtk_attribute_qtSection_h
#define __smtk_attribute_qtSection_h

#include <QObject>
#include "smtk/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"
#include <QList>

class qtSectionInternals;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtSection : public QObject
    {
      Q_OBJECT

    public:         
      qtSection(smtk::SectionPtr, QWidget* parent);
      virtual ~qtSection();

      smtk::SectionPtr getObject();
      QWidget* widget()
      {return this->Widget;}
      QWidget* parentWidget();
     
    public slots:
      virtual void showAdvanced(int show){;}

    protected slots:
      virtual void updateAttributeData() {;}
      
    protected:
      virtual void createWidget(){;}
      virtual void getDefinitions(smtk::AttributeDefinitionPtr attDef,
        QList<smtk::AttributeDefinitionPtr>& defs);

      QWidget* Widget;
    private:

      qtSectionInternals *Internals;
      
    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
