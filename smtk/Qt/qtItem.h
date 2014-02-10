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
// .NAME qtItem - an abstract class that encapsulates a single piece of data 
// .SECTION Description

#ifndef __smtk_attribute_qtItem_h
#define __smtk_attribute_qtItem_h

#include <QObject>
#include "smtk/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

class qtItemInternals;

/*--------------------------------------------------------------------------*/
/* Define a casting macro for use by the constants below.  */
#if defined(__cplusplus)
# define smtk_TYPE_CAST(T, V) static_cast< T >(V)
#else
# define smtk_TYPE_CAST(T, V) ((T)(V))
#endif

/*--------------------------------------------------------------------------*/

#define smtk_INT_MIN                 smtk_TYPE_CAST(int, ~(~0u >> 1))
#define smtk_INT_MAX                 smtk_TYPE_CAST(int, ~0u >> 1)
#define smtk_UNSIGNED_INT_MIN        smtk_TYPE_CAST(unsigned int, 0)
#define smtk_UNSIGNED_INT_MAX        smtk_TYPE_CAST(unsigned int, ~0u)
#define smtk_LONG_MIN                smtk_TYPE_CAST(long, ~(~0ul >> 1))
#define smtk_LONG_MAX                smtk_TYPE_CAST(long, ~0ul >> 1)
#define smtk_UNSIGNED_LONG_MIN       smtk_TYPE_CAST(unsigned long, 0ul)
#define smtk_UNSIGNED_LONG_MAX       smtk_TYPE_CAST(unsigned long, ~0ul)
#define smtk_FLOAT_MIN               smtk_TYPE_CAST(float, -1.0e+38f)
#define smtk_FLOAT_MAX               smtk_TYPE_CAST(float,  1.0e+38f)
#define smtk_DOUBLE_MIN              smtk_TYPE_CAST(double, -1.0e+299)
#define smtk_DOUBLE_MAX              smtk_TYPE_CAST(double,  1.0e+299)

#define smtk_DOUBLE_CONSTRAINT_PRECISION 0.000001
#define smtk_USER_DATA_TYPE 10000

/*--------------------------------------------------------------------------*/

namespace smtk
{
  namespace attribute
  {
    class qtBaseView;
    class QTSMTK_EXPORT qtItem : public QObject
    {
      Q_OBJECT

    public:         
      qtItem(smtk::attribute::ItemPtr, QWidget* parent, qtBaseView* bview);
      virtual ~qtItem();  
      
      smtk::attribute::ItemPtr getObject();
      QWidget* widget()
      {return this->Widget;}
      QWidget* parentWidget();
      
      virtual void addChildItem(qtItem*);
      virtual void clearChildItems();
      QList<qtItem*>& childItems() const;

      bool isLeafItem()
        {return this->IsLeafItem;}
      
      bool passAdvancedCheck();

    protected slots:
      virtual void updateItemData() {;}

    protected:
      virtual void createWidget(){;}
      virtual qtBaseView* baseView();

      QWidget* Widget;
      bool IsLeafItem;
    private:

      qtItemInternals *Internals;
    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
