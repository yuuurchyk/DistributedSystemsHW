# DistributedSystemsHW

## TODO:
* introduce delays
* docker container

## Approach
Internal communication is done via sockets (each socket is (ideally) managed by a dedicated thread)

## Progress
* implemented replication with log concerns
* implemented the logic of secondary reconnecting to master after connection is dropped
* implemented simple healthcheck

## API

### Master Node
* ```GET /messages```
* ```GET /ping```
* ```POST /newMessage {"message": "string", "writeConcern": int}```

### Secondary Node
* ```GET /messages```

## Thread Model

### Master Node
* thread pool for secondary nodes (ideally 1 thread per socket)
* thread pool for http requests
* 1 thread for managing secondaries connect/disconnect

### Secondary Node
* 1 thread for communication with master
* thread pool for http requests

## CLI and Example Usage

```
>>> master-node --name mstr --http-port 8084 --comm-port 8086
[   0.000s] [mstr,17400] [            main.cpp:86  ]                                    [Info] : Using 0 internal communication threads
[   0.000s] [mstr,17400] [   iocontextpool.cpp:36  ][NetUtils::IOContextPool]           [Warn] : replacing size 0 with 1
[   0.000s] [mstr,17400] [            main.cpp:90  ]                                    [Info] : Using 3 http threads
[   0.001s] [mstr,17400] [            main.cpp:96  ]                                    [Info] : Internal communication port=8086
[   0.001s] [mstr,17400] [            main.cpp:100 ]                                    [Info] : Listening for http requests on port 8084

>>> secondary-node --name jerry --http-port 9090 --master-ip "192.168.0.100" --master-comm-port 8086
[   0.000s] [jerry,26400] [            main.cpp:86  ]                                    [Info] : Using 3 http threads
[   0.000s] [jerry,26400] [            main.cpp:92  ]                                    [Info] : Master internal communication, ip=192.168.0.100, port=8086
[   0.000s] [jerry,26400] [   secondarynode.cpp:54  ][SecondaryNode]                     [Info] : Reconnecting to master node...
[   0.000s] [jerry,26400] [            main.cpp:99  ]                                    [Info] : Listening for http requests on port 9090
[   0.000s] [jerry,1b640] [   secondarynode.cpp:88  ][SecondaryNode]                     [Info] : Opened socket to master node
[   0.000s] [jerry,1b640] [   secondarynode.cpp:154 ][SecondaryNode]                     [Info] : Sending get messages request
[   0.001s] [jerry,1b640] [   secondarynode.cpp:181 ][SecondaryNode]                     [Warn] : Recieved 0 message(s) from master node
[   0.001s] [jerry,1b640] [   secondarynode.cpp:197 ][SecondaryNode]                     [Info] : Sending secondary node ready request
[   0.085s] [jerry,1b640] [   secondarynode.cpp:222 ][SecondaryNode]                     [Info] : Secondary node is considered ready
```
