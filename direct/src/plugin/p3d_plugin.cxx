// Filename: p3d_plugin.cxx
// Created by:  drose (29May09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "p3d_plugin_common.h"
#include "p3d_plugin_config.h"
#include "p3dInstanceManager.h"
#include "p3dInstance.h"
#include "p3dWindowParams.h"
#include "p3dUndefinedObject.h"
#include "p3dNoneObject.h"
#include "p3dBoolObject.h"
#include "p3dIntObject.h"
#include "p3dFloatObject.h"
#include "p3dStringObject.h"

#include <assert.h>

// Use a simple lock to protect the C-style API functions in this
// module from parallel access by multiple threads in the host.

bool initialized_lock = false;
LOCK _api_lock;

ofstream logfile;
string plugin_output_filename;
ostream *nout_stream;


bool 
P3D_initialize(int api_version, const char *contents_filename,
               const char *download_url, const char *platform) {
  if (api_version != P3D_API_VERSION) {
    // Can't accept an incompatible version.
    return false;
  }

  if (!initialized_lock) {
    INIT_LOCK(_api_lock);
    initialized_lock = true;
  }
  ACQUIRE_LOCK(_api_lock);

  if (contents_filename == NULL){ 
    contents_filename = "";
  }

  if (download_url == NULL){ 
    download_url = "";
  }

  if (platform == NULL) {
    platform = "";
  }

#ifdef P3D_PLUGIN_LOGFILE2
  string logfilename = P3D_PLUGIN_LOGFILE2;
#else
  string logfilename;
#endif  // P3D_PLUGIN_LOGFILE2

  if (logfilename.empty()) {
#ifdef _WIN32
    static const size_t buffer_size = 4096;
    char buffer[buffer_size];
    if (GetTempPath(buffer_size, buffer) != 0) {
      logfilename = buffer;
      logfilename += "panda3d.2.log";
    }
#else
    logfilename = "/tmp/panda3d.2.log";
#endif  // _WIN32
  }

  cerr << "logfile: " << logfilename << "\n";

  nout_stream = &cerr;
  logfile.open(logfilename.c_str(), ios::out | ios::trunc);
  if (logfile) {
    logfile.setf(ios::unitbuf);
    nout_stream = &logfile;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  bool result = inst_mgr->initialize(contents_filename, download_url, platform);
  RELEASE_LOCK(_api_lock);
  return result;
}

void 
P3D_finalize() {
  P3DInstanceManager::delete_global_ptr();
}

P3D_instance *
P3D_new_instance(P3D_request_ready_func *func, 
                 const P3D_token tokens[], size_t num_tokens,
                 void *user_data) {
  nout << "new_instance\n";
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *result = inst_mgr->create_instance(func, tokens, num_tokens, user_data);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_instance_start(P3D_instance *instance, const char *p3d_filename) {
  nout << "instance_start\n";
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (p3d_filename == NULL) {
    p3d_filename = "";
  }
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  bool result = false;
  if (inst != NULL) {
    result = inst_mgr->start_instance(inst, p3d_filename);
  }
  RELEASE_LOCK(_api_lock);
  return result;
}

void
P3D_instance_finish(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  if (inst != NULL) {
    inst_mgr->finish_instance(inst);
  }
  RELEASE_LOCK(_api_lock);
}

void
P3D_instance_setup_window(P3D_instance *instance,
                          P3D_window_type window_type,
                          int win_x, int win_y,
                          int win_width, int win_height,
                          P3D_window_handle parent_window) {
  nout << "setup_window\n";
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  P3DWindowParams wparams(window_type, win_x, win_y,
                          win_width, win_height, parent_window);

  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  if (inst != NULL) {
    inst->set_wparams(wparams);
  }
  RELEASE_LOCK(_api_lock);
}

P3D_object_type
P3D_object_get_type(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_object_type result = P3D_OBJECT_GET_TYPE(object);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_object_get_bool(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  bool result = P3D_OBJECT_GET_BOOL(object);
  RELEASE_LOCK(_api_lock);
  return result;
}

int
P3D_object_get_int(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  int result = P3D_OBJECT_GET_INT(object);
  RELEASE_LOCK(_api_lock);
  return result;
}

double
P3D_object_get_float(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  double result = P3D_OBJECT_GET_FLOAT(object);
  RELEASE_LOCK(_api_lock);
  return result;
}

int
P3D_object_get_string(P3D_object *object, char *buffer, int buffer_size) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  int result = P3D_OBJECT_GET_STRING(object, buffer, buffer_size);
  RELEASE_LOCK(_api_lock);
  return result;
}

int
P3D_object_get_repr(P3D_object *object, char *buffer, int buffer_size) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  int result = P3D_OBJECT_GET_REPR(object, buffer, buffer_size);
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_object_get_property(P3D_object *object, const char *property) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_object *result = P3D_OBJECT_GET_PROPERTY(object, property);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_object_set_property(P3D_object *object, const char *property, 
                        P3D_object *value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  bool result = P3D_OBJECT_SET_PROPERTY(object, property, value);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_object_has_method(P3D_object *object, const char *method_name) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  bool result = P3D_OBJECT_HAS_METHOD(object, method_name);
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_object_call(P3D_object *object, const char *method_name, 
                bool needs_response,
                P3D_object *params[], int num_params) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_object *result = P3D_OBJECT_CALL(object, method_name, needs_response,
                                       params, num_params);
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_object_eval(P3D_object *object, const char *expression) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_object *result = P3D_OBJECT_EVAL(object, expression);
  RELEASE_LOCK(_api_lock);
  return result;
}


void 
P3D_object_incref(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (object != NULL) {
    ACQUIRE_LOCK(_api_lock);
    P3D_OBJECT_INCREF(object);
    RELEASE_LOCK(_api_lock);
  }
}

void 
P3D_object_decref(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (object != NULL) {
    ACQUIRE_LOCK(_api_lock);
    P3D_OBJECT_DECREF(object);
    RELEASE_LOCK(_api_lock);
  }
}

P3D_class_definition *
P3D_make_class_definition() {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_class_definition *result = inst_mgr->make_class_definition();
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_undefined_object() {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_object *result = inst_mgr->new_undefined_object();
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_none_object() {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_object *result = inst_mgr->new_none_object();
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_bool_object(bool value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_object *result = inst_mgr->new_bool_object(value);
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_int_object(int value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = new P3DIntObject(value);
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_float_object(double value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = new P3DFloatObject(value);
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_string_object(const char *str, int length) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = new P3DStringObject(string(str, length));
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_instance_get_panda_script_object(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  P3D_object *result = NULL;
  if (inst != NULL) {
    result = inst->get_panda_script_object();
  }
  
  RELEASE_LOCK(_api_lock);
  return result;
}

void
P3D_instance_set_browser_script_object(P3D_instance *instance, 
                                       P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  if (inst != NULL) {
    inst->set_browser_script_object(object);
  }
  
  RELEASE_LOCK(_api_lock);
}


P3D_request *
P3D_instance_get_request(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  P3D_request *result = NULL;
  if (inst != NULL) {
    result = inst->get_request();
  }

  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_instance *
P3D_check_request(bool wait) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_instance *inst = inst_mgr->check_request();

  if (inst != NULL || !wait) {
    RELEASE_LOCK(_api_lock);
    return inst;
  }
  
  // Now we have to block until a request is available.
  while (inst == NULL && inst_mgr->get_num_instances() != 0) {
    RELEASE_LOCK(_api_lock);
    inst_mgr->wait_request();
    ACQUIRE_LOCK(_api_lock);
    inst = inst_mgr->check_request();
  }

  RELEASE_LOCK(_api_lock);
  return inst;
}

void
P3D_request_finish(P3D_request *request, bool handled) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  if (request != (P3D_request *)NULL) {
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    P3DInstance *inst = inst_mgr->validate_instance(request->_instance);
    if (inst != NULL) {
      inst->finish_request(request, handled);
    }
  }
  RELEASE_LOCK(_api_lock);
}

bool
P3D_instance_feed_url_stream(P3D_instance *instance, int unique_id,
                             P3D_result_code result_code,
                             int http_status_code, 
                             size_t total_expected_data,
                             const void *this_data, 
                             size_t this_data_size) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  bool result = false;
  if (inst != NULL) {
    result = inst->
      feed_url_stream(unique_id, result_code, http_status_code,
                      total_expected_data, 
                      (const unsigned char *)this_data, this_data_size);
  }

  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_instance_handle_event(P3D_instance *instance, P3D_event_data event) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  bool result = false;
  if (inst != NULL) {
    result = inst->handle_event(event);
  }

  RELEASE_LOCK(_api_lock);
  return result;
}
