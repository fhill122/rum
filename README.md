working in process


## Design Decision
- zmq tightly coupled, serialization decoupled
- serialization is done at language binding level
- zmq cpp wrapper is not hidden inside the core

## cheashee
```shell
# check which process occupies master port
sudo lsof -i :12580
```

## todo
- there is only 1 server, but could exist multiple client, how to use pub/sub to deal with this?  
  multiple pub in server, each connect to a single client's subcontainer. we need ping to wait until connection established
