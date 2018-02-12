/*  =========================================================================
    zp_proto - The ZeroMQ Pipes Protocol

    Codec class for zp_proto.

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

/*
@header
    zp_proto - The ZeroMQ Pipes Protocol
@discuss
@end
*/

#ifdef NDEBUG
#undef NDEBUG
#endif

#include "zp_classes.h"
#include "../include/zp_proto.h"

//  Structure of our class

struct _zp_proto_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  zp_proto message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char client_id [256];               //  Client unique identifier
    uint16_t status_code;               //  3-digit status code
    char status_reason [256];           //  Printable explanation
};

//  --------------------------------------------------------------------------
//  Network data encoding macros

//  Put a block of octets to the frame
#define PUT_OCTETS(host,size) { \
    memcpy (self->needle, (host), size); \
    self->needle += size; \
}

//  Get a block of octets from the frame
#define GET_OCTETS(host,size) { \
    if (self->needle + size > self->ceiling) { \
        zsys_warning ("zp_proto: GET_OCTETS failed"); \
        goto malformed; \
    } \
    memcpy ((host), self->needle, size); \
    self->needle += size; \
}

//  Put a 1-byte number to the frame
#define PUT_NUMBER1(host) { \
    *(byte *) self->needle = (byte) (host); \
    self->needle++; \
}

//  Put a 2-byte number to the frame
#define PUT_NUMBER2(host) { \
    self->needle [0] = (byte) (((host) >> 8)  & 255); \
    self->needle [1] = (byte) (((host))       & 255); \
    self->needle += 2; \
}

//  Put a 4-byte number to the frame
#define PUT_NUMBER4(host) { \
    self->needle [0] = (byte) (((host) >> 24) & 255); \
    self->needle [1] = (byte) (((host) >> 16) & 255); \
    self->needle [2] = (byte) (((host) >> 8)  & 255); \
    self->needle [3] = (byte) (((host))       & 255); \
    self->needle += 4; \
}

//  Put a 8-byte number to the frame
#define PUT_NUMBER8(host) { \
    self->needle [0] = (byte) (((host) >> 56) & 255); \
    self->needle [1] = (byte) (((host) >> 48) & 255); \
    self->needle [2] = (byte) (((host) >> 40) & 255); \
    self->needle [3] = (byte) (((host) >> 32) & 255); \
    self->needle [4] = (byte) (((host) >> 24) & 255); \
    self->needle [5] = (byte) (((host) >> 16) & 255); \
    self->needle [6] = (byte) (((host) >> 8)  & 255); \
    self->needle [7] = (byte) (((host))       & 255); \
    self->needle += 8; \
}

//  Get a 1-byte number from the frame
#define GET_NUMBER1(host) { \
    if (self->needle + 1 > self->ceiling) { \
        zsys_warning ("zp_proto: GET_NUMBER1 failed"); \
        goto malformed; \
    } \
    (host) = *(byte *) self->needle; \
    self->needle++; \
}

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) { \
        zsys_warning ("zp_proto: GET_NUMBER2 failed"); \
        goto malformed; \
    } \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
}

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) { \
        zsys_warning ("zp_proto: GET_NUMBER4 failed"); \
        goto malformed; \
    } \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
}

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) { \
        zsys_warning ("zp_proto: GET_NUMBER8 failed"); \
        goto malformed; \
    } \
    (host) = ((uint64_t) (self->needle [0]) << 56) \
           + ((uint64_t) (self->needle [1]) << 48) \
           + ((uint64_t) (self->needle [2]) << 40) \
           + ((uint64_t) (self->needle [3]) << 32) \
           + ((uint64_t) (self->needle [4]) << 24) \
           + ((uint64_t) (self->needle [5]) << 16) \
           + ((uint64_t) (self->needle [6]) << 8) \
           +  (uint64_t) (self->needle [7]); \
    self->needle += 8; \
}

//  Put a string to the frame
#define PUT_STRING(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER1 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a string from the frame
#define GET_STRING(host) { \
    size_t string_size; \
    GET_NUMBER1 (string_size); \
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("zp_proto: GET_STRING failed"); \
        goto malformed; \
    } \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}

