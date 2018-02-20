/*  =========================================================================
    zp_server - class description

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
    zp_server -
@discuss
@end
*/

#include "zp_classes.h"

//  Structure of our class

struct _zp_server_t {
    int filler;     //  Declare class properties here
};


//  --------------------------------------------------------------------------
//  Create a new zp_server

zp_server_t *
zp_server_new (void)
{
    zp_server_t *self = (zp_server_t *) zmalloc (sizeof (zp_server_t));
    assert (self);
    //  Initialize class properties here
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the zp_server

void
zp_server_destroy (zp_server_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zp_server_t *self = *self_p;
        //  Free class properties here
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

//  --------------------------------------------------------------------------
//  Self test of this class

// If your selftest reads SCMed fixture data, please keep it in
// src/selftest-ro; if your test creates filesystem objects, please
// do so under src/selftest-rw.
// The following pattern is suggested for C selftest code:
//    char *filename = NULL;
//    filename = zsys_sprintf ("%s/%s", SELFTEST_DIR_RO, "mytemplate.file");
//    assert (filename);
//    ... use the "filename" for I/O ...
//    zstr_free (&filename);
// This way the same "filename" variable can be reused for many subtests.
#define SELFTEST_DIR_RO "src/selftest-ro"
#define SELFTEST_DIR_RW "src/selftest-rw"

void
zp_server_test (bool verbose)
{
    printf (" * zp_server: ");

    //  @selftest
    //  Simple create/destroy test
    zp_server_t *self = zp_server_new ();
    assert (self);
    zp_server_destroy (&self);
    //  @end
    printf ("OK\n");
}


//  ---------------------------------------------------------------------------
//  register_new_client
//

static void
register_new_client (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  signal_command_invalid
//

static void
signal_command_invalid (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  exec_new_process
//

static void
exec_new_process (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  exec_status_get
//

static void
exec_status_get (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  exec_list
//

static void
exec_list (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  client_closed_connection
//

static void
client_closed_connection (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  deregister_the_client
//

static void
deregister_the_client (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  allow_time_to_settle
//

static void
allow_time_to_settle (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  client_expired
//

static void
client_expired (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  signal_operation_failed
//

static void
signal_operation_failed (client_t *self)
{

}


//  ---------------------------------------------------------------------------
//  client_had_exception
//

static void
client_had_exception (client_t *self)
{

}
