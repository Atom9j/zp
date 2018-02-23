/*  =========================================================================
    zp_exec - ZeroMQ pipes process execution engine

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of ZP, ZeroMQ Piper
    http://githut.com/zmonit/zp

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

#ifndef ZP_EXEC_H_INCLUDED
#define ZP_EXEC_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _zp_exec_t zp_exec_t;

//  @interface
//  Create new zp_exec actor instance.
//  @TODO: Describe the purpose of this actor!
//
//      zactor_t *zp_exec = zactor_new (zp_exec, NULL);
//
//  Destroy zp_exec instance.
//
//      zactor_destroy (&zp_exec);
//
//  Enable verbose logging of commands and activity:
//
//      zsock_bsend (zp_exec, "sp", "VERBOSE", NULL);
//
//  Start zp_exec actor.
//
//      zsock_bsend (zp_exec, "sp", "START", zp_proto_dup (msg));
//
//  Stop zp_exec actor.
//
//      zsock_bsend (zp_exec, "sp", "STOP", NULL);
//
//  Terminate zp_exec actor.
//
//      zsock_bsend (zp_exec, "sp", "$TERM", NULL);
//
//  This is the zp_exec constructor as a zactor_fn;
ZP_EXPORT void
    zp_exec_actor (zsock_t *pipe, void *args);

//  Destructor for zp_exec actor
ZP_EXPORT void
    zp_exec_destructor (zactor_t *pipe);

//  Self test of this actor
ZP_EXPORT void
    zp_exec_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
