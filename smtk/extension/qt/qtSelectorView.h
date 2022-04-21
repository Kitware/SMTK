//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtSelectorView_h
#define smtk_extension_qtSelectorView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

/**\brief Provides the QT UI for a Selector View.
  *
  * A Selector View is similar to a Group View in that it is a set of Views.
  * However, unlike the Group View only one of its children views is visible.
  * The View that is visible is based on the value of a Discrete Item contained
  * in an attribute.  Currently that item should be the only item in the attribute.
  *
  * The structure of the Selector View has the following Top-Level view attributes in additon to
  * those that all views have:
  *
  * - Type = "Selector"
  * - SelectorName - The name of the attribute to be used for selection. if the attribute
  * does not exist it will be created
  * - SelectorType - The name of the selector attribute's definition.
  *
  * In terms of children components, the Selector View has a similar structure  as Group Views.
  * The Views component represent the list of children views used by the Selector.
  * Each child is represented by a View component that includes the following view attributes:
  *
  * - Title - name of the child view
  * - Enum - the enumeration value that the selector attribute's item needs to be set to for
  * this view to be visible
  *
  * \sa qtGroupView
  */

class qtSelectorViewInternals;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtSelectorView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtSelectorView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtSelectorView(const smtk::view::Information& info);
  ~qtSelectorView() override;

  void getChildView(const std::string& viewType, QList<qtBaseView*>& views);
  qtBaseView* getChildView(int pageIndex);

  virtual void addChildView(qtBaseView*, int viewEnumIndex);
  virtual void clearChildViews();
  const QList<qtBaseView*>& childViews() const;
  //Returns true if the view does not contain any information to display
  bool isEmpty() const override;
  bool isValid() const override;
public Q_SLOTS:
  void updateUI() override;
  void showAdvanceLevelOverlay(bool show) override;
  void updateModelAssociation() override;
  void onShowCategory() override;
  void selectionChanged();
  void refreshSelector();

protected:
  void createWidget() override;
  bool createSelector();
  bool createChildren();

private:
  qtSelectorViewInternals* m_internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
