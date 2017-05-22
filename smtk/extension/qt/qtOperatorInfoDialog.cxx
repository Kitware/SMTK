//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtOperatorInfoDialog - A Information Dialog for SMTK Operators
// .SECTION Description
// .SECTION Caveats

#include "qtOperatorInfoDialog.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "ui_qtOperatorInfoDialog.h"

#include "smtk/model/Operator.h"

using namespace smtk::extension;

qtOperatorInfoDialog::qtOperatorInfoDialog(QWidget* Parent)
  : QDialog(Parent)
  , m_dialog(new Ui::qtOperatorInfoDialog())
{
  this->m_dialog->setupUi(this);
  this->setObjectName("qtOperatorInfoDialog");
}

qtOperatorInfoDialog::~qtOperatorInfoDialog()
{
  delete this->m_dialog;
}

void qtOperatorInfoDialog::displayOperator(smtk::model::OperatorPtr op)
{
  this->m_op = op;
  this->m_dialog->textBrowser->clear();
  // Process the information of the operator's attribute
  smtk::attribute::AttributePtr att = op->specification();
  if (!att)
  {
    this->m_dialog->textBrowser->insertHtml(
      "<b>Missing Operator Specification</b>\nNo Information Available.");
    return;
  }
  smtk::attribute::DefinitionPtr def = att->definition();
  std::string s("<!DOCTYPE html><html><head><title>");
  s.append(def->label()).append("</title></head><body>");
  this->m_dialog->textBrowser->insertHtml(s.c_str());

  s = "<h2>";
  s.append(def->label()).append(" Operator Information</h2>");
  this->m_dialog->textBrowser->insertHtml(s.c_str());
  this->m_dialog->textBrowser->insertHtml("<hr size=\"1\" /><br>");
  this->m_dialog->textBrowser->insertHtml(def->detailedDescription().c_str());
  s = "Operator ";
  s.append(def->label()).append(" Information");
  this->setWindowTitle(s.c_str());
  s = "<table border=\"1\" cellpadding=\"10\"><tr><th>Parameter           </th><th>Description     "
      "              </th></tr>";
  std::size_t i, n = def->numberOfItemDefinitions(),
                 numItemsDisplayed = static_cast<std::size_t>(0);
  for (i = static_cast<std::size_t>(0); i < n; i++)
  {
    auto item = def->itemDefinition(static_cast<int>(i));
    std::string info =
      (item->detailedDescription() != "") ? item->detailedDescription() : item->briefDescription();
    if (info == "")
    {
      continue;
    }
    // For now we will only display advance level 0 and 1 (this way we can hide items used in the API
    // by setting their level > 1)
    if (item->advanceLevel() > 1)
    {
      continue;
    }
    else if (item->advanceLevel() == 1)
    {
      s.append("<tr bgcolor=\"pink\"><td><strong>");
    }
    else
    {
      s.append("<tr><td><strong>");
    }
    numItemsDisplayed++;
    s.append(item->label()).append("</strong></td><td>");
    s.append(info.c_str()).append("</td></tr>");
    //this->m_dialog->textBrowser->insertHtml(item->detailedDescription().c_str());
    //this->m_dialog->textBrowser->insertHtml("</td></tr>");
  }
  // Did we have any items displayed?
  if (numItemsDisplayed)
  {
    s.append("</table></body></html>");
    this->m_dialog->textBrowser->insertHtml("<br><h2>Parameter Descriptions</h2>");
    this->m_dialog->textBrowser->insertHtml(s.c_str());
  }
}
