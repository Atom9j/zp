[![Build Status](https://travis-ci.org/zmonit/zp.svg?branch=master)](https://travis-ci.org/zmonit/zp)

# zp
ZeroMQ Pipes: connect programs through ZeroMQ from command line

TODO: read zguide - http://zguide.zeromq.org/page:all#Large-Scale-File-Publishing-FileMQ

## What zp does
Zp is all about reliable transfer data between endpoints. It utilize zeromq engine to make transport. By default it can send output of the command as well as content of files. It can be seen as an alternative for `ssh`/`scp` and `rsync`, however in this incarnation with nice and easy to use C API and a library.

> Note: this paper does not describe any actual system, just states the ideas to implement.

## The pipe
Connect two programs via `zp` as a unix pipe. `zp` command will act as openssh client and will run command on remote machine and pass results back

    cat /etc/passwd | zp $user@udp://localhost grep $user

## Async job run
While synchronous run is a great thing to do, there are cases you’d like to have an ability to run jobs in a background.

    $ zp --async $user@some.host long-running-process
    $ id=id

To get a list of running jobs

    $ zp jobs $user@some.host
    id1
    id2
    id3

To get status of running command

    $ zp job status $user@some.host id1
    id1
        status = running
        command_line = “long-running-process”
        stdout = connected
        stderr = disconnected

To monitor stdin/stderr

    $ zp job follow [--stdout/--stderr/--all] $user@some.host id

## File transfer
Use chunked transfer of files to host

    zpc -r files/ $user@some.host


## Architecture

There are going to be three main components

`zpd` - zp daemon, it will fork/clone for new request, handle the results, tracks the async jobs
`zpc` - zp copy, copy files
`zp` - zero pipe - run commands

`czmq` - High level ZeroMQ bindings - zauth, zproc, zactor, zloop, zargs, …
`zproject` - …
`zproto` - ... 

## The protocol

> Note: This is a DRAFT, the protocol can change as a part of further development process

Protocol would be rather simple (excluding async job feature)
Standard messages open/close, ping/pong
CHUNK/CHUNK_ACK - maybe better names

## Advantages over ssh/rsync

Q: Why should I replace my $tool of choice?
A: You SHALL NOT :-). Really, if you're happy with your tools, do not change.

However there's a humble list of advantages over existing tools

 * One tool to handle remote access + file transfer
 * Reliable chunked transport
 * Supports reconnection
 * More data flows patterns (thinkk of PUB/SUB)
 * Advanced encryption out of the box
 * C library libzp with a lot of language bindings
 * Lean, small and fast
 * ...
