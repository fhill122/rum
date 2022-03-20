working in progress


## Design Decision
- zmq tightly coupled, serialization decoupled
- serialization is done at language binding level

## cheashee
```shell
# check which process occupies master port
sudo lsof -i :12580
```

## Advantages over ros
Major:
- no master
- free serialization choices
- auto choose the best transportation among itc, ipc or tcp
- few dependencies (only libzmq is mandatory)

Misc:
- nodelet without any code modification
- service call with intra-process communication as well (nodelet like)
- safe guard publish types in build time

Why so harsh on ros? Coz I'm mean...

## todo
- there is only 1 server, but could exist multiple client, how to use pub/sub to deal with this?  
  multiple pub in server, each connect to a single client's subcontainer. we need ping to wait until connection established
- serialization should be in cpprum?
- rename ipc itc to intrap and interp
