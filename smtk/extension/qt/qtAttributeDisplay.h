//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAttributeDisplay - display controls for entity associated Attribute
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtAttributeDisplay_h
#define __smtk_extension_qtAttributeDisplay_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QWidget>

#include <QMap>

class QTableWidgetItem;
class qtAttributeDisplayInternals;

namespace smtk
{
  namespace extension
  {

    class qtUIManager;

    class SMTKQTEXT_EXPORT qtAttributeDisplay : public QWidget
    {
      Q_OBJECT

    public:
      qtAttributeDisplay(QWidget* p, smtk::extension::qtUIManager* uiman);
      virtual ~qtAttributeDisplay();

    public slots:
      void onShowCategory();
      void onShowCategory(const std::string& strCategory);
      void onAttributeDefSelected();
      void onFieldSelected();
      virtual void getDefinitionsWithAssociations();

    signals:
      void attColorChanged();
      void attributeFieldSelected(const QString& attdeftype,
        const QString& itemname);

    protected slots:
      void enableShowBy(int enable);

    protected:
      virtual void createWidget( );
      smtk::attribute::ItemPtr getAttributeItemFromItem(QTableWidgetItem * item);

      void initSelectionFilters(const QString& currentItemName="");
      void initSelectPropCombo(smtk::attribute::DefinitionPtr attDef,
                               const QString& currentItemName="");

    private:

      qtAttributeDisplayInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