//  Put a long string to the frame
#define PUT_LONGSTR(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER4 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a long string from the frame
#define GET_LONGSTR(host) { \
    size_t string_size; \
    GET_NUMBER4 (string_size); \
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("zp_proto: GET_LONGSTR failed"); \
        goto malformed; \
    } \
    free ((host)); \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}

//  --------------------------------------------------------------------------
//  bytes cstring conversion macros

// create new array of unsigned char from properly encoded string
// len of resulting array is strlen (str) / 2
// caller is responsibe for freeing up the memory
#define BYTES_FROM_STR(dst, _str) { \
    char *str = (char*) (_str); \
    if (!str || (strlen (str) % 2) != 0) \
        return NULL; \
\
    size_t strlen_2 = strlen (str) / 2; \
    byte *mem = (byte*) zmalloc (strlen_2); \
    size_t i; \
\
    for (i = 0; i != strlen_2; i++) \
    { \
        char buff[3] = {0x0, 0x0, 0x0}; \
        strncpy (buff, str, 2); \
        unsigned int uint; \
        sscanf (buff, "%x", &uint); \
        assert (uint <= 0xff); \
        mem [i] = (byte) (0xff & uint); \
        str += 2; \
    } \
    dst = mem; \
}

// convert len bytes to hex string
// caller is responsibe for freeing up the memory
#define STR_FROM_BYTES(dst, _mem, _len) { \
    byte *mem = (byte*) (_mem); \
    size_t len = (size_t) (_len); \
    char* ret = (char*) zmalloc (2*len + 1); \
    char* aux = ret; \
    size_t i; \
    for (i = 0; i != len; i++) \
    { \
        sprintf (aux, "%02x", mem [i]); \
        aux+=2; \
    } \
    dst = ret; \
}

//  --------------------------------------------------------------------------
//  Create a new zp_proto

zp_proto_t *
zp_proto_new (void)
{
    zp_proto_t *self = (zp_proto_t *) zmalloc (sizeof (zp_proto_t));
    return self;
}

//  --------------------------------------------------------------------------
//  Create a new zp_proto from zpl/zconfig_t *

zp_proto_t *
    zp_proto_new_zpl (zconfig_t *config)
{
    assert (config);
    zp_proto_t *self = NULL;
    char *message = zconfig_get (config, "message", NULL);
    if (!message) {
        zsys_error ("Can't find 'message' section");
        return NULL;
    }

    if (streq ("ZP_PROTO_CONNECTION_OPEN", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_CONNECTION_OPEN);
    }
    else
    if (streq ("ZP_PROTO_CONNECTION_PING", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_CONNECTION_PING);
    }
    else
    if (streq ("ZP_PROTO_CONNECTION_PONG", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_CONNECTION_PONG);
    }
    else
    if (streq ("ZP_PROTO_CONNECTION_CLOSE", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_CONNECTION_CLOSE);
    }
    else
    if (streq ("ZP_PROTO_OK", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_OK);
    }
    else
    if (streq ("ZP_PROTO_ERROR", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_ERROR);
    }
    else
       {
        zsys_error ("message=%s is not known", message);
        return NULL;
       }

    char *s = zconfig_get (config, "routing_id", NULL);
    if (s) {
        byte *bvalue;
        BYTES_FROM_STR (bvalue, s);
        if (!bvalue) {
            zp_proto_destroy (&self);
            return NULL;
        }
        zframe_t *frame = zframe_new (bvalue, strlen (s) / 2);
        free (bvalue);
        self->routing_id = frame;
    }

    zconfig_t *content = NULL;
    switch (self->id) {
        case ZP_PROTO_CONNECTION_OPEN:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            char *s = zconfig_get (content, "client_id", NULL);
            if (!s) {
                zp_proto_destroy (&self);
                return NULL;
            }
            strncpy (self->client_id, s, 256);
            }
            break;
        case ZP_PROTO_CONNECTION_PING:
            break;
        case ZP_PROTO_CONNECTION_PONG:
            break;
        case ZP_PROTO_CONNECTION_CLOSE:
            break;
        case ZP_PROTO_OK:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            char *es = NULL;
            char *s = zconfig_get (content, "status_code", NULL);
            if (!s) {
                zsys_error ("content/status_code not found");
                zp_proto_destroy (&self);
                return NULL;
            }
            uint64_t uvalue = (uint64_t) strtoll (s, &es, 10);
            if (es != s+strlen (s)) {
                zsys_error ("content/status_code: %s is not a number", s);
                zp_proto_destroy (&self);
                return NULL;
            }
            self->status_code = uvalue;
            }
            {
            char *s = zconfig_get (content, "status_reason", NULL);
            if (!s) {
                zp_proto_destroy (&self);
                return NULL;
            }
            strncpy (self->status_reason, s, 256);
            }
            break;
        case ZP_PROTO_ERROR:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            char *es = NULL;
            char *s = zconfig_get (content, "status_code", NULL);
            if (!s) {
                zsys_error ("content/status_code not found");
                zp_proto_destroy (&self);
                return NULL;
            }
            uint64_t uvalue = (uint64_t) strtoll (s, &es, 10);
            if (es != s+strlen (s)) {
                zsys_error ("content/status_code: %s is not a number", s);
                zp_proto_destroy (&self);
                return NULL;
            }
            self->status_code = uvalue;
            }
            {
            char *s = zconfig_get (content, "status_reason", NULL);
            if (!s) {
                zp_proto_destroy (&self);
                return NULL;
            }
            strncpy (self->status_reason, s, 256);
            }
            break;
    }
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the zp_proto

