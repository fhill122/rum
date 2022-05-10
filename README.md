working in progress


## Design Decision
- zmq tightly coupled, serialization decoupled
- serialization is done at language binding level

## cheashee
```shell
# check which process occupies master port
sudo lsof -i :10086
```

## Advantages over ros
Major:
- no master
- free serialization choices
- auto choose the best transportation among intra-process, ipc or tcp
- few dependencies (only libzmq is mandatory)

Misc:
- nodelet without any code modification
- service call with intra-process communication as well (nodelet like)
- safe guard publish types in build time

Why so harsh on ros? Coz I'm mean...

## todo
- serialization should be in cpprum?
- moodycamel queue with token
- try udp discovery:
  https://zguide.zeromq.org/docs/chapter8/#Cooperative-Discovery-Using-UDP-Broadcasts
  https://github.com/computersarecool/cpp_sockets