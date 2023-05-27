"""
this file contains implementation of advanced ping program,
firstly is constructs and sends an ICMP-ECHO-REQUEST packet then opens a TCP connection with remote
Watchdog server, which serves as time counter, if program doesn't receive an ICMP-ECHO-RESPONSE within
10 seconds, the watchdog terminates the program.

:since: 27/05/2023

:Author: Lior Vinman
"""


import os
import random
import signal
import socket
import struct
import sys
import io
import time


WATCHDOG_ADDR = ("127.0.0.1", 3000)  # Address of watchdog server


def get_ttl(packet) -> int:
    """
    This function extracts the TTL (Time-To-Live) field from ICMP-ECHO-RESPONSE packet.

    :param packet: Packet to extract from.
    :type packet: bytes

    :return: TTL field.
    :rtype: int
    """

    ip_header = packet[:20]
    return struct.unpack('!B', ip_header[8:9])[0]


def build_packet(seq) -> bytes:
    """
    This function creates a ping (ICMP-ECHO-REQUEST) packet.

    :param seq: A sequence number for the packet.
    :type seq: int

    :return: A valid ping packet, ready to send.
    :rtype: bytes
    """

    def checksum_data(buff) -> int:
        """
        This function calculates checksum for the given buffer,
        used to calculate the checksum field in the ICMP header for a valid packet sending.

        :param buff: Buffer for calculating checksum.
        :type buff: bytes

        :return: The checksum for the calculated data.
        :rtype: int
        """
        checksum = 0

        if len(buff) % 2 != 0:
            buff += b"\x00"

        for i in range(0, len(buff), 2):
            chunk = buff[i:i+2]
            word = struct.unpack('!H', chunk)[0]
            checksum += word

        checksum = (checksum >> 16) + (checksum & 0xFFFF)
        checksum += checksum >> 16
        checksum = ~checksum & 0xFFFF
        return checksum

    icmp_type = 8
    icmp_code = 0
    icmp_checksum = 0
    icmp_identifier = random.randint(1, 65535)
    icmp_sequence = seq

    icmp_header = struct.pack("!BBHHH", icmp_type, icmp_code, icmp_checksum, icmp_identifier, icmp_sequence)

    icmp_checksum = checksum_data(icmp_header)

    icmp_header = struct.pack("!BBHHH", icmp_type, icmp_code, icmp_checksum, icmp_identifier, icmp_sequence)

    icmp_packet = icmp_header

    return icmp_packet


def main():

    if len(sys.argv) != 2:
        print("Usage: python3 ./better_ping.py <IP>.")
        sys.exit(1)

    seq = 0

    rtt_list = []

    op_start = time.time()

    with socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP) as sock:

        try:

            while True:

                seq += 1

                ping_pkt = build_packet(seq)

                start = time.time()

                sock.sendto(ping_pkt, (sys.argv[1], 0))

                pid = os.fork()

                # child branch -> pid is 0
                # starting the watchdog counter at child
                if pid == 0:
                    os.execvp("python3", ["python3", "watchdog.py"])

                # parent branch -> pid is NOT 0
                # at parent opening a TCP connection & waiting for ICMP response
                else:

                    time.sleep(1)  # a second so WD could start up

                    with socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP) as watchdog:

                        watchdog.connect(WATCHDOG_ADDR)

                        watchdog.sendall(sys.argv[1].encode())

                        pong_pkt, res_addr = sock.recvfrom(io.DEFAULT_BUFFER_SIZE)

                        end = time.time()

                        rtt_list.append((end - start) - 1)

                        watchdog.shutdown(socket.SHUT_RDWR)

                    os.kill(pid, signal.SIGTERM)
                    os.wait()

                print(f"{len(pong_pkt)} bytes from {sys.argv[1]}: icmp_seq={seq} ttl={get_ttl(pong_pkt)}",
                      "time={:.5f}s".format((end - start) - 1))

        except KeyboardInterrupt:

            op_end = time.time()

            t = (op_end - op_start)

            avg_rtt = sum(rtt_list) / len(rtt_list)  # average RTT

            diffs = [abs(val - avg_rtt) for val in rtt_list]  # calc mdev (= mean deviation)
            mdev = sum(diffs) / len(diffs)

            print("\n",
                  f"--- {sys.argv[1]} statistics ---\n",
                  f"{seq - 1} packets transmitted, {seq - 1} received, 0% loss,",
                  "time {:.5f}s\n".format(t),
                  "rtt min/avg/max/mdev = {:.5f}/{:.5f}/{:.5f}/{:.5f} s".format(min(rtt_list), avg_rtt, max(rtt_list), mdev))
            sys.exit(0)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Error occurred: {e}.")
        sys.exit(1)