void
zp_proto_destroy (zp_proto_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zp_proto_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Create a deep copy of a zp_proto instance

zp_proto_t *
zp_proto_dup (zp_proto_t *other)
{
    assert (other);
    zp_proto_t *copy = zp_proto_new ();

    // Copy the routing and message id
    zp_proto_set_routing_id (copy, zp_proto_routing_id (other));
    zp_proto_set_id (copy, zp_proto_id (other));

    // Copy the rest of the fields
    zp_proto_set_client_id (copy, zp_proto_client_id (other));
    zp_proto_set_status_code (copy, zp_proto_status_code (other));
    zp_proto_set_status_reason (copy, zp_proto_status_reason (other));

    return copy;
}

//  --------------------------------------------------------------------------
//  Receive a zp_proto from the socket. Returns 0 if OK, -1 if
//  the recv was interrupted, or -2 if the message is malformed.
//  Blocks if there is no message waiting.

int
zp_proto_recv (zp_proto_t *self, zsock_t *input)
{
    assert (input);
    int rc = 0;
    zmq_msg_t frame;
    zmq_msg_init (&frame);

    if (zsock_type (input) == ZMQ_ROUTER) {
        zframe_destroy (&self->routing_id);
        self->routing_id = zframe_recv (input);
        if (!self->routing_id || !zsock_rcvmore (input)) {
            zsys_warning ("zp_proto: no routing ID");
            rc = -1;            //  Interrupted
            goto malformed;
        }
    }
    int size;
    size = zmq_msg_recv (&frame, zsock_resolve (input), 0);
    if (size == -1) {
        zsys_warning ("zp_proto: interrupted");
        rc = -1;                //  Interrupted
        goto malformed;
    }
    //  Get and check protocol signature
    self->needle = (byte *) zmq_msg_data (&frame);
    self->ceiling = self->needle + zmq_msg_size (&frame);

    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 42)) {
        zsys_warning ("zp_proto: invalid signature");
        rc = -2;                //  Malformed
        goto malformed;
    }
    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case ZP_PROTO_CONNECTION_OPEN:
            {
                char protocol [256];
                GET_STRING (protocol);
                if (strneq (protocol, "MALAMUTE")) {
                    zsys_warning ("zp_proto: protocol is invalid");
                    rc = -2;    //  Malformed
                    goto malformed;
                }
            }
            {
                uint16_t version;
                GET_NUMBER2 (version);
                if (version != 1) {
                    zsys_warning ("zp_proto: version is invalid");
                    rc = -2;    //  Malformed
                    goto malformed;
                }
            }
            GET_STRING (self->client_id);
            break;

        case ZP_PROTO_CONNECTION_PING:
            break;

        case ZP_PROTO_CONNECTION_PONG:
            break;

        case ZP_PROTO_CONNECTION_CLOSE:
            break;

        case ZP_PROTO_OK:
            GET_NUMBER2 (self->status_code);
            GET_STRING (self->status_reason);
            break;

        case ZP_PROTO_ERROR:
            GET_NUMBER2 (self->status_code);
            GET_STRING (self->status_reason);
            break;

        default:
            zsys_warning ("zp_proto: bad message ID");
            rc = -2;            //  Malformed
            goto malformed;
    }
    //  Successful return
    zmq_msg_close (&frame);
    return rc;

    //  Error returns
    malformed:
        zmq_msg_close (&frame);
        return rc;              //  Invalid message
}


