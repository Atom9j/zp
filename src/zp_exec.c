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

/*
@header
    zp_exec - ZeroMQ pipes process execution engine
@discuss
@end
*/

#include "zp_classes.h"

//  Structure of our actor

struct _zp_exec_t {
    zsock_t *pipe;              //  Actor command pipe
    zpoller_t *poller;          //  Socket poller
    bool terminated;            //  Did caller ask us to quit?
    bool verbose;               //  Verbose logging enabled?
    //  TODO: Declare properties
};


//  --------------------------------------------------------------------------
//  Create a new zp_exec instance

static zp_exec_t *
zp_exec_new (zsock_t *pipe, void *args)
{
    zp_exec_t *self = (zp_exec_t *) zmalloc (sizeof (zp_exec_t));
    assert (self);

    self->pipe = pipe;
    self->terminated = false;
    self->poller = zpoller_new (self->pipe, NULL);

    //  TODO: Initialize properties

    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the zp_exec instance

static void
zp_exec_destroy (zp_exec_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zp_exec_t *self = *self_p;

        //  TODO: Free actor properties

        //  Free object itself
        zpoller_destroy (&self->poller);
        free (self);
        *self_p = NULL;
    }
}


//  Start this actor. Return a value greater or equal to zero if initialization
//  was successful. Otherwise -1.

static int
zp_exec_start (zp_exec_t *self)
{
    assert (self);

    //  TODO: Add startup actions

    return 0;
}


//  Stop this actor. Return a value greater or equal to zero if stopping
//  was successful. Otherwise -1.

static int
zp_exec_stop (zp_exec_t *self)
{
    assert (self);

    //  TODO: Add shutdown actions

    return 0;
}


//  Here we handle incoming message from the node

static void
zp_exec_recv_api (zp_exec_t *self)
{
    //  Get the whole message of the pipe in one go
    zmsg_t *request = zmsg_recv (self->pipe);
    if (!request)
       return;        //  Interrupted

    char *command = zmsg_popstr (request);
    if (streq (command, "START"))
        zp_exec_start (self);
    else
    if (streq (command, "STOP"))
        zp_exec_stop (self);
    else
    if (streq (command, "VERBOSE"))
        self->verbose = true;
    else
    if (streq (command, "$TERM"))
        //  The $TERM command is send by zactor_destroy() method
        self->terminated = true;
    else {
        zsys_error ("invalid command '%s'", command);
        assert (false);
    }
    zstr_free (&command);
    zmsg_destroy (&request);
}


//  --------------------------------------------------------------------------
//  This is the actor which runs in its own thread.

void
zp_exec_actor (zsock_t *pipe, void *args)
{
    zp_exec_t * self = zp_exec_new (pipe, args);
    if (!self)
        return;          //  Interrupted

    //  Signal actor successfully initiated
    zsock_signal (self->pipe, 0);

    while (!self->terminated) {
        zsock_t *which = (zsock_t *) zpoller_wait (self->poller, 0);
        if (which == self->pipe)
            zp_exec_recv_api (self);
       //  Add other sockets when you need them.
    }
    zp_exec_destroy (&self);
}

//  --------------------------------------------------------------------------
//  Self test of this actor.

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
zp_exec_test (bool verbose)
{
    printf (" * zp_exec: ");
    //  @selftest
    //  Simple create/destroy test
    zactor_t *zp_exec = zactor_new (zp_exec_actor, NULL);
    assert (zp_exec);

    zactor_destroy (&zp_exec);
    //  @end

    printf ("OK\n");
}
