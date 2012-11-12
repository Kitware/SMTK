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

#ifndef __slctk_attribute_qtItem_h
#define __slctk_attribute_qtItem_h

#include <QObject>
#include "AttributeExports.h"
#include "attribute/PublicPointerDefs.h"
class qtItemInternals;

/*--------------------------------------------------------------------------*/
/* Define a casting macro for use by the constants below.  */
#if defined(__cplusplus)
# define slctk_TYPE_CAST(T, V) static_cast< T >(V)
#else
# define slctk_TYPE_CAST(T, V) ((T)(V))
#endif

/*--------------------------------------------------------------------------*/

#define slctk_INT_MIN                 slctk_TYPE_CAST(int, ~(~0u >> 1))
#define slctk_INT_MAX                 slctk_TYPE_CAST(int, ~0u >> 1)
#define slctk_UNSIGNED_INT_MIN        slctk_TYPE_CAST(unsigned int, 0)
#define slctk_UNSIGNED_INT_MAX        slctk_TYPE_CAST(unsigned int, ~0u)
#define slctk_LONG_MIN                slctk_TYPE_CAST(long, ~(~0ul >> 1))
#define slctk_LONG_MAX                slctk_TYPE_CAST(long, ~0ul >> 1)
#define slctk_UNSIGNED_LONG_MIN       slctk_TYPE_CAST(unsigned long, 0ul)
#define slctk_UNSIGNED_LONG_MAX       slctk_TYPE_CAST(unsigned long, ~0ul)
#define slctk_FLOAT_MIN               slctk_TYPE_CAST(float, -1.0e+38f)
#define slctk_FLOAT_MAX               slctk_TYPE_CAST(float,  1.0e+38f)
#define slctk_DOUBLE_MIN              slctk_TYPE_CAST(double, -1.0e+299)
#define slctk_DOUBLE_MAX              slctk_TYPE_CAST(double,  1.0e+299)

#define slctk_DOUBLE_CONSTRAINT_PRECISION 0.000001
#define slctk_USER_DATA_TYPE 10000

/*--------------------------------------------------------------------------*/

namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT qtItem : public QObject
    {
      Q_OBJECT

    public:         
      qtItem(slctk::AttributeItemPtr, QWidget* parent);
      virtual ~qtItem();  
      
      slctk::AttributeItemPtr getObject();
      QWidget* widget()
      {return this->Widget;}
      QWidget* parentWidget();
      
      virtual void addChildItem(qtItem*);
      virtual void clearChildItems();
      QList<qtItem*>& childItems() const;

      bool isLeafItem()
        {return this->IsLeafItem;}
      
      bool passAdvancedCheck(bool advanced);

    protected slots:
      virtual void updateItemData() {;}

    protected:
      virtual void createWidget(){;}
      QWidget* Widget;
      bool IsLeafItem;
    private:

      qtItemInternals *Internals;
      
    }; // class
  }; // namespace attribute
}; // namespace slctk

#endif
