#!/usr/bin/python3

import leader
import threading

if __name__ == "__main__":
    threads = []

    threads.append(threading.Thread(target=leader.leader_ok))
    threads.append(threading.Thread(target=leader.leader_close_immediately))
    threads.append(threading.Thread(target=leader.leader_write_and_close))

    for t in threads:
        t.start()
    for t in threads:
        t.join()
