//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtItem - an abstract UI class for attribute item
// .SECTION Description

#ifndef __smtk_extension_qtItem_h
#define __smtk_extension_qtItem_h

#include <QObject>
#include "smtk/extension/qt/Exports.h"
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
  namespace extension
  {
    class qtBaseView;
    class SMTKQTEXT_EXPORT qtItem : public QObject
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

      virtual void setLabelVisible(bool) {;}

      bool passAdvancedCheck();
      void showAdvanceLevelOverlay(bool);

    signals:
       void widgetSizeChanged();

    protected slots:
      virtual void updateItemData();
      virtual void onAdvanceLevelChanged(int levelIdx);
      virtual void onChildWidgetSizeChanged(){;}

   protected:
      virtual void createWidget() {;}
      virtual qtBaseView* baseView();
      virtual void setAdvanceLevel(int level);

      QWidget* Widget;
      bool IsLeafItem;

    private:

      qtItemInternals *Internals;
    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
