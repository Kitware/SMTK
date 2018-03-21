//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "job.h"
#include "cJSON.h"

#include <QDebug>
#include <QString>

namespace cumulus
{

Job::Job()
{
}

Job::Job(const QString& id, const QString& name, const QString& status,
  const QList<QString>& outputFolderIds, const QString& machine)
{
  m_id = id;
  m_name = name;
  m_status = status;
  m_outputFolderIds = outputFolderIds;
  m_machine = machine;
  m_numberOfNodes = 0;
  m_numberOfCores = 0;
}

Job::Job(const Job& job)
{
  m_id = job.id();
  m_name = job.name();
  m_status = job.status();
  m_outputFolderIds = job.outputFolderIds();
  m_machine = job.machine();
  m_notes = job.notes();
  m_numberOfNodes = job.numberOfNodes();
  m_numberOfCores = job.numberOfCores();
  m_start = job.started();
  m_finish = job.finished();
  m_downloadFolder = job.downloadFolder();
}

Job::~Job()
{
}

void Job::setStatus(const QString& status)
{
  m_status = status;

  // Return if finish time already set
  if (m_finish.isValid())
  {
    return;
  }

  // Check if status is terminal
  if ((status == "terminated") || (status == "unexpectederror") || (status == "error") ||
    (status == "complete"))
  {
    m_finish = QDateTime::currentDateTime();
    qDebug() << "Set finish, timestamp:" << m_finish.toSecsSinceEpoch();
  }
}

void Job::setDownloadFolder(const QString& path)
{
  //qDebug() << "Setting download folder:" << path;
  m_downloadFolder = path;
}

Job Job::fromJSON(cJSON* obj)
{
  cJSON* idItem = cJSON_GetObjectItem(obj, "_id");
  if (!idItem || idItem->type != cJSON_String)
  {
    return Job();
  }
  QString id(idItem->valuestring);

  cJSON* nameItem = cJSON_GetObjectItem(obj, "name");
  if (!nameItem || nameItem->type != cJSON_String)
  {
    return Job();
  }
  QString name(nameItem->valuestring);

  cJSON* statusItem = cJSON_GetObjectItem(obj, "status");
  if (!statusItem || statusItem->type != cJSON_String)
  {
    return Job();
  }
  QString status(statusItem->valuestring);

  QList<QString> outputFolderIds;
  cJSON* outputItem = cJSON_GetObjectItem(obj, "output");
  if (outputItem && outputItem->type == cJSON_Array)
  {

    for (cJSON* output = outputItem->child; output; output = output->next)
    {
      cJSON* folderIdItem = cJSON_GetObjectItem(output, "folderId");
      if (!folderIdItem || folderIdItem->type != cJSON_String)
      {
        continue;
      }

      outputFolderIds.append(QString(folderIdItem->valuestring));
    }
  }

  QString machine;
  cJSON* paramsItem = cJSON_GetObjectItem(obj, "params");
  if (paramsItem && paramsItem->type == cJSON_Object)
  {
    cJSON* machineItem = cJSON_GetObjectItem(paramsItem, "machine");
    if (machineItem && machineItem->type == cJSON_String)
    {
      machine = QString(machineItem->valuestring);
    }
  }

  Job newJob(id, name, status, outputFolderIds, machine);

  // Check for optional "cmb" items, which are stored as job "metadata"
  cJSON* cmbItem = cJSON_GetObjectItem(obj, "metadata");
  if (cmbItem && cmbItem->type == cJSON_Object)
  {
    cJSON* notesItem = cJSON_GetObjectItem(cmbItem, "notes");
    if (notesItem && notesItem->type == cJSON_String)
    {
      newJob.m_notes = QString(notesItem->valuestring);
    } // end if (notesItem)

    cJSON* nodesItem = cJSON_GetObjectItem(cmbItem, "numberOfNodes");
    if (nodesItem && nodesItem->type == cJSON_Number)
    {
      newJob.m_numberOfNodes = nodesItem->valueint;
    } // end if (nodesItem)

    cJSON* coresItem = cJSON_GetObjectItem(cmbItem, "numberOfCores");
    if (coresItem && coresItem->type == cJSON_Number)
    {
      newJob.m_numberOfCores = coresItem->valueint;
    } // end if (nodesItem)

    cJSON* startItem = cJSON_GetObjectItem(cmbItem, "startTimeStamp");
    if (startItem && startItem->type == cJSON_Number)
    {
      double startTimestamp = startItem->valuedouble;
      qint64 startInt = static_cast<qint64>(startTimestamp);
      newJob.m_start.setSecsSinceEpoch(startInt);
      //qDebug() << "started:" << newJob.m_start;
    } // end if (startItem)

    cJSON* finishedItem = cJSON_GetObjectItem(cmbItem, "finishTimeStamp");
    if (finishedItem && finishedItem->type == cJSON_Number)
    {
      double finishedTimestamp = finishedItem->valuedouble;
      qint64 finishedInt = static_cast<qint64>(finishedTimestamp);
      newJob.m_finish.setSecsSinceEpoch(finishedInt);
      //qDebug() << "finished:" << newJob.m_finish;
    } // end if (finishedItem)

    cJSON* folderItem = cJSON_GetObjectItem(cmbItem, "downloadFolder");
    if (folderItem && folderItem->type == cJSON_String)
    {
      //qDebug() << "Download folder:" << folderItem->valuestring;
      newJob.m_downloadFolder = QString(folderItem->valuestring);
    } // end if (notesItem)
  }   // if (cmbItem)

  return newJob;
}

cJSON* Job::cmbDataToJSON() const
{
  cJSON* cmbObject = cJSON_CreateObject();
  cJSON_AddStringToObject(cmbObject, "notes", m_notes.toLatin1().constData());
  cJSON_AddNumberToObject(cmbObject, "numberOfNodes", m_numberOfNodes);
  cJSON_AddNumberToObject(cmbObject, "numberOfCores", m_numberOfCores);
  cJSON_AddNumberToObject(cmbObject, "startTimeStamp", m_start.toSecsSinceEpoch());
  cJSON_AddNumberToObject(cmbObject, "finishTimeStamp", m_finish.toSecsSinceEpoch());
  if (!m_downloadFolder.isEmpty())
  {
    cJSON_AddStringToObject(cmbObject, "downloadFolder", m_downloadFolder.toLatin1().constData());
  }
  return cmbObject;
}

} // end namespace
