// $Id$

#include "Notify_Test_Client.h"

ACE_RCSID(Notify_Tests, Notify_Test_Client, "$Id$")

#define NOTIFY_FACTORY_NAME "NotifyEventChannelFactory"
#define NAMING_SERVICE_NAME "NameService"

Notify_Test_Client::Notify_Test_Client (void)
  : done_ (0)
{
  // @@ Later: accept the inter filter operator as a param.
  ifgop_ = CosNotifyChannelAdmin::OR_OP;
}

Notify_Test_Client::~Notify_Test_Client ()
{
}

void
Notify_Test_Client::init (int argc, char *argv [] TAO_ENV_ARG_DECL)
{
  this->init_ORB (argc, argv TAO_ENV_ARG_PARAMETER);
  ACE_CHECK;
  this->resolve_naming_service (TAO_ENV_SINGLE_ARG_PARAMETER);
  ACE_CHECK;
  this->resolve_Notify_factory (TAO_ENV_SINGLE_ARG_PARAMETER);
  ACE_CHECK;
}

void
Notify_Test_Client::init_ORB (int argc,
                              char *argv []
                              TAO_ENV_ARG_DECL)
{
  this->orb_ = CORBA::ORB_init (argc,
                                argv,
                                ""
                                TAO_ENV_ARG_PARAMETER);
  ACE_CHECK;

  CORBA::Object_ptr poa_object  =
    this->orb_->resolve_initial_references("RootPOA"
                                           TAO_ENV_ARG_PARAMETER);
  ACE_CHECK;

  if (CORBA::is_nil (poa_object))
    {
      ACE_ERROR ((LM_ERROR,
                  " (%P|%t) Unable to initialize the POA.\n"));
      return;
    }
  this->root_poa_ =
    PortableServer::POA::_narrow (poa_object TAO_ENV_ARG_PARAMETER);
  ACE_CHECK;

  PortableServer::POAManager_var poa_manager =
    root_poa_->the_POAManager (TAO_ENV_SINGLE_ARG_PARAMETER);
  ACE_CHECK;

  poa_manager->activate (TAO_ENV_SINGLE_ARG_PARAMETER);
  ACE_CHECK;
}

void
Notify_Test_Client::resolve_naming_service (TAO_ENV_SINGLE_ARG_DECL)
{
  CORBA::Object_var naming_obj =
    this->orb_->resolve_initial_references (NAMING_SERVICE_NAME
                                            TAO_ENV_ARG_PARAMETER);
  ACE_CHECK;

  // Need to check return value for errors.
  if (CORBA::is_nil (naming_obj.in ()))
    ACE_THROW (CORBA::UNKNOWN ());

  this->naming_context_ =
    CosNaming::NamingContext::_narrow (naming_obj.in () 
                                       TAO_ENV_ARG_PARAMETER);
  ACE_CHECK;
}

void
Notify_Test_Client::resolve_Notify_factory (TAO_ENV_SINGLE_ARG_DECL)
{
  CosNaming::Name name (1);
  name.length (1);
  name[0].id = CORBA::string_dup (NOTIFY_FACTORY_NAME);

  CORBA::Object_var obj =
    this->naming_context_->resolve (name
                                   TAO_ENV_ARG_PARAMETER);
  ACE_CHECK;

  this->notify_factory_ =
    CosNotifyChannelAdmin::EventChannelFactory::_narrow (
                                                    obj.in ()
                                                    TAO_ENV_ARG_PARAMETER
                                                  );
  ACE_CHECK;
}

int
Notify_Test_Client::ORB_run (void)
{
  while (!this->done_)
    if (this->orb_->work_pending ())
      this->orb_->perform_work ();

  return 0;
}

void
Notify_Test_Client::shutdown (TAO_ENV_SINGLE_ARG_DECL_NOT_USED)
{
  this->done_ = 1;
}

CORBA::ORB_ptr
Notify_Test_Client::orb (void)
{
  return this->orb_.in ();
}


PortableServer::POA_ptr
Notify_Test_Client::root_poa (void)
{
  return this->root_poa_.in ();
}


CosNaming::NamingContext_ptr
Notify_Test_Client::naming_context (void)
{
  return this->naming_context_.in ();
}

  
CosNotifyChannelAdmin::EventChannelFactory_ptr
Notify_Test_Client::notify_factory (void)
{
  return this->notify_factory_.in ();
}


CosNotifyChannelAdmin::EventChannel_ptr
Notify_Test_Client::create_event_channel (const char* cname,
                                          int resolve
                                          TAO_ENV_ARG_DECL)
{
  CosNotifyChannelAdmin::EventChannel_var ec;
  CosNaming::Name name (1);
  name.length (1);
  name[0].id = CORBA::string_dup (cname);

  if (resolve)
    {
      CORBA::Object_var obj = naming_context_->resolve (name);
      ec = CosNotifyChannelAdmin::EventChannel::_narrow (obj.in ());

      if (CORBA::is_nil (ec.in ()))
        {
          return 0;
        }
    }
  else
    {
      CosNotifyChannelAdmin::ChannelID id;
      CosNotification::QoSProperties initial_qos;
      CosNotification::AdminProperties initial_admin;

      ec = notify_factory_->create_channel (initial_qos,
                                            initial_admin,
                                            id
                                            TAO_ENV_ARG_PARAMETER);
      ACE_CHECK;

      naming_context_->rebind(name, ec.in());
    }

  return ec._retn ();
}


CORBA::Boolean&
Notify_Test_Client::done (void)
{
  return this->done_;
}


