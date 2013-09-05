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
// .NAME qtAttribute - a class that encapsulates the UI of an Attribute 
// .SECTION Description

#ifndef __smtk_attribute_qtAttribute_h
#define __smtk_attribute_qtAttribute_h

#include <QObject>
#include "smtk/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

class qtAttributeInternals;
class QWidget;

namespace smtk
{
  namespace attribute
  {
  class qtItem;
    class QTSMTK_EXPORT qtAttribute : public QObject
    {
      Q_OBJECT

    public:         
      qtAttribute(smtk::AttributePtr, QWidget* parent);
      virtual ~qtAttribute();  

      smtk::AttributePtr getObject();
      QWidget* widget()
      {return this->Widget;}
      QWidget* parentWidget();

      virtual void addItem(qtItem*);
      virtual void clearItems();
      QList<qtItem*>& items() const;

      // create all the items
      static qtItem* createItem(smtk::AttributeItemPtr item, QWidget* p);
      static qtItem* createValueItem(smtk::ValueItemPtr item, QWidget* p);
      static qtItem* createDirectoryItem(smtk::DirectoryItemPtr item, QWidget* p);
      static qtItem* createAttributeRefItem(smtk::AttributeRefItemPtr item, QWidget* p);
      static qtItem* createFileItem(smtk::FileItemPtr item, QWidget* p, bool dirOnly=false);
      static qtItem* createGroupItem(smtk::GroupItemPtr item, QWidget* p);

    protected slots:
      virtual void updateItemsData();
      
    protected:
      virtual void createWidget();

      QWidget* Widget;
    private:

      qtAttributeInternals *Internals;
      
    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
