/*  =========================================================================
    zp_proto - The ZeroMQ Pipes Protocol

    Codec header for zp_proto.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: zp_proto.xml, or
     * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of ZP, ZeroMQ Piper
    http://githut.com/zmonit/zp

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

#ifndef ZP_PROTO_H_INCLUDED
#define ZP_PROTO_H_INCLUDED

/*  These are the zp_proto messages:

    CONNECTION_OPEN - Client opening connection to server. Client has own (unique) uuid
generated and stored by it. Server replies with OK or ERROR.
        protocol            string      Constant "MALAMUTE"
        version             number 2    Protocol version 1
        client_id           string      Client unique identifier

    CONNECTION_PING - Client pings the server. Server replies with CONNECTION-PONG, or
ERROR with status COMMAND-INVALID if the client is not recognized
(e.g. after a server restart or network recovery).

    CONNECTION_PONG - Server replies to a client connection ping.

    CONNECTION_CLOSE - Client closes the connection. This is polite though not mandatory.
Server will reply with OK.

    OK - Server replies with success status. Actual status code provides more
information. An OK always has a 2xx status code.
        status_code         number 2    3-digit status code
        status_reason       string      Printable explanation

    ERROR - Server replies with failure status. Actual status code provides more
information. An ERROR always has a 4xx, or 5xx status code. Is similar
to HTTP status codes.
        status_code         number 2    3-digit status code
        status_reason       string      Printable explanation
*/

#define ZP_PROTO_SUCCESS                    200
#define ZP_PROTO_BAD_REQUEST                400
#define ZP_PROTO_INTERNAL_SERVER_ERROR      500
#define ZP_PROTO_NOT_IMPLEMENTED            501

#define ZP_PROTO_CONNECTION_OPEN            1
#define ZP_PROTO_CONNECTION_PING            2
#define ZP_PROTO_CONNECTION_PONG            3
#define ZP_PROTO_CONNECTION_CLOSE           4
#define ZP_PROTO_OK                         5
#define ZP_PROTO_ERROR                      6

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef ZP_PROTO_T_DEFINED
typedef struct _zp_proto_t zp_proto_t;
#define ZP_PROTO_T_DEFINED
#endif

//  @interface
//  Create a new empty zp_proto
ZP_EXPORT zp_proto_t *
    zp_proto_new (void);

//  Create a new zp_proto from zpl/zconfig_t *
ZP_EXPORT zp_proto_t *
    zp_proto_new_zpl (zconfig_t *config);

//  Destroy a zp_proto instance
ZP_EXPORT void
    zp_proto_destroy (zp_proto_t **self_p);

//  Create a deep copy of a zp_proto instance
ZP_EXPORT zp_proto_t *
    zp_proto_dup (zp_proto_t *other);

//  Receive a zp_proto from the socket. Returns 0 if OK, -1 if
//  the read was interrupted, or -2 if the message is malformed.
//  Blocks if there is no message waiting.
ZP_EXPORT int
    zp_proto_recv (zp_proto_t *self, zsock_t *input);

//  Send the zp_proto to the output socket, does not destroy it
ZP_EXPORT int
    zp_proto_send (zp_proto_t *self, zsock_t *output);


//  Print contents of message to stdout
ZP_EXPORT void
    zp_proto_print (zp_proto_t *self);

//  Export class as zconfig_t*. Caller is responsibe for destroying the instance
ZP_EXPORT zconfig_t *
    zp_proto_zpl (zp_proto_t *self, zconfig_t* parent);

//  Get/set the message routing id
ZP_EXPORT zframe_t *
    zp_proto_routing_id (zp_proto_t *self);
ZP_EXPORT void
    zp_proto_set_routing_id (zp_proto_t *self, zframe_t *routing_id);

//  Get the zp_proto id and printable command
ZP_EXPORT int
    zp_proto_id (zp_proto_t *self);
ZP_EXPORT void
    zp_proto_set_id (zp_proto_t *self, int id);
ZP_EXPORT const char *
    zp_proto_command (zp_proto_t *self);

//  Get/set the client_id field
ZP_EXPORT const char *
    zp_proto_client_id (zp_proto_t *self);
ZP_EXPORT void
    zp_proto_set_client_id (zp_proto_t *self, const char *value);

//  Get/set the status_code field
ZP_EXPORT uint16_t
    zp_proto_status_code (zp_proto_t *self);
ZP_EXPORT void
    zp_proto_set_status_code (zp_proto_t *self, uint16_t status_code);

//  Get/set the status_reason field
ZP_EXPORT const char *
    zp_proto_status_reason (zp_proto_t *self);
ZP_EXPORT void
    zp_proto_set_status_reason (zp_proto_t *self, const char *value);

//  Self test of this class
ZP_EXPORT void
    zp_proto_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define zp_proto_dump       zp_proto_print

#ifdef __cplusplus
}
#endif

#endif
