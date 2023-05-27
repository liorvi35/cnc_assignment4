"""
this file contains implementation simple ping program,
it sends an ICMP-ECHO-REQUEST and expects to receive ICMP-ECHO-RESPONSE.

:since: 27/05/2023

:Author: Lior Vinman
"""

import scapy.all as scapy
import time
import sys


def main():

    if len(sys.argv) != 2:
        print("Usage: python3 ./ping.py <IP>")
        sys.exit(1)

    seq = 0  # sequence number

    rtt_list = []  # list of Round-Trip-Time, in seconds

    num_recv, pkt_loss = 0, 0  # how many recv & lost

    try:

        op_start = time.time()

        while True:

            seq += 1
            ip_hdr = scapy.IP(dst=sys.argv[1])  # IP header
            icmp_hdr = scapy.ICMP(seq=seq)  # ICMP header
            ping_pkt = (ip_hdr / icmp_hdr)  # constructing the packet

            start = time.time()  # RTT-Start
            pong_pkt = scapy.sr1(ping_pkt, verbose=False)  # send & recv
            end = time.time()  # RTT-End

            if pong_pkt is not None: # if received response
                num_recv += 1
                print(f"{len(ping_pkt)} bytes from {sys.argv[1]}: icmp_seq={seq} ttl={pong_pkt['IP'].ttl}", "time={:.3f}s".format(end - start))

                rtt_list.append(end - start)

            time.sleep(1)

    except KeyboardInterrupt:

        op_end = time.time()

        t = (op_end - op_start)  # total operation time

        if len(rtt_list) == 0:

            print("\n",
                  f"--- {sys.argv[1]} statistics ---\n",
                  f"{seq} packets transmitted, 0 received, 100% loss,",
                  "time {:.3f}s\n".format(t),
                  "rtt min/avg/max/mdev = 0/0/0/0 s")

        else:

            avg_rtt = sum(rtt_list) / len(rtt_list)  # average RTT

            diffs = [abs(val - avg_rtt) for val in rtt_list]  # calc mdev (mean deviation)
            mdev = sum(diffs) / len(diffs)

            pkt_loss = (num_recv / seq) * 100  # % of how many packets received

            print("\n",
                  f"--- {sys.argv[1]} statistics ---\n",
                  f"{seq} packets transmitted, {num_recv} received, {int(100 - pkt_loss)}% loss,", "time {:.3f}s\n".format(t),
                  "rtt min/avg/max/mdev = {:.3f}/{:.3f}/{:.3f}/{:.3f} s".format(min(rtt_list), avg_rtt, max(rtt_list), mdev))

        sys.exit(0)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"\nError occurred: {e}")
        sys.exit(1)
