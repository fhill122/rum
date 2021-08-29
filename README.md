working in process


## Design Decision
- zmq tightly coupled, serialization decoupled
- serialization is done at language binding level
- zmq cpp wrapper is not hidden inside the core
- 

## todo
### stage I
- node only listen to master, and keeps its own sync, connects to its interested peers
-  node broadcast full sync info to master

### stage II
- ping server for each node