//  --------------------------------------------------------------------------
//  Send the zp_proto to the socket. Does not destroy it. Returns 0 if
//  OK, else -1.

int
zp_proto_send (zp_proto_t *self, zsock_t *output)
{
    assert (self);
    assert (output);

    if (zsock_type (output) == ZMQ_ROUTER)
        zframe_send (&self->routing_id, output, ZFRAME_MORE + ZFRAME_REUSE);

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case ZP_PROTO_CONNECTION_OPEN:
            frame_size += 1 + strlen ("MALAMUTE");
            frame_size += 2;            //  version
            frame_size += 1 + strlen (self->client_id);
            break;
        case ZP_PROTO_OK:
            frame_size += 2;            //  status_code
            frame_size += 1 + strlen (self->status_reason);
            break;
        case ZP_PROTO_ERROR:
            frame_size += 2;            //  status_code
            frame_size += 1 + strlen (self->status_reason);
            break;
    }
    //  Now serialize message into the frame
    zmq_msg_t frame;
    zmq_msg_init_size (&frame, frame_size);
    self->needle = (byte *) zmq_msg_data (&frame);
    PUT_NUMBER2 (0xAAA0 | 42);
    PUT_NUMBER1 (self->id);
    size_t nbr_frames = 1;              //  Total number of frames to send

    switch (self->id) {
        case ZP_PROTO_CONNECTION_OPEN:
            PUT_STRING ("MALAMUTE");
            PUT_NUMBER2 (1);
            PUT_STRING (self->client_id);
            break;

        case ZP_PROTO_OK:
            PUT_NUMBER2 (self->status_code);
            PUT_STRING (self->status_reason);
            break;

        case ZP_PROTO_ERROR:
            PUT_NUMBER2 (self->status_code);
            PUT_STRING (self->status_reason);
            break;

    }
    //  Now send the data frame
    zmq_msg_send (&frame, zsock_resolve (output), --nbr_frames? ZMQ_SNDMORE: 0);

    return 0;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
zp_proto_print (zp_proto_t *self)
{
    assert (self);
    switch (self->id) {
        case ZP_PROTO_CONNECTION_OPEN:
            zsys_debug ("ZP_PROTO_CONNECTION_OPEN:");
            zsys_debug ("    protocol=malamute");
            zsys_debug ("    version=1");
            zsys_debug ("    client_id='%s'", self->client_id);
            break;

        case ZP_PROTO_CONNECTION_PING:
            zsys_debug ("ZP_PROTO_CONNECTION_PING:");
            break;

        case ZP_PROTO_CONNECTION_PONG:
            zsys_debug ("ZP_PROTO_CONNECTION_PONG:");
            break;

        case ZP_PROTO_CONNECTION_CLOSE:
            zsys_debug ("ZP_PROTO_CONNECTION_CLOSE:");
            break;

        case ZP_PROTO_OK:
            zsys_debug ("ZP_PROTO_OK:");
            zsys_debug ("    status_code=%ld", (long) self->status_code);
            zsys_debug ("    status_reason='%s'", self->status_reason);
            break;

        case ZP_PROTO_ERROR:
            zsys_debug ("ZP_PROTO_ERROR:");
            zsys_debug ("    status_code=%ld", (long) self->status_code);
            zsys_debug ("    status_reason='%s'", self->status_reason);
            break;

    }
}

//  --------------------------------------------------------------------------
//  Export class as zconfig_t*. Caller is responsibe for destroying the instance

