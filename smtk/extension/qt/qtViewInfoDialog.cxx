//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtViewInfoDialog - A Information Dialog for SMTK Operations
// .SECTION Description
// .SECTION Caveats

#include "qtViewInfoDialog.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "ui_qtViewInfoDialog.h"

#include "smtk/view/Configuration.h"

#include <QTextStream>

using namespace smtk::extension;

qtViewInfoDialog::qtViewInfoDialog(QWidget* Parent)
  : QDialog(Parent)
  , m_dialog(new Ui::qtViewInfoDialog())
{
  m_dialog->setupUi(this);
  this->setObjectName("qtViewInfoDialog");
}

qtViewInfoDialog::~qtViewInfoDialog()
{
  delete m_dialog;
}

void qtViewInfoDialog::displayInfo(smtk::view::ConfigurationPtr view)
{
  std::string s;
  m_view = view;
  m_dialog->textBrowser->clear();
  s = m_view->label();
  s.append(" Information");
  this->setWindowTitle(s.c_str());

  s = "<!DOCTYPE html><html><head><title>";
  s.append(m_view->label()).append("</title></head><body>");
  m_dialog->textBrowser->insertHtml(s.c_str());

  s = "<h2>";
  s.append(m_view->label()).append("  Information</h2>");
  m_dialog->textBrowser->insertHtml(s.c_str());
  m_dialog->textBrowser->insertHtml("<hr size=\"1\" /><br>");

  // Find the description if there is one
  int index = m_view->details().findChild("Description");
  if (index < 0)
  {
    m_dialog->textBrowser->insertHtml("No Information Avaliable");
  }
  else
  {
    s = m_view->details().child(index).contents();
    m_dialog->textBrowser->insertHtml(s.c_str());
  }
}

void qtViewInfoDialog::displayInfo(smtk::attribute::AttributePtr att)
{
  m_attribute = att;
  m_dialog->textBrowser->clear();

  QString title = att->definition()->label().c_str() + QString(" Information");
  this->setWindowTitle(title);

  QString html;
  qtViewInfoDialog::formatInfoHtml(att, html);
  m_dialog->textBrowser->insertHtml(html);
  m_dialog->textBrowser->moveCursor(QTextCursor::Start);
}

void qtViewInfoDialog::formatInfoHtml(smtk::attribute::AttributePtr att, QString& html)
{
  if (!att)
  {
    html = "<b>Missing Specification</b>\nNo Information Available.";
    return;
  }
  smtk::attribute::DefinitionPtr def = att->definition();

  html = "";
  QTextStream qs(&html);
  qs << "<!DOCTYPE html><html><head>\n"
     << "<title>" << att->definition()->label().c_str() << "</title>\n"
     << "</head>\n"
     << "<body>\n"
     << "<h2>" << def->label().c_str() << "  Information</h2>\n"

     << "<hr size=\"1\" /><br>\n";

  std::string desc = def->detailedDescription();
  if (desc.empty())
  {
    desc = def->briefDescription();
  }
  if (!desc.empty())
  {
    qs << desc.c_str() << "\n";
  }

  std::size_t i, n = def->numberOfItemDefinitions(),
                 numItemsDisplayed = static_cast<std::size_t>(0);
  for (i = static_cast<std::size_t>(0); i < n; i++)
  {
    auto item = def->itemDefinition(static_cast<int>(i));
    std::string info =
      !item->detailedDescription().empty() ? item->detailedDescription() : item->briefDescription();
    if (info.empty())
    {
      continue;
    }
    // For now we will only display advance level 0 and 1 (this way we can hide items used in the API
    // by setting their level > 1)
    if (item->advanceLevel() > 1)
    {
      continue;
    }

    if (numItemsDisplayed == 0)
    {
      // Begin table
      qs << "<br><h2>Parameter Descriptions</h2>\n"
         << "<table border=\"1\" cellpadding=\"10\">\n"
         << "<tr><th>Parameter</th><th>Description</th></tr>\n";
    }
    numItemsDisplayed++;

    QString style = item->advanceLevel() == 1 ? " bgcolor=\"pink\"" : "";
    qs << "<tr" << style << ">\n"
       << "<td><strong>" << item->label().c_str() << "</strong></td>\n"
       << "<td>" << info.c_str() << "</td>\n</tr>\n";
  }

  if (numItemsDisplayed)
  {
    qs << "</table>\n";
  }

  qs << "</body></html>\n";
}
