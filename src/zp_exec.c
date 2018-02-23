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
    //  Properties!
    zp_proto_t *msg;            //  Message with arguments passed in
    zproc_t *proc;              //  Started process (if any)
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

    //  Initialize properties
    self->proc = NULL;

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

        //  Free actor properties
        zp_proto_destroy (&self->msg);
        zproc_destroy (&self->proc);

        //  Free object itself
        zpoller_destroy (&self->poller);
        free (self);
        *self_p = NULL;
    }
}


//  Start this actor. Return a value greater or equal to zero if initialization
//  was successful. Otherwise -1.

static int
zp_exec_start (zp_exec_t *self, zp_proto_t *msg)
{
    assert (self);
    assert (msg);
    assert (zp_proto_id (msg) == ZP_PROTO_EXEC);
    assert (!self->proc); // cannot pass START twice

    //  Add startup actions
    zproc_t *proc = zproc_new ();
    assert (proc);
    zlist_t *args = zp_proto_get_args (msg);
    zproc_set_args (proc, &args);
    zhash_t *env = zp_proto_get_env (msg);
    if (env)
        zproc_set_env (proc, &env);

    zproc_set_stdout (proc, NULL);
    zpoller_add (self->poller, zproc_stdout (proc));
    zpoller_add (self->poller, zproc_stderr (proc));

    self->msg = msg;
    self->proc = proc;

    zp_proto_set_returncode (self->msg, ZP_PROTO_RUNNING);
    return 0;
}


//  Stop this actor. Return a value greater or equal to zero if stopping
//  was successful. Otherwise -1.

static int
zp_exec_stop (zp_exec_t *self)
{
    assert (self);

    //  TODO: Add shutdown actions

    if (self->proc) {
        zproc_kill (self->proc, SIGKILL);
    }
    return 0;
}


//  Here we handle incoming message from the node

static void
zp_exec_recv_api (zp_exec_t *self)
{
    //  Get the whole message of the pipe in one go
    zp_proto_t *msg = NULL;
    char *command = NULL;
    int r = zsock_brecv (self->pipe, "sp", &command, &msg);
    if (r == -1)
       return;        //  Interrupted

    if (streq (command, "START"))
        zp_exec_start (self, msg);
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
    zp_proto_destroy (&msg);
}

//  Read chunk from sock and write to self->pipe
static void
zp_send_exec_chunk (zp_exec_t *self, void *sock, int fileno)
{
    zp_proto_t *msg = zp_proto_new ();
    zp_proto_set_id (msg, ZP_PROTO_EXEC_CHUNK);
    zp_proto_set_handle (msg, 42); // where to store handle?
    zp_proto_set_fd (msg, fileno);

    zchunk_t *chunk;
    int r = zsock_brecv (sock, "c", &chunk);
    if (r == 0)
        zp_proto_set_chunk (msg, &chunk);
    zsock_bsend (self->pipe, "sp", "CHUNk", msg);
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
        else
        if (self->proc && zproc_stdout (self->proc))
            zp_send_exec_chunk (self, zproc_stdout (self->proc), STDOUT_FILENO);
        else
        if (self->proc && zproc_stderr (self->proc))
            zp_send_exec_chunk (self, zproc_stderr (self->proc), STDERR_FILENO);
    }
    zp_exec_destroy (&self);
}

void
zp_exec_destructor (zactor_t *self)
{
    assert (self);
    if (zsock_bsend (self, "sp", "$TERM", NULL) == 0)
        zsock_wait (self);
}

//  --------------------------------------------------------------------------
//  Self test of this actor.

void
zp_exec_test (bool verbose)
{
    printf (" * zp_exec: ");
    //  @selftest
    //  Simple create/destroy test
    zactor_t *zp_exec = zactor_new (zp_exec_actor, NULL);
    assert (zp_exec);
    zactor_set_destructor (zp_exec, zp_exec_destructor);
    zactor_destroy (&zp_exec);
    //  @end

    printf ("OK\n");
}
