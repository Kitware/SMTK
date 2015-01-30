//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/Options.h"
#include "smtk/AutoInit.h"

#include "smtk/io/ExportJSON.h"
#include "smtk/io/ImportJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "cJSON.h"

#include "unitForwardingOperator_xml.h"

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::io;

using smtk::attribute::IntItem;

namespace {

template<class T>
void printItem(typename T::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::size_t ne = item->numberOfValues();
  std::cout << " " << tname << " (" << ne << ") [";
  for (std::size_t e = 0; e < ne; ++e)
    {
    std::cout << " " << item->value(e);
    }
  std::cout << " ]";
}

template<>
void printItem<smtk::attribute::GroupItem>(smtk::attribute::GroupItem::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::cout << " " << tname << " ";
}

template<>
void printItem<smtk::attribute::VoidItem>(smtk::attribute::VoidItem::Ptr item, const std::string& tname)
{
  std::cout << (item->name().empty() ? "(null)" : item->name()) << ":";
  std::cout << " " << tname << " ";
}

} // anonymous namespace

#define smtkAttributeItemTypeCase(ENUM, CLASSNAME, ENTRYTYPE, ITEM, CODE) \
  case smtk::attribute::Item:: ENUM : \
    { \
    smtk::attribute:: CLASSNAME ::Ptr typedItem = \
      smtk::dynamic_pointer_cast<smtk::attribute:: CLASSNAME >( ITEM ); \
    std::string enumName( # ENUM ); \
    typedef smtk::attribute:: CLASSNAME ItemTType; \
    if (typedItem) \
      { \
      CODE; \
      } \
    } \
    break

#define smtkAttributeItemTypeSwitch(CODE) \
    smtkAttributeItemTypeCase(INT,IntItem,int,item,CODE); \
    smtkAttributeItemTypeCase(DOUBLE,DoubleItem,double,item,CODE); \
    smtkAttributeItemTypeCase(STRING,StringItem,std::string,item,CODE); \
    smtkAttributeItemTypeCase(FILE,StringItem,std::string,item,CODE); \
    smtkAttributeItemTypeCase(DIRECTORY,StringItem,std::string,item,CODE); \
    smtkAttributeItemTypeCase(ATTRIBUTE_REF,RefItem,smtk::attribute::AttributePtr,item,CODE); \
    smtkAttributeItemTypeCase(MODEL_ENTITY,ModelEntityItem,smtk::model::EntityRef,item,CODE); \
    smtkAttributeItemTypeCase(GROUP,GroupItem,smtk::attribute::ItemPtr,item,CODE); \
    smtkAttributeItemTypeCase(VOID,VoidItem,void,item,CODE); \
    default: \
     break; \

static void printParams(smtk::attribute::AttributePtr attr, const std::string& msg)
{
  std::cout << msg << "\n";
  if (!attr)
    return;
  std::size_t ni = attr->numberOfItems();
  for (std::size_t i = 0; i < ni; ++i)
    {
    std::cout << "  ";
    smtk::attribute::ItemPtr item = attr->item(i);
    switch (item->type())
      {
      smtkAttributeItemTypeSwitch(printItem<ItemTType>(typedItem, enumName));
      }
    std::cout << "\n";
    }
}

// Implement a forwarding session that does no communication.
// Instead, it just copies operator parameters _to_ a "remote"
// session and results back _from_ the "remote" session.
class TestForwardingSession : public smtk::model::DefaultSession
{
public:
  smtkTypeMacro(TestForwardingSession); // Provides typedefs for Ptr, SelfType, ...
  smtkSuperclassMacro(smtk::model::DefaultSession); // Provides typedefs for Superclass, ...
  smtkCreateMacro(TestForwardingSession); // Provides static create() method
  smtkSharedFromThisMacro(Session); // Provides shared_from_this() method
  smtkDeclareModelingKernel(); // Declares name() and utility methods/members.

  smtk::model::Manager::Ptr remoteModel;
  smtk::model::Session::Ptr remoteSession;

  void addSomeRemoteDanglers(const UUIDs& danglers)
    {
    UUIDs::const_iterator it;
    for (it = danglers.begin(); it != danglers.end(); ++it)
      {
      this->remoteSession->declareDanglingEntity(
        smtk::model::EntityRef(this->remoteModel, *it),
        smtk::model::SESSION_NOTHING);
      }
    }

  bool checkLocalDanglers(const UUIDs& danglers, smtk::model::ManagerPtr modelMgr)
    {
    UUIDs::const_iterator it;
    for (it = danglers.begin(); it != danglers.end(); ++it)
      {
      smtk::model::DanglingEntities::const_iterator cit =
        this->m_dangling.find(
          smtk::model::EntityRef(modelMgr, *it));
      if (cit == this->m_dangling.end())
        return false;
      }
    return true;
    }

protected:
  TestForwardingSession()
    {
    this->initializeOperatorSystem(TestForwardingSession::s_operators);
    }

  virtual SessiondInfoBits transcribeInternal(const EntityRef& entity, SessiondInfoBits flags)
    {
    return remoteSession->transcribe(EntityRef(remoteModel, entity.entity()), flags);
    }

  virtual bool ableToOperateDelegate(RemoteOperatorPtr oper)
    {
    OperatorPtr remOp = remoteSession->op(oper->name());
    remOp->setSpecification(remoteSession->operatorSystem()->copyAttribute(oper->specification()));
    return remOp->ableToOperate();
    }

  virtual OperatorResult operateDelegate(RemoteOperatorPtr localOp)
    {
    printParams(localOp->specification(), "local input");
    OperatorPtr remOp = remoteSession->op(localOp->name());
    remOp->setSpecification(remoteSession->operatorSystem()->copyAttribute(localOp->specification()));
    OperatorResult remResult = remOp->operate();
    OperatorResult localResult = this->operatorSystem()->copyAttribute(remResult);

    // Kill remote operator and result.
    remOp->session()->operatorSystem()->removeAttribute(remOp->specification());
    remResult->system()->removeAttribute(remResult);

    printParams(localResult, "local output");
    return localResult;
    }
};
smtkImplementsModelingKernel(
  forwarding,
  "{\"kernel\":\"test-forwarding\", \"engines\":[]}",
  SessionHasNoStaticSetup,
  TestForwardingSession,
  false /* forwarding session should not inherit local operators */
);

class TestForwardingOperator : public Operator
{
public:
  smtkTypeMacro(TestForwardingOperator);
  smtkCreateMacro(TestForwardingOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();


  virtual bool ableToOperate()
    {
    if (!this->specification()->isValid())
      return false;

    if (!this->specification()->find("addToCount")->isEnabled())
      {
      return false;
      }

    return true;
    }

protected:
  TestForwardingOperator() { }

  virtual OperatorResult operateInternal()
    {
    printParams(this->specification(), "actual input");

    OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
    this->s_state += this->specification()->findInt("addToCount")->value();
    result->findInt("state")->setValue(this->s_state);

    printParams(result, "actual output");
    return result;
    }

  // Note that any state to be preserved between
  // invocations of the operator must be kept in
  // smtk::model::Manager or class static. This is
  // because the operator is created by the session
  // each time it is requested.
  static Integer s_state;
};
smtkImplementsModelOperator(
  TestForwardingOperator, forwarding,
  "forwarding operator", unitForwardingOperator_xml,
  smtk::model::DefaultSession); //TestForwardingSession);

Integer TestForwardingOperator::s_state = 1;

void printSessionOperatorNames(const SessionRef& br, const std::string& msg)
{
  StringList opNames = br.operatorNames();
  StringList::const_iterator it;
  std::cout << "Session \"" << br.name() << "\" [" << br.session()->className() << ", " << msg << "] operators:\n";
  for (it = opNames.begin(); it != opNames.end(); ++it)
    {
    smtk::model::OperatorPtr op = br.op(*it);
    std::cout
      << "  " << *it
      << " [" << op->className() << "]"
      << "\n";
    }
  std::cout << "\n";
}

// Test remote bridging: create 2 model::Manager instances,
// add a native operator to manager A's "native" session,
// serialize the session session into a DefaultSession instance
// that backs it into manager B, and invoke the remote version
// of the operator attached to the DefaultSession on manager B.
// Check that the operation was invoked on manager A and that
// the OperatorResult from both operations are identical (by
// having the native operator cache its result parameters).
int main()
{
  int status = 0;

  try {

    // Create the managers
    smtk::model::Manager::Ptr remoteMgr = smtk::model::Manager::create();
    smtk::model::Manager::Ptr localMgr = smtk::model::Manager::create();

    // The default session of the "remote" manager:
    Session::Ptr remoteSession = remoteMgr->createAndRegisterSession("native");
    SessionRef remoteSess(remoteMgr, remoteSession->sessionId());
    remoteSess.setName("remote session");
    printSessionOperatorNames(remoteSess, "remote");

    // Now we want to mirror the remote manager locally.
    // Serialize the "remote" session session:
    cJSON* sessJSON = cJSON_CreateObject();
    ExportJSON::forManagerSession(remoteSession->sessionId(), sessJSON, remoteMgr);
    // ... and import the session locally to a new session object.
    TestForwardingSession::Ptr localSession = TestForwardingSession::create();
    localSession->remoteSession = remoteSession;
    localSession->remoteModel = remoteMgr;
    localMgr->registerSession(localSession);
    SessionRef localSess(localMgr, localSession->sessionId());
    test(localSession->operatorNames().size() == 0, "Forwarding session should have no operators by default.");
    printSessionOperatorNames(localSess, "local, pre-import");
    ImportJSON::ofRemoteSession(sessJSON->child, localSession, localMgr);
    printSessionOperatorNames(localSess, "local, post-import");
    test(localSession->operatorNames().size() == remoteSession->operatorNames().size(),
      "Forwarding session operator count should match remote session after import.");

    // Run the local operator.
    // Examine the remote version to verify the operation was forwarded.
    OperatorPtr localOp = localSession->op("forwarding operator");
    test(
      smtk::dynamic_pointer_cast<smtk::model::RemoteOperator>(localOp) ?
      true : false, "Local forwarding operator was not an instance of RemoteOperator.");
    test(
      localOp->className() == "smtk::model::RemoteOperator",
      "Local operator did not return expected className() \"smtk::model::RemoteOperator\"");
    localOp->ensureSpecification();
    test(
      !!localOp->specification(),
      "Local operator should have a default specification, not a null pointer.");
    localOp->findInt("addToCount")->setValue(1);
    localOp->findAs<IntItem>("addToCount")->setValue(2);
    test(
      localOp->specification()->findInt("addToCount")->value() == 2,
      "Setting a valid parameter had no effect.");
    OperatorResult localResult = localOp->operate();

    test(localResult->findInt("outcome")->value() == OPERATION_SUCCEEDED, "Operation should have succeeded.");
    test(localResult->findInt("state")->value() == 3, "Operation should have yielded state == 3.");
    localOp->eraseResult(localResult);

    std::cout << "\n---\n\n";

    // Rerun the local operator.
    localOp->specification()->findInt("addToCount")->setValue(8);
    localResult = localOp->operate();

    test(localResult->findInt("outcome")->value() == OPERATION_SUCCEEDED, "Operation should have succeeded.");
    test(localResult->findInt("state")->value() == 11, "Operation should have yielded state == 11.");

    // Rerun the local operator but with an "improper" input (disabling a parameter that really
    // shouldn't be optional is a surrogate for situations where an operator may be unable to run)
    // This also tests the setParameterEnabled method.
    test(localOp->specification()->find("addToCount")->isOptional(), "Expected optional \"addToCount\".");

    localOp->specification()->findInt("addToCount")->setIsEnabled(false);
    test(!localOp->specification()->find("addToCount")->isEnabled(), "Did not disable \"addToCount\".");
    localResult = localOp->operate();

    test(localResult->findInt("outcome")->value() == UNABLE_TO_OPERATE, "Operator should have been unable to execute.");

    // Test transferring remote dangling entity list to local session.
    // (As a preentityref to a full implementation of TestForwardingSession::transcribeInternal)
    UUIDGenerator ugen;
    UUIDs danglers;
    for (int i = 0; i < 8; ++i)
      danglers.insert(ugen.random());
    localSession->addSomeRemoteDanglers(danglers);
    cJSON* jsonDanglers = cJSON_CreateObject();
    smtk::io::ExportJSON::forDanglingEntities(remoteSession->sessionId(), jsonDanglers, remoteMgr);
    //std::cout << "\n\n\n" << cJSON_Print(jsonDanglers) << "\n\n\n";
    smtk::io::ImportJSON::ofDanglingEntities(jsonDanglers, localMgr);
    test(localSession->checkLocalDanglers(danglers, localMgr), "All generated danglers should have been serialized.");

  } catch (const std::string& msg) {
    (void) msg; // Ignore the message; it's already been printed.
    std::cerr << "Exiting...\n";
    status = -1;
  }

  return status;
}
