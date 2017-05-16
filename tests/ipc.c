/*
    Copyright (c) 2012 Martin Sustrik  All rights reserved.

*/

#include "../src/nn.h"
#include "../src/pair.h"
#include "../src/pubsub.h"
#include "../src/ipc.h"

#include "testutil.h"

/*  Tests IPC transport. */

#define SOCKET_ADDRESS "ipc://test.ipc"

int main ()
{
    int sb;
    int sc;
    int i;
    int s1, s2;

    int size;
    char * buf;

    /*  Try closing a IPC socket while it not connected. */
    sc = test_socket (AF_SP, NN_PAIR);
    test_connect (sc, SOCKET_ADDRESS);
    test_close (sc);

    /*  Open the socket anew. */
    sc = test_socket (AF_SP, NN_PAIR);
    test_connect (sc, SOCKET_ADDRESS);

    /*  Leave enough time for at least one re-connect attempt. */
    nn_sleep (200);

    sb = test_socket (AF_SP, NN_PAIR);
    test_bind (sb, SOCKET_ADDRESS);

    /*  Ping-pong test. */
    for (i = 0; i != 1; ++i) {
        test_send (sc, "0123456789012345678901234567890123456789");
        test_recv (sb, "0123456789012345678901234567890123456789");
        test_send (sb, "0123456789012345678901234567890123456789");
        test_recv (sc, "0123456789012345678901234567890123456789");
    }

    /*  Batch transfer test. */
    for (i = 0; i != 100; ++i) {
        test_send (sc, "XYZ");
    }
    for (i = 0; i != 100; ++i) {
        test_recv (sb, "XYZ");
    }

    /*  Send something large enough to trigger overlapped I/O on Windows. */
    size = 10000;
    buf = malloc (size);
    for (i = 0; i < size; ++i) {
        buf[i] = 48 + i % 10;
    }
    buf[size-1] = '\0';
    test_send (sc, buf);
    test_recv (sb, buf);
    free (buf);

    test_close (sc);
    test_close (sb);

    /*  Test whether connection rejection is handled decently. */
    sb = test_socket (AF_SP, NN_PAIR);
    test_bind (sb, SOCKET_ADDRESS);
    s1 = test_socket (AF_SP, NN_PAIR);
    test_connect (s1, SOCKET_ADDRESS);
    s2 = test_socket (AF_SP, NN_PAIR);
    test_connect (s2, SOCKET_ADDRESS);
    nn_sleep (100);
    test_close (s2);
    test_close (s1);
    test_close (sb);

    /*  Test two sockets binding to the same address. */
    sb = test_socket (AF_SP, NN_PAIR);
    test_bind (sb, SOCKET_ADDRESS);
    s1 = test_socket (AF_SP, NN_PAIR);
    test_bind (s1, SOCKET_ADDRESS);
    sc = test_socket (AF_SP, NN_PAIR);
    test_connect (sc, SOCKET_ADDRESS);
    nn_sleep (100);
    test_send (sb, "ABC");
    test_recv (sc, "ABC");
    test_close (sb);
    test_send (s1, "ABC");
    test_recv (sc, "ABC");   
    test_close (sc);
    test_close (s1);

    /*  Test closing a socket that is waiting to bind. */
    sb = test_socket (AF_SP, NN_PAIR);
    test_bind (sb, SOCKET_ADDRESS);
    nn_sleep (100);
    s1 = test_socket (AF_SP, NN_PAIR);
    test_bind (s1, SOCKET_ADDRESS);
    sc = test_socket (AF_SP, NN_PAIR);
    test_connect (sc, SOCKET_ADDRESS);
    nn_sleep (100);
    test_send (sb, "ABC");
    test_recv (sc, "ABC");
    test_close (s1);
    test_send (sb, "ABC");
    test_recv (sc, "ABC");
    test_close (sb);
    test_close (sc);

    return 0;
}

