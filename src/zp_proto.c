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
    zlist_t *args;                      //  Arguments of command to be run
    zhash_t *env;                       //  Optional environment variables to pass to executed process.
    size_t env_bytes;                   //  Size of hash content
    zhash_t *aux;                       //  Additional data controlling program execution
    size_t aux_bytes;                   //  Size of hash content
    uint32_t handle;                    //  Handle id of a running job. Use it to check the status of command execution.
    uint16_t returncode;                //  Return code, ZP_PROTO_QUEUED, ZP_PROTO_RUNNING
    uint32_t fd;                        //  (server) fd number
    zchunk_t *chunk;                    //  Chunk with a result
    zlist_t *handles;                   //  List of handles belonging to this (?) client
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
    if (streq ("ZP_PROTO_EXEC", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_EXEC);
    }
    else
    if (streq ("ZP_PROTO_EXEC_STATUS_REPLY", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_EXEC_STATUS_REPLY);
    }
    else
    if (streq ("ZP_PROTO_EXEC_CHUNK", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_EXEC_CHUNK);
    }
    else
    if (streq ("ZP_PROTO_EXEC_LIST_REPLY", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_EXEC_LIST_REPLY);
    }
    else
    if (streq ("ZP_PROTO_EXEC_STATUS_GET", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_EXEC_STATUS_GET);
    }
    else
    if (streq ("ZP_PROTO_EXEC_LIST", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_EXEC_LIST);
    }
    else
    if (streq ("ZP_PROTO_EXEC_CANCEL", message)) {
        self = zp_proto_new ();
        zp_proto_set_id (self, ZP_PROTO_EXEC_CANCEL);
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
        case ZP_PROTO_EXEC:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            zconfig_t *zstrings = zconfig_locate (content, "args");
            if (zstrings) {
                zlist_t *strings = zlist_new ();
                zlist_autofree (strings);
                for (zconfig_t *child = zconfig_child (zstrings);
                                child != NULL;
                                child = zconfig_next (child))
                {
                    zlist_append (strings, zconfig_value (child));
                }
                self->args = strings;
            }
            }
            {
            zconfig_t *zhash = zconfig_locate (content, "env");
            if (zhash) {
                zhash_t *hash = zhash_new ();
                zhash_autofree (hash);
                for (zconfig_t *child = zconfig_child (zhash);
                                child != NULL;
                                child = zconfig_next (child))
                {
                    zhash_update (hash, zconfig_name (child), zconfig_value (child));
                }
                self->env = hash;
            }
            }
            {
            zconfig_t *zhash = zconfig_locate (content, "aux");
            if (zhash) {
                zhash_t *hash = zhash_new ();
                zhash_autofree (hash);
                for (zconfig_t *child = zconfig_child (zhash);
                                child != NULL;
                                child = zconfig_next (child))
                {
                    zhash_update (hash, zconfig_name (child), zconfig_value (child));
                }
                self->aux = hash;
            }
            }
            break;
        case ZP_PROTO_EXEC_STATUS_REPLY:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            char *es = NULL;
            char *s = zconfig_get (content, "handle", NULL);
            if (!s) {
                zsys_error ("content/handle not found");
                zp_proto_destroy (&self);
                return NULL;
            }
            uint64_t uvalue = (uint64_t) strtoll (s, &es, 10);
            if (es != s+strlen (s)) {
                zsys_error ("content/handle: %s is not a number", s);
                zp_proto_destroy (&self);
                return NULL;
            }
            self->handle = uvalue;
            }
            {
            char *es = NULL;
            char *s = zconfig_get (content, "returncode", NULL);
            if (!s) {
                zsys_error ("content/returncode not found");
                zp_proto_destroy (&self);
                return NULL;
            }
            uint64_t uvalue = (uint64_t) strtoll (s, &es, 10);
            if (es != s+strlen (s)) {
                zsys_error ("content/returncode: %s is not a number", s);
                zp_proto_destroy (&self);
                return NULL;
            }
            self->returncode = uvalue;
            }
            break;
        case ZP_PROTO_EXEC_CHUNK:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            char *es = NULL;
            char *s = zconfig_get (content, "handle", NULL);
            if (!s) {
                zsys_error ("content/handle not found");
                zp_proto_destroy (&self);
                return NULL;
            }
            uint64_t uvalue = (uint64_t) strtoll (s, &es, 10);
            if (es != s+strlen (s)) {
                zsys_error ("content/handle: %s is not a number", s);
                zp_proto_destroy (&self);
                return NULL;
            }
            self->handle = uvalue;
            }
            {
            char *es = NULL;
            char *s = zconfig_get (content, "fd", NULL);
            if (!s) {
                zsys_error ("content/fd not found");
                zp_proto_destroy (&self);
                return NULL;
            }
            uint64_t uvalue = (uint64_t) strtoll (s, &es, 10);
            if (es != s+strlen (s)) {
                zsys_error ("content/fd: %s is not a number", s);
                zp_proto_destroy (&self);
                return NULL;
            }
            self->fd = uvalue;
            }
            {
            char *s = zconfig_get (content, "chunk", NULL);
            if (!s) {
                zp_proto_destroy (&self);
                return NULL;
            }
            byte *bvalue;
            BYTES_FROM_STR (bvalue, s);
            if (!bvalue) {
                zp_proto_destroy (&self);
                return NULL;
            }
            zchunk_t *chunk = zchunk_new (bvalue, strlen (s) / 2);
            free (bvalue);
            self->chunk = chunk;
            }
            break;
        case ZP_PROTO_EXEC_LIST_REPLY:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            zconfig_t *zstrings = zconfig_locate (content, "handles");
            if (zstrings) {
                zlist_t *strings = zlist_new ();
                zlist_autofree (strings);
                for (zconfig_t *child = zconfig_child (zstrings);
                                child != NULL;
                                child = zconfig_next (child))
                {
                    zlist_append (strings, zconfig_value (child));
                }
                self->handles = strings;
            }
            }
            break;
        case ZP_PROTO_EXEC_STATUS_GET:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            char *es = NULL;
            char *s = zconfig_get (content, "handle", NULL);
            if (!s) {
                zsys_error ("content/handle not found");
                zp_proto_destroy (&self);
                return NULL;
            }
            uint64_t uvalue = (uint64_t) strtoll (s, &es, 10);
            if (es != s+strlen (s)) {
                zsys_error ("content/handle: %s is not a number", s);
                zp_proto_destroy (&self);
                return NULL;
            }
            self->handle = uvalue;
            }
            break;
        case ZP_PROTO_EXEC_LIST:
            break;
        case ZP_PROTO_EXEC_CANCEL:
            content = zconfig_locate (config, "content");
            if (!content) {
                zsys_error ("Can't find 'content' section");
                zp_proto_destroy (&self);
                return NULL;
            }
            {
            char *es = NULL;
            char *s = zconfig_get (content, "handle", NULL);
            if (!s) {
                zsys_error ("content/handle not found");
                zp_proto_destroy (&self);
                return NULL;
            }
            uint64_t uvalue = (uint64_t) strtoll (s, &es, 10);
            if (es != s+strlen (s)) {
                zsys_error ("content/handle: %s is not a number", s);
                zp_proto_destroy (&self);
                return NULL;
            }
            self->handle = uvalue;
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
        if (self->args)
            zlist_destroy (&self->args);
        zhash_destroy (&self->env);
        zhash_destroy (&self->aux);
        zchunk_destroy (&self->chunk);
        if (self->handles)
            zlist_destroy (&self->handles);

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
    {
        zlist_t *lcopy = zlist_dup (zp_proto_args (other));
        zp_proto_set_args (copy, &lcopy);
    }
    {
        zhash_t *dup_hash = zhash_dup (zp_proto_env (other));
        zp_proto_set_env (copy, &dup_hash);
    }
    {
        zhash_t *dup_hash = zhash_dup (zp_proto_aux (other));
        zp_proto_set_aux (copy, &dup_hash);
    }
    zp_proto_set_handle (copy, zp_proto_handle (other));
    zp_proto_set_returncode (copy, zp_proto_returncode (other));
    zp_proto_set_fd (copy, zp_proto_fd (other));
    {
        zchunk_t *dup_chunk = zchunk_dup (zp_proto_chunk (other));
        zp_proto_set_chunk (copy, &dup_chunk);
    }
    {
        zlist_t *lcopy = zlist_dup (zp_proto_handles (other));
        zp_proto_set_handles (copy, &lcopy);
    }

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

        case ZP_PROTO_EXEC:
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                zlist_destroy (&self->args);
                self->args = zlist_new ();
                zlist_autofree (self->args);
                while (list_size--) {
                    char *string = NULL;
                    GET_LONGSTR (string);
                    zlist_append (self->args, string);
                    free (string);
                }
            }
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                zhash_destroy (&self->env);
                self->env = zhash_new ();
                zhash_autofree (self->env);
                while (hash_size--) {
                    char key [256];
                    char *value = NULL;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->env, key, value);
                    free (value);
                }
            }
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                zhash_destroy (&self->aux);
                self->aux = zhash_new ();
                zhash_autofree (self->aux);
                while (hash_size--) {
                    char key [256];
                    char *value = NULL;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->aux, key, value);
                    free (value);
                }
            }
            break;

        case ZP_PROTO_EXEC_STATUS_REPLY:
            GET_NUMBER4 (self->handle);
            GET_NUMBER2 (self->returncode);
            break;

        case ZP_PROTO_EXEC_CHUNK:
            GET_NUMBER4 (self->handle);
            GET_NUMBER4 (self->fd);
            {
                size_t chunk_size;
                GET_NUMBER4 (chunk_size);
                if (self->needle + chunk_size > (self->ceiling)) {
                    zsys_warning ("zp_proto: chunk is missing data");
                    rc = -2;    //  Malformed
                    goto malformed;
                }
                zchunk_destroy (&self->chunk);
                self->chunk = zchunk_new (self->needle, chunk_size);
                self->needle += chunk_size;
            }
            break;

        case ZP_PROTO_EXEC_LIST_REPLY:
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                zlist_destroy (&self->handles);
                self->handles = zlist_new ();
                zlist_autofree (self->handles);
                while (list_size--) {
                    char *string = NULL;
                    GET_LONGSTR (string);
                    zlist_append (self->handles, string);
                    free (string);
                }
            }
            break;

        case ZP_PROTO_EXEC_STATUS_GET:
            GET_NUMBER4 (self->handle);
            break;

        case ZP_PROTO_EXEC_LIST:
            break;

        case ZP_PROTO_EXEC_CANCEL:
            GET_NUMBER4 (self->handle);
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
        case ZP_PROTO_EXEC:
            frame_size += 4;            //  Size is 4 octets
            if (self->args) {
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    frame_size += 4 + strlen (args);
                    args = (char *) zlist_next (self->args);
                }
            }
            frame_size += 4;            //  Size is 4 octets
            if (self->env) {
                self->env_bytes = 0;
                char *item = (char *) zhash_first (self->env);
                while (item) {
                    self->env_bytes += 1 + strlen (zhash_cursor (self->env));
                    self->env_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->env);
                }
            }
            frame_size += self->env_bytes;
            frame_size += 4;            //  Size is 4 octets
            if (self->aux) {
                self->aux_bytes = 0;
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    self->aux_bytes += 1 + strlen (zhash_cursor (self->aux));
                    self->aux_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            frame_size += self->aux_bytes;
            break;
        case ZP_PROTO_EXEC_STATUS_REPLY:
            frame_size += 4;            //  handle
            frame_size += 2;            //  returncode
            break;
        case ZP_PROTO_EXEC_CHUNK:
            frame_size += 4;            //  handle
            frame_size += 4;            //  fd
            frame_size += 4;            //  Size is 4 octets
            if (self->chunk)
                frame_size += zchunk_size (self->chunk);
            break;
        case ZP_PROTO_EXEC_LIST_REPLY:
            frame_size += 4;            //  Size is 4 octets
            if (self->handles) {
                char *handles = (char *) zlist_first (self->handles);
                while (handles) {
                    frame_size += 4 + strlen (handles);
                    handles = (char *) zlist_next (self->handles);
                }
            }
            break;
        case ZP_PROTO_EXEC_STATUS_GET:
            frame_size += 4;            //  handle
            break;
        case ZP_PROTO_EXEC_CANCEL:
            frame_size += 4;            //  handle
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

        case ZP_PROTO_EXEC:
            if (self->args) {
                PUT_NUMBER4 (zlist_size (self->args));
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    PUT_LONGSTR (args);
                    args = (char *) zlist_next (self->args);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            if (self->env) {
                PUT_NUMBER4 (zhash_size (self->env));
                char *item = (char *) zhash_first (self->env);
                while (item) {
                    PUT_STRING (zhash_cursor (self->env));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->env);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty hash
            if (self->aux) {
                PUT_NUMBER4 (zhash_size (self->aux));
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    PUT_STRING (zhash_cursor (self->aux));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty hash
            break;

        case ZP_PROTO_EXEC_STATUS_REPLY:
            PUT_NUMBER4 (self->handle);
            PUT_NUMBER2 (self->returncode);
            break;

        case ZP_PROTO_EXEC_CHUNK:
            PUT_NUMBER4 (self->handle);
            PUT_NUMBER4 (self->fd);
            if (self->chunk) {
                PUT_NUMBER4 (zchunk_size (self->chunk));
                memcpy (self->needle,
                        zchunk_data (self->chunk),
                        zchunk_size (self->chunk));
                self->needle += zchunk_size (self->chunk);
            }
            else
                PUT_NUMBER4 (0);    //  Empty chunk
            break;

        case ZP_PROTO_EXEC_LIST_REPLY:
            if (self->handles) {
                PUT_NUMBER4 (zlist_size (self->handles));
                char *handles = (char *) zlist_first (self->handles);
                while (handles) {
                    PUT_LONGSTR (handles);
                    handles = (char *) zlist_next (self->handles);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            break;

        case ZP_PROTO_EXEC_STATUS_GET:
            PUT_NUMBER4 (self->handle);
            break;

        case ZP_PROTO_EXEC_CANCEL:
            PUT_NUMBER4 (self->handle);
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

        case ZP_PROTO_EXEC:
            zsys_debug ("ZP_PROTO_EXEC:");
            zsys_debug ("    args=");
            if (self->args) {
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    zsys_debug ("        '%s'", args);
                    args = (char *) zlist_next (self->args);
                }
            }
            zsys_debug ("    env=");
            if (self->env) {
                char *item = (char *) zhash_first (self->env);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->env), item);
                    item = (char *) zhash_next (self->env);
                }
            }
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    aux=");
            if (self->aux) {
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->aux), item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;

        case ZP_PROTO_EXEC_STATUS_REPLY:
            zsys_debug ("ZP_PROTO_EXEC_STATUS_REPLY:");
            zsys_debug ("    handle=%ld", (long) self->handle);
            zsys_debug ("    returncode=%ld", (long) self->returncode);
            break;

        case ZP_PROTO_EXEC_CHUNK:
            zsys_debug ("ZP_PROTO_EXEC_CHUNK:");
            zsys_debug ("    handle=%ld", (long) self->handle);
            zsys_debug ("    fd=%ld", (long) self->fd);
            zsys_debug ("    chunk=[ ... ]");
            break;

        case ZP_PROTO_EXEC_LIST_REPLY:
            zsys_debug ("ZP_PROTO_EXEC_LIST_REPLY:");
            zsys_debug ("    handles=");
            if (self->handles) {
                char *handles = (char *) zlist_first (self->handles);
                while (handles) {
                    zsys_debug ("        '%s'", handles);
                    handles = (char *) zlist_next (self->handles);
                }
            }
            break;

        case ZP_PROTO_EXEC_STATUS_GET:
            zsys_debug ("ZP_PROTO_EXEC_STATUS_GET:");
            zsys_debug ("    handle=%ld", (long) self->handle);
            break;

        case ZP_PROTO_EXEC_LIST:
            zsys_debug ("ZP_PROTO_EXEC_LIST:");
            break;

        case ZP_PROTO_EXEC_CANCEL:
            zsys_debug ("ZP_PROTO_EXEC_CANCEL:");
            zsys_debug ("    handle=%ld", (long) self->handle);
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
        case ZP_PROTO_EXEC:
        {
            zconfig_put (root, "message", "ZP_PROTO_EXEC");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            if (self->args) {
                zconfig_t *strings = zconfig_new ("args", config);
                size_t i = 0;
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    char *key = zsys_sprintf ("%zu", i);
                    zconfig_putf (strings, key, "%s", args);
                    zstr_free (&key);
                    i++;
                    args = (char *) zlist_next (self->args);
                }
            }
            if (self->env) {
                zconfig_t *hash = zconfig_new ("env", config);
                char *item = (char *) zhash_first (self->env);
                while (item) {
                    zconfig_putf (hash, zhash_cursor (self->env), "%s", item);
                    item = (char *) zhash_next (self->env);
                }
            }
            if (self->aux) {
                zconfig_t *hash = zconfig_new ("aux", config);
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    zconfig_putf (hash, zhash_cursor (self->aux), "%s", item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            break;
            }
        case ZP_PROTO_EXEC_STATUS_REPLY:
        {
            zconfig_put (root, "message", "ZP_PROTO_EXEC_STATUS_REPLY");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            zconfig_putf (config, "handle", "%ld", (long) self->handle);
            zconfig_putf (config, "returncode", "%ld", (long) self->returncode);
            break;
            }
        case ZP_PROTO_EXEC_CHUNK:
        {
            zconfig_put (root, "message", "ZP_PROTO_EXEC_CHUNK");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            zconfig_putf (config, "handle", "%ld", (long) self->handle);
            zconfig_putf (config, "fd", "%ld", (long) self->fd);
            {
            char *hex = NULL;
            STR_FROM_BYTES (hex, zchunk_data (self->chunk), zchunk_size (self->chunk));
            zconfig_putf (config, "chunk", "%s", hex);
            zstr_free (&hex);
            }
            break;
            }
        case ZP_PROTO_EXEC_LIST_REPLY:
        {
            zconfig_put (root, "message", "ZP_PROTO_EXEC_LIST_REPLY");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            if (self->handles) {
                zconfig_t *strings = zconfig_new ("handles", config);
                size_t i = 0;
                char *handles = (char *) zlist_first (self->handles);
                while (handles) {
                    char *key = zsys_sprintf ("%zu", i);
                    zconfig_putf (strings, key, "%s", handles);
                    zstr_free (&key);
                    i++;
                    handles = (char *) zlist_next (self->handles);
                }
            }
            break;
            }
        case ZP_PROTO_EXEC_STATUS_GET:
        {
            zconfig_put (root, "message", "ZP_PROTO_EXEC_STATUS_GET");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            zconfig_putf (config, "handle", "%ld", (long) self->handle);
            break;
            }
        case ZP_PROTO_EXEC_LIST:
        {
            zconfig_put (root, "message", "ZP_PROTO_EXEC_LIST");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            break;
            }
        case ZP_PROTO_EXEC_CANCEL:
        {
            zconfig_put (root, "message", "ZP_PROTO_EXEC_CANCEL");

            if (self->routing_id) {
                char *hex = NULL;
                STR_FROM_BYTES (hex, zframe_data (self->routing_id), zframe_size (self->routing_id));
                zconfig_putf (root, "routing_id", "%s", hex);
                zstr_free (&hex);
            }

            zconfig_t *config = zconfig_new ("content", root);
            zconfig_putf (config, "handle", "%ld", (long) self->handle);
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
        case ZP_PROTO_EXEC:
            return ("EXEC");
            break;
        case ZP_PROTO_EXEC_STATUS_REPLY:
            return ("EXEC_STATUS_REPLY");
            break;
        case ZP_PROTO_EXEC_CHUNK:
            return ("EXEC_CHUNK");
            break;
        case ZP_PROTO_EXEC_LIST_REPLY:
            return ("EXEC_LIST_REPLY");
            break;
        case ZP_PROTO_EXEC_STATUS_GET:
            return ("EXEC_STATUS_GET");
            break;
        case ZP_PROTO_EXEC_LIST:
            return ("EXEC_LIST");
            break;
        case ZP_PROTO_EXEC_CANCEL:
            return ("EXEC_CANCEL");
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
//  Get the args field, without transferring ownership

zlist_t *
zp_proto_args (zp_proto_t *self)
{
    assert (self);
    return self->args;
}

//  Get the args field and transfer ownership to caller

zlist_t *
zp_proto_get_args (zp_proto_t *self)
{
    assert (self);
    zlist_t *args = self->args;
    self->args = NULL;
    return args;
}

//  Set the args field, transferring ownership from caller

void
zp_proto_set_args (zp_proto_t *self, zlist_t **args_p)
{
    assert (self);
    assert (args_p);
    zlist_destroy (&self->args);
    self->args = *args_p;
    *args_p = NULL;
}



//  --------------------------------------------------------------------------
//  Get the env field without transferring ownership

zhash_t *
zp_proto_env (zp_proto_t *self)
{
    assert (self);
    return self->env;
}

//  Get the env field and transfer ownership to caller

zhash_t *
zp_proto_get_env (zp_proto_t *self)
{
    zhash_t *env = self->env;
    self->env = NULL;
    return env;
}

//  Set the env field, transferring ownership from caller

void
zp_proto_set_env (zp_proto_t *self, zhash_t **env_p)
{
    assert (self);
    assert (env_p);
    zhash_destroy (&self->env);
    self->env = *env_p;
    *env_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get the aux field without transferring ownership

zhash_t *
zp_proto_aux (zp_proto_t *self)
{
    assert (self);
    return self->aux;
}

//  Get the aux field and transfer ownership to caller

zhash_t *
zp_proto_get_aux (zp_proto_t *self)
{
    zhash_t *aux = self->aux;
    self->aux = NULL;
    return aux;
}

//  Set the aux field, transferring ownership from caller

void
zp_proto_set_aux (zp_proto_t *self, zhash_t **aux_p)
{
    assert (self);
    assert (aux_p);
    zhash_destroy (&self->aux);
    self->aux = *aux_p;
    *aux_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the handle field

uint32_t
zp_proto_handle (zp_proto_t *self)
{
    assert (self);
    return self->handle;
}

void
zp_proto_set_handle (zp_proto_t *self, uint32_t handle)
{
    assert (self);
    self->handle = handle;
}


//  --------------------------------------------------------------------------
//  Get/set the returncode field

uint16_t
zp_proto_returncode (zp_proto_t *self)
{
    assert (self);
    return self->returncode;
}

void
zp_proto_set_returncode (zp_proto_t *self, uint16_t returncode)
{
    assert (self);
    self->returncode = returncode;
}


//  --------------------------------------------------------------------------
//  Get/set the fd field

uint32_t
zp_proto_fd (zp_proto_t *self)
{
    assert (self);
    return self->fd;
}

void
zp_proto_set_fd (zp_proto_t *self, uint32_t fd)
{
    assert (self);
    self->fd = fd;
}


//  --------------------------------------------------------------------------
//  Get the chunk field without transferring ownership

zchunk_t *
zp_proto_chunk (zp_proto_t *self)
{
    assert (self);
    return self->chunk;
}

//  Get the chunk field and transfer ownership to caller

zchunk_t *
zp_proto_get_chunk (zp_proto_t *self)
{
    zchunk_t *chunk = self->chunk;
    self->chunk = NULL;
    return chunk;
}

//  Set the chunk field, transferring ownership from caller

void
zp_proto_set_chunk (zp_proto_t *self, zchunk_t **chunk_p)
{
    assert (self);
    assert (chunk_p);
    zchunk_destroy (&self->chunk);
    self->chunk = *chunk_p;
    *chunk_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get the handles field, without transferring ownership

zlist_t *
zp_proto_handles (zp_proto_t *self)
{
    assert (self);
    return self->handles;
}

//  Get the handles field and transfer ownership to caller

zlist_t *
zp_proto_get_handles (zp_proto_t *self)
{
    assert (self);
    zlist_t *handles = self->handles;
    self->handles = NULL;
    return handles;
}

//  Set the handles field, transferring ownership from caller

void
zp_proto_set_handles (zp_proto_t *self, zlist_t **handles_p)
{
    assert (self);
    assert (handles_p);
    zlist_destroy (&self->handles);
    self->handles = *handles_p;
    *handles_p = NULL;
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
    zp_proto_set_id (self, ZP_PROTO_EXEC);

    zlist_t *exec_args = zlist_new ();
    zlist_append (exec_args, "Name: Brutus");
    zlist_append (exec_args, "Age: 43");
    zp_proto_set_args (self, &exec_args);
    zhash_t *exec_env = zhash_new ();
    zhash_insert (exec_env, "Name", "Brutus");
    zp_proto_set_env (self, &exec_env);
    zhash_t *exec_aux = zhash_new ();
    zhash_insert (exec_aux, "Name", "Brutus");
    zp_proto_set_aux (self, &exec_aux);
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
        zlist_t *args = zp_proto_get_args (self);
        assert (args);
        assert (zlist_size (args) == 2);
        assert (streq ((char *) zlist_first (args), "Name: Brutus"));
        assert (streq ((char *) zlist_next (args), "Age: 43"));
        zlist_destroy (&args);
        zlist_destroy (&exec_args);
        zhash_t *env = zp_proto_get_env (self);
        // Order of values is not guaranted
        assert (zhash_size (env) == 1);
        assert (streq ((char *) zhash_first (env), "Brutus"));
        assert (streq ((char *) zhash_cursor (env), "Name"));
        zhash_destroy (&env);
        if (instance == 2)
            zhash_destroy (&exec_env);
        zhash_t *aux = zp_proto_get_aux (self);
        // Order of values is not guaranted
        assert (zhash_size (aux) == 1);
        assert (streq ((char *) zhash_first (aux), "Brutus"));
        assert (streq ((char *) zhash_cursor (aux), "Name"));
        zhash_destroy (&aux);
        if (instance == 2)
            zhash_destroy (&exec_aux);
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_EXEC_STATUS_REPLY);

    zp_proto_set_handle (self, 123);
    zp_proto_set_returncode (self, 123);
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
        assert (zp_proto_handle (self) == 123);
        assert (zp_proto_returncode (self) == 123);
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_EXEC_CHUNK);

    zp_proto_set_handle (self, 123);
    zp_proto_set_fd (self, 123);
    zchunk_t *exec_chunk_chunk = zchunk_new ("Captcha Diem", 12);
    zp_proto_set_chunk (self, &exec_chunk_chunk);
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
        assert (zp_proto_handle (self) == 123);
        assert (zp_proto_fd (self) == 123);
        assert (memcmp (zchunk_data (zp_proto_chunk (self)), "Captcha Diem", 12) == 0);
        if (instance == 2)
            zchunk_destroy (&exec_chunk_chunk);
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_EXEC_LIST_REPLY);

    zlist_t *exec_list_reply_handles = zlist_new ();
    zlist_append (exec_list_reply_handles, "Name: Brutus");
    zlist_append (exec_list_reply_handles, "Age: 43");
    zp_proto_set_handles (self, &exec_list_reply_handles);
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
        zlist_t *handles = zp_proto_get_handles (self);
        assert (handles);
        assert (zlist_size (handles) == 2);
        assert (streq ((char *) zlist_first (handles), "Name: Brutus"));
        assert (streq ((char *) zlist_next (handles), "Age: 43"));
        zlist_destroy (&handles);
        zlist_destroy (&exec_list_reply_handles);
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_EXEC_STATUS_GET);

    zp_proto_set_handle (self, 123);
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
        assert (zp_proto_handle (self) == 123);
        if (instance == 2) {
            zp_proto_destroy (&self);
            self = self_temp;
        }
    }
    zp_proto_set_id (self, ZP_PROTO_EXEC_LIST);

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
    zp_proto_set_id (self, ZP_PROTO_EXEC_CANCEL);

    zp_proto_set_handle (self, 123);
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
        assert (zp_proto_handle (self) == 123);
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
