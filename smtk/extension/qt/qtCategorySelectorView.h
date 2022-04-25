//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtCategorySelectorView_h
#define smtk_extension_qtCategorySelectorView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

/**\brief Provides the QT UI for a Category-based Selection View.
  *
  * A Category-based Selector View is similar to a Group View in that it is a set of Views.
  * However, unlike the Group View only one of its children views is visible.
  * The View that is visible is based on the value of the current category.  If category filtering is
  * off then nothing is visible
  *
  * The structure of the Category-based Selector View has the following Top-Level view attributes in additon to
  * those that all views have:
  *
  * - Type = "Category"
  *
  * In terms of children components, the Category-based Selector View has a similar structure  as Group Views.
  * The Views component represent the list of children views used by the Selector.
  * Each child is represented by a View component that includes the following view attributes:
  *
  * - Title - name of the child view
  * - Category - the  value that the category the Filter by Category pull down needs to be set to for
  * this view to be visible
  *
  * \sa qtGroupView
  */

class qtCategorySelectorViewInternals;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtCategorySelectorView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtCategorySelectorView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtCategorySelectorView(const smtk::view::Information& info);
  ~qtCategorySelectorView() override;

  void getChildView(const std::string& viewType, QList<qtBaseView*>& views);
  qtBaseView* getChildView(int pageIndex);

  virtual void addChildView(qtBaseView*, const std::string& category);
  virtual void clearChildViews();
  const QList<qtBaseView*>& childViews() const;
  bool isValid() const override;

public Q_SLOTS:
  void updateUI() override;
  void showAdvanceLevelOverlay(bool show) override;
  void updateModelAssociation() override;
  void onShowCategory() override;

protected:
  void createWidget() override;
  bool createChildren();

private:
  qtCategorySelectorViewInternals* Internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
