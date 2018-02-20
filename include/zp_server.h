/*  =========================================================================
    zp_server - ZeroMq Pipes Server

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: zp_server.xml, or
     * The code generation script that built this file: zproto_server_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of ZP, ZeroMQ Piper
    http://githut.com/zmonit/zp

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

#ifndef ZP_SERVER_H_INCLUDED
#define ZP_SERVER_H_INCLUDED

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  To work with zp_server, use the CZMQ zactor API:
//
//  Create new zp_server instance, passing logging prefix:
//
//      zactor_t *zp_server = zactor_new (zp_server, "myname");
//
//  Destroy zp_server instance
//
//      zactor_destroy (&zp_server);
//
//  Enable verbose logging of commands and activity:
//
//      zstr_send (zp_server, "VERBOSE");
//
//  Bind zp_server to specified endpoint. TCP endpoints may specify
//  the port number as "*" to aquire an ephemeral port:
//
//      zstr_sendx (zp_server, "BIND", endpoint, NULL);
//
//  Return assigned port number, specifically when BIND was done using an
//  an ephemeral port:
//
//      zstr_sendx (zp_server, "PORT", NULL);
//      char *command, *port_str;
//      zstr_recvx (zp_server, &command, &port_str, NULL);
//      assert (streq (command, "PORT"));
//
//  Specify configuration file to load, overwriting any previous loaded
//  configuration file or options:
//
//      zstr_sendx (zp_server, "LOAD", filename, NULL);
//
//  Set configuration path value:
//
//      zstr_sendx (zp_server, "SET", path, value, NULL);
//
//  Save configuration data to config file on disk:
//
//      zstr_sendx (zp_server, "SAVE", filename, NULL);
//
//  Send zmsg_t instance to zp_server:
//
//      zactor_send (zp_server, &msg);
//
//  Receive zmsg_t instance from zp_server:
//
//      zmsg_t *msg = zactor_recv (zp_server);
//
//  This is the zp_server constructor as a zactor_fn:
//
ZP_EXPORT void
    zp_server (zsock_t *pipe, void *args);

//  Self test of this class
ZP_EXPORT void
    zp_server_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
