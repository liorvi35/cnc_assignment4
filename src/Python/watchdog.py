"""
this file contains implementation of watchdog counter,
serves as timer of better ping program.

:since: 27/05/2023

:Author: Lior Vinman
"""

import io
import os
import signal
import time
import socket


WATCHDOG_ADDR = ("127.0.0.1", 3000)  # Address of watchdog server
MAX_CONN = 300  # Maximal number of connections


try:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP) as sock:

        # setting IP & PORT be reusable, very important so won't be error each time when creating process
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)

        sock.bind(WATCHDOG_ADDR)
        sock.listen(MAX_CONN)

        client, address = sock.accept()

        with client:

            target_addr = client.recv(io.DEFAULT_BUFFER_SIZE)  # ICMP destination

            # implementing a very basic timer
            # if time passes (i.e. if chile process didn't have been killed) we assume that we didn't receive an answer
            for i in range(10):
                time.sleep(1)

            client.shutdown(socket.SHUT_RDWR)

    print(f"{target_addr.decode()} is not reachable.")

    os.kill(os.getppid(), signal.SIGTERM)

except KeyboardInterrupt:

    # if having a KI we don't do anything (because KI here = KI at main ping branch,
    #                                       and there we're taking care of error)

    pass

