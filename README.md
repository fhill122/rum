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
