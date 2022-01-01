working in process

todo



## Design Decision
- zmq tightly coupled, serialization decoupled
- serialization is done at language binding level
- zmq cpp wrapper is not hidden inside the core
- two message types, publihser type, callback type.  
  serializer provides:
  - publisher type -> zmq message
  - publisher type -> callback type
  - zmq message -> callback type  
  
  flatbuffers makes it so complicated, I hate it

## todo
### stage I
- node only listen to master, and keeps its own sync, connects to its interested peers
-  node broadcast full sync info to master

### stage II
- ping server for each node