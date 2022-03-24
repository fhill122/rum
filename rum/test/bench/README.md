# naive bench

Result varies a lot in different platforms.  
Some tests on mac m1 shows that zmq is significantly faster in linux than mac, especially for ipc socket. ipc of zmq is even slower than tcp when message exceeds a certain size.

pingpong test:

