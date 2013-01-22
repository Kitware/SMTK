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
// .NAME qtAttributeSection - the Attribute Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtAttributeSection_h
#define __smtk_attribute_qtAttributeSection_h

#include "smtk/Qt/qtSection.h"

class qtAttributeSectionInternals;
class QTableWidgetItem;
class QKeyEvent;

namespace smtk
{
  namespace attribute
  {
    class QTSMTK_EXPORT qtAttributeSection : public qtSection
    {
      Q_OBJECT

    public:
      qtAttributeSection(smtk::SectionPtr, QWidget* p);
      virtual ~qtAttributeSection();

      QTableWidgetItem* getSelectedItem();
      int currentViewBy();
      int currentCategory();
      virtual void createNewAttribute(smtk::AttributeDefinitionPtr attDef);

      enum enumViewBy
        {
        VIEWBY_Attribute = 0,
        VIEWBY_PROPERTY
        };

    public slots:
      void onViewBy(int);
      void onViewByWithDefinition(
        int viewBy, smtk::AttributeDefinitionPtr attDef);
      void onShowCategory();
      void onListBoxSelectionChanged();
      void onAttributeValueChanged(QTableWidgetItem*);
      void onAttributeNameChanged(QTableWidgetItem*);
      void onCreateNew();
      void onCopySelected();
      void onDeleteSelected();
      void onAttributeModified();
      void updateAssociationEnableState(smtk::AttributePtr);
      virtual void updateModelAssociation();

    protected:
      virtual void createWidget( );
      smtk::AttributePtr getAttributeFromItem(QTableWidgetItem * item);
      smtk::AttributeItemPtr getAttributeItemFromItem(QTableWidgetItem * item);

      smtk::AttributePtr getSelectedAttribute();
      QTableWidgetItem* addAttributeListItem(smtk::AttributePtr childData);
      void addAttributePropertyItems(
        smtk::AttributePtr childData, const QString& group);
      void updateTableWithAttribute(smtk::AttributePtr dataItem, const QString& group);
      void updateTableWithProperty(QString& propertyName);
      void addTableGroupItems(
        smtk::GroupItemPtr childData, int& numRows, const char* strCommonLabel=NULL);
      void addTableValueItems(
        smtk::ValueItemPtr attItem, int& numRows);
      void addTableValueItems(
        smtk::ValueItemPtr attItem, int& numRows,
        const char* attLabel, int advanced);

      void updateChildWidgetsEnableState(
        smtk::AttributeItemPtr linkedData, QTableWidgetItem* item);
      void updateItemWidgetsEnableState(
        smtk::ValueItemPtr linkedData, int &startRow, bool enabled);
      virtual void getAllDefinitions();
      bool hasMultiDefinition(const QString& group);

    private:

      qtAttributeSectionInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
