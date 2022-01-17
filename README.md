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
- rum returns handler now, is it really appropriate?
  In a multi component environment, a component may exist but leave its subscribers alive, and if further messages are received, callback would take in place in wrong memory.  
  maybe auto removal upon handler destruction?