zconfig_t *
zp_proto_zpl (zp_proto_t *self, zconfig_t *parent)
{
    assert (self);

    zconfig_t *root = zconfig_new ("zp_proto", parent);

    switch (self->id) {
        case ZP_PROTO_CONNECTION_OPEN:
        {
            zconfig_put (root, "message", "ZP_PROTO_CONNECTION_OPEN");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            zconfig_putf (config, "protocol", "%s", "malamute");
            zconfig_putf (config, "version", "%s", "1");
            zconfig_putf (config, "client_id", "%s", self->client_id);
            break;
            }
        case ZP_PROTO_CONNECTION_PING:
        {
            zconfig_put (root, "message", "ZP_PROTO_CONNECTION_PING");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            break;
            }
        case ZP_PROTO_CONNECTION_PONG:
        {
            zconfig_put (root, "message", "ZP_PROTO_CONNECTION_PONG");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            break;
            }
        case ZP_PROTO_CONNECTION_CLOSE:
        {
            zconfig_put (root, "message", "ZP_PROTO_CONNECTION_CLOSE");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            break;
            }
        case ZP_PROTO_OK:
        {
            zconfig_put (root, "message", "ZP_PROTO_OK");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            zconfig_putf (config, "status_code", "%ld", (long) self->status_code);
            zconfig_putf (config, "status_reason", "%s", self->status_reason);
            break;
            }
        case ZP_PROTO_ERROR:
        {
            zconfig_put (root, "message", "ZP_PROTO_ERROR");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            zconfig_putf (config, "status_code", "%ld", (long) self->status_code);
            zconfig_putf (config, "status_reason", "%s", self->status_reason);
            break;
            }
    }
    return root;
}

//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
zp_proto_routing_id (zp_proto_t *self)
{
    assert (self);
    return self->routing_id;
}

