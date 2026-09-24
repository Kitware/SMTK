//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtLogView.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QLayout>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

#include <cstdint>

namespace smtk
{
namespace qt
{

qtLogView::qtLogView(const std::filesystem::path& logPath, QWidget* parent)
  : QDialog(parent)
  , m_path(logPath)
  , m_file(m_path, std::ios::binary)
{
  m_contents = new QPlainTextEdit(this);
  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  QFontMetricsF fontMetrics(fixedFont);
  m_contents->document()->setDefaultFont(fixedFont);
  m_contents->setCenterOnScroll(false);
  m_contents->setLineWrapMode(QPlainTextEdit::NoWrap);
  m_contents->setReadOnly(true);
  // m_contents->setSizeHint(80 * fontMetrics.averageCharWidth(), 20 * fontMetrics.height());
  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
  m_updateTimer = new QTimer(this);
  m_updateTimer->setInterval(100 /*ms*/);
  m_updateTimer->setSingleShot(false);
  auto* layout = new QVBoxLayout;
  layout->addWidget(m_contents);
  layout->addWidget(m_buttonBox);
  this->setLayout(layout);
  QObject::connect(m_buttonBox, &QDialogButtonBox::accepted, this, &qtLogView::done);
  QObject::connect(m_updateTimer, &QTimer::timeout, this, &qtLogView::readMore);
  // this->readMore();
  m_updateTimer->start();
}

QSize qtLogView::sizeHint() const
{
  QFontMetricsF fontMetrics(m_contents->document()->defaultFont());
  QSize result(80 * fontMetrics.averageCharWidth(), 20 * fontMetrics.height());
  return result;
}

void qtLogView::readMore()
{
  if (!m_file.is_open())
  {
    // Try to re-open
    m_contents->clear();
    m_lastRead = 0;
    if (std::filesystem::exists(m_path))
    {
      m_file = std::ifstream(m_path, std::ios::binary);
    }
  }
  if (!m_file.is_open())
  {
    // Wait for the timer before trying to reopen
    return;
  }
  auto size = std::filesystem::file_size(m_path);
  if (size < static_cast<std::uintmax_t>(m_lastRead))
  {
    // Assume file has been truncated.
    // Re-read the entire file.
    m_lastRead = 0;
    m_contents->clear();
  }
  else if (size == static_cast<std::uintmax_t>(m_lastRead))
  {
    return;
  }
  // File sizes and offsets are byte counts, so reads must use binary mode
  // (Windows text mode translates CRLF). A short read must not prevent the
  // next timer tick from reading data appended after reaching EOF.
  m_file.clear();
  m_file.seekg(m_lastRead, std::ios_base::beg);
  std::string buf(size - m_lastRead, '\0');
  m_file.read(buf.data(), size - m_lastRead);
  buf.resize(static_cast<std::size_t>(m_file.gcount()));
  m_lastRead += m_file.gcount();
  if (buf.empty())
  {
    return;
  }
  m_contents->appendPlainText(QString::fromStdString(buf));
  QTextCursor cursor(m_contents->document()->lastBlock());
  m_contents->setTextCursor(cursor);
}

void qtLogView::done()
{
  m_updateTimer->stop();
  this->accept();
}

} // namespace qt
} // namespace smtk
