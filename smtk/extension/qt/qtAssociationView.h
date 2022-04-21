//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtAssociationView - View for modifying attribute association information
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtk_extension_qtAssociationView_h
#define smtk_extension_qtAssociationView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include <QMap>
#include <QModelIndex>

class qtAssociationViewInternals;
class QTableWidgetItem;
class QKeyEvent;
class QStandardItem;
class QTableWidget;

namespace smtk
{
namespace extension
{
class qtAssociationWidget;
class qtBaseView;
/**\brief A View that provides a GUI to edit an attribute's association information.
  *
  * The JSON and XML format for this View is the same as the one used for qtAttributeView with the
  * exception that the Type is Associations instead of Attribute.  Note that the View does not
  * allow for the creation of attributes or the editing of an attribute's non-association
  * related information.
  *
  * Here is an example XML
  * ```
  *   <View Type="Associations" Title="Material Assignment" Label="Assignment">
  *    <AttributeTypes>
  *      <Att Type="material"/>
  *    </AttributeTypes>
  *  </View>
  * ```
  */
class SMTKQTEXT_EXPORT qtAssociationView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtAssociationView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtAssociationView(const smtk::view::Information& info);
  ~qtAssociationView() override;
  const QMap<QString, QList<smtk::attribute::DefinitionPtr>>& attDefinitionMap() const;

  bool isEmpty() const override;

public Q_SLOTS:
  void updateUI() override;
  void onShowCategory() override;
  void updateModelAssociation() override;
  void onAttributeChanged(int);
  void associationsChanged();

Q_SIGNALS:
  void attAssociationChanged();

protected:
  void createWidget() override;
  virtual smtk::extension::qtAssociationWidget* createAssociationWidget(
    QWidget* parent,
    qtBaseView* view);
  virtual void getAllDefinitions();
  smtk::attribute::AttributePtr getAttributeFromIndex(int index);

private:
  qtAssociationViewInternals* Internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