void
zp_proto_set_routing_id (zp_proto_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the zp_proto id

int
zp_proto_id (zp_proto_t *self)
{
    assert (self);
    return self->id;
}

void
zp_proto_set_id (zp_proto_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
zp_proto_command (zp_proto_t *self)
{
    assert (self);
    switch (self->id) {
        case ZP_PROTO_CONNECTION_OPEN:
            return ("CONNECTION_OPEN");
            break;
        case ZP_PROTO_CONNECTION_PING:
            return ("CONNECTION_PING");
            break;
        case ZP_PROTO_CONNECTION_PONG:
            return ("CONNECTION_PONG");
            break;
        case ZP_PROTO_CONNECTION_CLOSE:
            return ("CONNECTION_CLOSE");
            break;
        case ZP_PROTO_OK:
            return ("OK");
            break;
        case ZP_PROTO_ERROR:
            return ("ERROR");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the client_id field

const char *
zp_proto_client_id (zp_proto_t *self)
{
    assert (self);
    return self->client_id;
}

void
zp_proto_set_client_id (zp_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->client_id)
        return;
    strncpy (self->client_id, value, 255);
    self->client_id [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the status_code field

uint16_t
zp_proto_status_code (zp_proto_t *self)
{
    assert (self);
    return self->status_code;
}

void
zp_proto_set_status_code (zp_proto_t *self, uint16_t status_code)
{
    assert (self);
    self->status_code = status_code;
}


//  --------------------------------------------------------------------------
//  Get/set the status_reason field

const char *
zp_proto_status_reason (zp_proto_t *self)
{
    assert (self);
    return self->status_reason;
}

void
zp_proto_set_status_reason (zp_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->status_reason)
        return;
    strncpy (self->status_reason, value, 255);
    self->status_reason [255] = 0;
}



//  --------------------------------------------------------------------------
//  Selftest

void
zp_proto_test (bool verbose)
{
    printf (" * zp_proto: ");

    if (verbose)
        printf ("\n");

    //  @selftest
    //  Simple create/destroy test
    zconfig_t *config;
    zp_proto_t *self = zp_proto_new ();
    assert (self);
    zp_proto_destroy (&self);
    //  Create pair of sockets we can send through
    //  We must bind before connect if we wish to remain compatible with ZeroMQ < v4
    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    int rc = zsock_bind (output, "inproc://selftest-zp_proto");
    assert (rc == 0);

    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    rc = zsock_connect (input, "inproc://selftest-zp_proto");
    assert (rc == 0);


    //  Encode/send/decode and verify each message type
    int instance;
    self = zp_proto_new ();
    zp_proto_set_id (self, ZP_PROTO_CONNECTION_OPEN);

    zp_proto_set_client_id (self, "Life is short but Now lasts for ever");
    // convert to zpl
    config = zp_proto_zpl (self, NULL);
    if (verbose)
        zconfig_print (config);
    //  Send twice
    zp_proto_send (self, output);
    zp_proto_send (self, output);

    for (instance = 0; instance < 3; instance++) {
        zp_proto_t *self_temp = self;
        if (instance < 2)
            zp_proto_recv (self, input);
        else {
            self = zp_proto_new_zpl (config);
            assert (self);
            zconfig_destroy (&config);
        }
        if (instance < 2)
            assert (zp_proto_routing_id (self));
        assert (streq (zp_proto_client_id (self), "Life is short but Now lasts for ever"));
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_CONNECTION_PING);

    // convert to zpl
    config = zp_proto_zpl (self, NULL);
    if (verbose)
        zconfig_print (config);
    //  Send twice
    zp_proto_send (self, output);
    zp_proto_send (self, output);

    for (instance = 0; instance < 3; instance++) {
        zp_proto_t *self_temp = self;
        if (instance < 2)
            zp_proto_recv (self, input);
        else {
            self = zp_proto_new_zpl (config);
            assert (self);
            zconfig_destroy (&config);
        }
        if (instance < 2)
            assert (zp_proto_routing_id (self));
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_CONNECTION_PONG);

    // convert to zpl
    config = zp_proto_zpl (self, NULL);
    if (verbose)
        zconfig_print (config);
    //  Send twice
    zp_proto_send (self, output);
    zp_proto_send (self, output);

    for (instance = 0; instance < 3; instance++) {
        zp_proto_t *self_temp = self;
        if (instance < 2)
            zp_proto_recv (self, input);
        else {
            self = zp_proto_new_zpl (config);
            assert (self);
            zconfig_destroy (&config);
        }
        if (instance < 2)
            assert (zp_proto_routing_id (self));
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_CONNECTION_CLOSE);

    // convert to zpl
    config = zp_proto_zpl (self, NULL);
    if (verbose)
        zconfig_print (config);
    //  Send twice
    zp_proto_send (self, output);
    zp_proto_send (self, output);

    for (instance = 0; instance < 3; instance++) {
        zp_proto_t *self_temp = self;
        if (instance < 2)
            zp_proto_recv (self, input);
        else {
            self = zp_proto_new_zpl (config);
            assert (self);
            zconfig_destroy (&config);
        }
        if (instance < 2)
            assert (zp_proto_routing_id (self));
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_OK);

    zp_proto_set_status_code (self, 123);
    zp_proto_set_status_reason (self, "Life is short but Now lasts for ever");
    // convert to zpl
    config = zp_proto_zpl (self, NULL);
    if (verbose)
        zconfig_print (config);
    //  Send twice
    zp_proto_send (self, output);
    zp_proto_send (self, output);

    for (instance = 0; instance < 3; instance++) {
        zp_proto_t *self_temp = self;
        if (instance < 2)
            zp_proto_recv (self, input);
        else {
            self = zp_proto_new_zpl (config);
            assert (self);
            zconfig_destroy (&config);
        }
        if (instance < 2)
            assert (zp_proto_routing_id (self));
        assert (zp_proto_status_code (self) == 123);
        assert (streq (zp_proto_status_reason (self), "Life is short but Now lasts for ever"));
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_ERROR);

    zp_proto_set_status_code (self, 123);
    zp_proto_set_status_reason (self, "Life is short but Now lasts for ever");
    // convert to zpl
    config = zp_proto_zpl (self, NULL);
    if (verbose)
        zconfig_print (config);
    //  Send twice
    zp_proto_send (self, output);
    zp_proto_send (self, output);

    for (instance = 0; instance < 3; instance++) {
        zp_proto_t *self_temp = self;
        if (instance < 2)
            zp_proto_recv (self, input);
        else {
            self = zp_proto_new_zpl (config);
            assert (self);
            zconfig_destroy (&config);
        }
        if (instance < 2)
            assert (zp_proto_routing_id (self));
        assert (zp_proto_status_code (self) == 123);
        assert (streq (zp_proto_status_reason (self), "Life is short but Now lasts for ever"));
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }

    zp_proto_destroy (&self);
    zsock_destroy (&input);
    zsock_destroy (&output);
#if defined (__WINDOWS__)
    zsys_shutdown();
#endif
    //  @end

    printf ("OK\n");
}
