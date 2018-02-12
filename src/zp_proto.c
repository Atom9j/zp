/*  =========================================================================
    zp_proto - class description

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
    zp_proto -
@discuss
@end
*/

#include "zp_classes.h"

//  Structure of our class

struct _zp_proto_t {
    int filler;     //  Declare class properties here
};


//  --------------------------------------------------------------------------
//  Create a new zp_proto

zp_proto_t *
zp_proto_new (void)
{
    zp_proto_t *self = (zp_proto_t *) zmalloc (sizeof (zp_proto_t));
    assert (self);
    //  Initialize class properties here
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
zp_proto_test (bool verbose)
{
    printf (" * zp_proto: ");

    //  @selftest
    //  Simple create/destroy test
    zp_proto_t *self = zp_proto_new ();
    assert (self);
    zp_proto_destroy (&self);
    //  @end
    printf ("OK\n");
}
