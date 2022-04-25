//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtTimeZoneRegionModel.h"
#include "smtk/common/timezonespec.h"

#include <QDebug>
#include <QList>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QTextStream>
#include <QVector>
#include <QtGlobal>

#include <cassert>
#include <sstream>

// This a read-only model of a two-level tree.
// Top level is "continent"
// Second level is "timezone"
// Each entry is assigned an integer value. Continents are numbered
// starting at 1, and timezones are numbered starting at the continent
// number * 1000 plus 1.

using namespace smtk::extension;

namespace
{
struct TimeZoneEntry
{
  int InternalId;
  QString ID;
  QString Continent;
  QString Region;
  QString Offset;
  QString DSTAdjust;
  QString Abbrev;
  QString DSTAbbrev;
};
} // namespace

class qtTimeZoneRegionModel::TimeZoneRegionModelInternal
{
public:
  QList<QString> ContinentList;
  QList<int> ZoneCountList;
  QMap<int, TimeZoneEntry> ZoneMap;
};

qtTimeZoneRegionModel::qtTimeZoneRegionModel(QObject* parent)
  : QAbstractItemModel(parent)
{
  this->Internal = new qtTimeZoneRegionModel::TimeZoneRegionModelInternal;
}

qtTimeZoneRegionModel::~qtTimeZoneRegionModel()
{
  delete this->Internal;
}

void qtTimeZoneRegionModel::initialize()
{
  if (!this->Internal->ZoneMap.isEmpty())
  {
    qWarning() << "Cannot re-initialize qtTimeZoneRegionModel";
    return;
  }

  // Parse timezonespec_csv string
  // First 7 columns of each line are:
  // "ID","STD ABBR","STD NAME","DST ABBR","DST NAME","GMT offset","DST adjustment"
  // And "ID" == "Continent/Region"

  QSet<QString> continentSet;
  QString tzSpec(smtk::common::timezonespec_csv);
  QTextStream tzStream(&tzSpec);
  int continentId = 0;
  int regionId = 0;
  QString line = tzStream.readLine(); // Skip header line
  QStringList parts;
  for (line = tzStream.readLine(); !line.isNull(); line = tzStream.readLine())
  {
    QStringList rawParts = line.split(',');
    // Strip quotes from each item
    parts.clear();
    Q_FOREACH (QString part, rawParts)
    {
      int first = 1;
      int length = part.length() - 2;
      QString newPart = part.mid(first, length);
      parts.append(newPart);
    }

    QStringList idParts = parts[0].split('/');
    QString continent = idParts[0];
    if (!continentSet.contains(continent))
    {
      continentSet.insert(continent);
      continentId = continentSet.size();
      this->Internal->ContinentList.append(continent);
      this->Internal->ZoneCountList.append(0);
      regionId = 1000 * continentId;
    }

    ++regionId;
    int row = continentId - 1;
    this->Internal->ZoneCountList[row] += 1;

    TimeZoneEntry entry;
    entry.InternalId = regionId;
    entry.ID = parts[0];
    entry.Continent = idParts[0];
    entry.Region = idParts[1];
    entry.Offset = parts[5].left(6);    // +HH:MM
    entry.DSTAdjust = parts[6].left(6); // +HH:MM
    entry.Abbrev = parts[1];
    entry.DSTAbbrev = parts[3];
    this->Internal->ZoneMap.insert(regionId, entry);
  }
  qDebug() << "Continent List size" << this->Internal->ContinentList.size();
  qDebug() << "Zone Map size" << this->Internal->ZoneMap.size();
}

QModelIndex qtTimeZoneRegionModel::index(int row, int column, const QModelIndex& parent) const
{
  int internalId = 0;
  if (!parent.isValid())
  {
    return this->createIndex(row, column, internalId);
  }

  int parentId = parent.internalId();
  if (0 == parentId)
  {
    internalId = row + 1; // continent
  }
  else
  {
    internalId = 1000 * parentId + row + 1; // region
  }
  return this->createIndex(row, column, internalId);
}

QModelIndex qtTimeZoneRegionModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return QModelIndex();
  }

  int internalId = index.internalId();
  if (0 == internalId)
  {
    return QModelIndex();
  }
  else if (internalId < 1000)
  {
    return this->createIndex(0, 0); // continent's parent is root
  }
  // (else region)
  int parentId = internalId / 1000;
  int parentRow = parentId - 1;
  return this->createIndex(parentRow, 0, parentId);
}

int qtTimeZoneRegionModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
  {
    return 1;
  }

  int parentId = parent.internalId();
  if (0 == parentId)
  {
    return this->Internal->ContinentList.size();
  }
  else if (parentId < 1000)
  {
    int row = parentId - 1;
    return this->Internal->ZoneCountList.at(row);
  }
  // (else region)
  return 0;
}

int qtTimeZoneRegionModel::columnCount(const QModelIndex& parent) const
{
  if (!parent.isValid())
  {
    // Return max number of columns used in the model
    return 3;
  }

  int parentId = parent.internalId();
  if (0 == parentId)
  {
    return 1;
  }
  else if (parentId < 1000)
  {
    return 3;
  }
  // (else error)
  return 0;
}

QVariant qtTimeZoneRegionModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    qWarning() << "Invalid Index";
    return QVariant();
  }

  QString result;
  int internalId = index.internalId();

  // Special case for centering offset/dst column for regions
  if ((Qt::TextAlignmentRole == role) && (internalId > 1000) && (1 == index.column()))
  {
    return QVariant(Qt::AlignHCenter);
  }

  if (Qt::DisplayRole != role)
  {
    return QVariant();
  }

  if (0 == internalId)
  {
    return "Root";
  }
  else if (internalId < 1000)
  {
    if (index.column() > 0)
    {
      return "";
    }
    int row = internalId - 1;
    QString continent = this->Internal->ContinentList[row];
    int zoneCount = this->Internal->ZoneCountList[row];
    QTextStream(&result) << continent << "  (" << zoneCount << ")";
    return result;
  }

  // (else) Zone has 3 columns
  assert(this->Internal->ZoneMap.contains(internalId));
  TimeZoneEntry entry = this->Internal->ZoneMap.value(internalId);
  switch (index.column())
  {
    case 0:
      result = entry.Region;
      break;

    case 1:
      result = entry.Offset;
      if (!entry.DSTAdjust.isEmpty())
      {
        result += '/';
        result += entry.DSTAdjust;
      }
      break;

    case 2:
      result = entry.Abbrev;
      if (!entry.DSTAbbrev.isEmpty())
      {
        result += '/';
        result += entry.DSTAbbrev;
      }
      break;

    default:
      return "Unsupported column number";
      break;
  }
  return result;
}

QVariant qtTimeZoneRegionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((Qt::Horizontal == orientation) && (Qt::DisplayRole == role))
  {
    return "Region";
  }

  // (else)
  return QAbstractItemModel::headerData(section, orientation, role);
}

QString qtTimeZoneRegionModel::regionId(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return QString();
  }

  int internalId = index.internalId();
  const TimeZoneEntry entry = this->Internal->ZoneMap.value(internalId);
  return entry.ID;
}

QModelIndex qtTimeZoneRegionModel::findModelIndex(const QString& regionId) const
{
  if (regionId.isEmpty())
  {
    return QModelIndex(); // safety first
  }

  QStringList parts = regionId.split('/');
  QString continent = parts[0];
  QString region = parts[1];
  QModelIndex rootIndex = this->createIndex(0, 0);

  // Find continent index by brute force
  int contRow = 0;
  Q_FOREACH (QString name, this->Internal->ContinentList)
  {
    if (name == continent)
    {
      break;
    }
    ++contRow;
  }

  if (contRow >= this->Internal->ContinentList.size())
  {
    qWarning() << "Did not find continent: " << continent;
    return QModelIndex();
  }
  QModelIndex contIndex = this->index(contRow, 0, rootIndex);
  qDebug() << "contRow" << contRow << "contIndex" << contIndex;

  // Find zone by brute force
  QMap<int, TimeZoneEntry>::const_iterator regionIter = this->Internal->ZoneMap.constBegin();
  for (; regionIter != this->Internal->ZoneMap.constEnd(); ++regionIter)
  {
    TimeZoneEntry entry = regionIter.value();
    if (entry.Region == region)
    {
      int regionRow = (entry.InternalId % 1000) - 1;
      QModelIndex regionIndex = this->index(regionRow, 0, contIndex);
      return regionIndex;
    }
  }

  qWarning() << "Did not find continent/region" << continent << region;
  return QModelIndex();
}
