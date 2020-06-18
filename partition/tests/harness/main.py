#!/usr/bin/python3

import leader
import threading

if __name__ == "__main__":
    threads = []

    threads.append(threading.Thread(target=leader.run, args=(leader.LeaderOkHandler, 5100)))
    threads.append(threading.Thread(target=leader.run, args=(leader.LeaderCloseImmediatelyHandler, 5102)))
    threads.append(threading.Thread(target=leader.run, args=(leader.LeaderWriteAndCloseHandler, 5103)))

    for t in threads:
        t.start()
    for t in threads:
        t.join()
