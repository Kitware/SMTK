//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef SHIBOKEN_SKIP
#include "smtk/bridge/remote/RemusConnection.h"
#include "smtk/bridge/remote/Session.h"

#include "smtk/AutoInit.h"

#include "smtk/model/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/common/UUID.h"

#include <fstream>

using namespace smtk::model;
using namespace smtk::bridge::remote;

#ifdef SMTK_BUILD_CGM
smtkComponentInitMacro(smtk_cgm_session);
#endif

int main(int argc, char* argv[])
{
  if (argc < 3)
    {
    std::cout
      << "Usage:\n"
      << "  " << argv[0] << " filename hint [remusHost [remusPort]]\n"
      << "where\n"
      << "  filename     is the path to a model file to read.\n"
      << "  hint         is the name of an SMTK session or worker.\n"
      << "  remusHost    is either \"local\" or a hostname where a\n"
      << "               remus server is running. \"local\" is default.\n"
      << "  remusPort    is the port number of the remus server.\n"
      << "               The default port is 50505.\n"
      ;
    return 1;
    }

  std::string fileName(argv[1]);
  std::string sessionName(argc > 2 ? argv[2] : "");
  std::string hostname(argc > 3 ? argv[3] : "local");
  int port(argc > 4 ? atoi(argv[4]) : 50505);

  smtk::model::Manager::Ptr mgr = smtk::model::Manager::create();
  mgr->log().setFlushToStream(
    new std::ofstream("/tmp/unitRemusSession.log"), true, false);
  RemusConnection::Ptr bconn =
    RemusConnection::create();
  bconn->setModelManager(mgr);
  // Do not search for workers in default paths; we don't want to pick things up by accident:
  bconn->clearSearchDirs(true);
  if (argc > 5)
     bconn->addSearchDir(argv[5]);

  bool didConnect = true;
  if (hostname != "local")
    didConnect = bconn->connectToServer(hostname, port);
  else
    didConnect = bconn->connectToServer();
  if (!didConnect)
    {
    std::cerr << "Could not connect to tcp://" << hostname << ":" << port << "\n";
    return 2;
    }

  std::vector<std::string> opnames;
  std::vector<std::string> bnames = bconn->sessionNames();
  std::vector<std::string>::const_iterator strit;
  std::cout << "Session names available:\n";
  for (strit = bnames.begin(); strit != bnames.end(); ++strit)
    std::cout << "  " << *strit << "\n";

  std::set<std::string> bnameset(bnames.begin(), bnames.end());
  if (bnameset.find(sessionName) == bnameset.end())
    {
    std::cerr << "Session \"" << sessionName << "\" not available.\n";
    return 4;
    }

  opnames = bconn->operatorNames(sessionName);
  std::cout << "Operators for session \"" << sessionName << "\":\n";
  for (strit = opnames.begin(); strit != opnames.end(); ++strit)
    std::cout << "  " << *strit << "\n";

  StringData fileTypes = bconn->supportedFileTypes(sessionName);
  StringData::const_iterator sdit;
  std::cout << "File types available for \"" << sessionName << "\":\n";
  for (sdit = fileTypes.begin(); sdit != fileTypes.end(); ++sdit)
    {
    std::cout << "  " << sdit->first << ":\n";
    for (strit = sdit->second.begin(); strit != sdit->second.end(); ++strit)
      std::cout << "    " << *strit << "\n";
    }

  smtk::common::UUID sessionSessionId = bconn->beginSession(sessionName);
  if (sessionSessionId.isNull())
    {
    std::cerr << "Null session session ID.\n";
    return 8;
    }
  std::cout << "Started session " << sessionSessionId.toString() << "\n";

  smtk::model::OperatorResult readResult = bconn->readFile(fileName, "", sessionName);
  std::string strout;
  switch (readResult->findInt("outcome")->value())
    {
  case smtk::model::UNABLE_TO_OPERATE:   strout = "unable to operate"; break;
  case smtk::model::OPERATION_CANCELED:  strout = "canceled"; break;
  case smtk::model::OPERATION_FAILED:    strout = "failed"; break;
  case smtk::model::OPERATION_SUCCEEDED: strout = "succeeded"; break;
  case smtk::model::OUTCOME_UNKNOWN:
  default:
    strout = "unknown"; break;
    }
  std::cout << "Read file? " << strout << " (" << readResult->findInt("outcome")->value() << ")\n";
  std::cout << "Output model is " << readResult->findModelEntity("entities")->value() << "\n";

  opnames = bconn->operatorNames(sessionSessionId);
  std::cout << "Operators for session \"" << sessionSessionId << "\":\n";
  for (strit = opnames.begin(); strit != opnames.end(); ++strit)
    std::cout << "  " << *strit << "\n";

  bool didEnd = bconn->endSession(sessionSessionId);
  std::cout << "Ended session " << sessionSessionId.toString() << ": " << (didEnd ? "Y" : "N") << "\n";

  return 0;
}
#endif // SHIBOKEN_SKIP
