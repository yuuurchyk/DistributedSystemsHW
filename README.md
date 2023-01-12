# DistributedSystemsHW

## TODO:
* docker container

## Approach
Internal communication is done via sockets (each socket is (ideally) managed by a dedicated thread)

## Progress
* implemented replication with log write concerns
* implemented the logic of secondary reconnecting to master after connection is dropped
* implemented artificial delays

## API

### Master Node
* ```GET /messages```
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

Here is an example of setting up 1 master and 1 secondary node and sending request to master with write concern = 2

```
root@pop-os:/# master-node --name mstrr --http-port 6008 --comm-port 4000
[   0.001s] [mstrr,3eb00] [                      main.cpp:86  ]                                    [Info] : Using 3 http threads
[   0.001s] [mstrr,3eb00] [                      main.cpp:95  ]                                    [Info] : Internal communication port=4000
[   0.001s] [mstrr,3eb00] [                      main.cpp:96  ]                                    [Info] : Using 0 internal communication threads
[   0.001s] [mstrr,3eb00] [             iocontextpool.cpp:36  ][NetUtils::IOContextPool]           [Warn] : replacing size 0 with 1
[   0.001s] [mstrr,3eb00] [                      main.cpp:117 ]                                    [Info] : Listening for http requests on port 6008
[   3.084s] [mstrr,32640] [            socketacceptor.cpp:107 ][NetUtils::SocketAcceptor]          [Info] : Accepting incoming connection
[   3.084s] [mstrr,33640] [                masternode.cpp:67  ][MasterNode]                        [Info] : adding secondary: id=0
[   4.831s] [mstrr,31640] [                  endpoint.cpp:249 ][Proto2::Endpoint,id=sec_0]         [Info] : incoming_getMessages(startMsgId=0)
[   5.836s] [mstrr,31640] [                  endpoint.cpp:249 ][Proto2::Endpoint,id=sec_0]         [Info] : incoming_getMessages(startMsgId=0)
[   6.881s] [mstrr,31640] [                  endpoint.cpp:285 ][Proto2::Endpoint,id=sec_0]         [Info] : incoming_secondaryNodeReady(secondaryNodeName='jerry')
[   6.882s] [mstrr,33640] [                masternode.cpp:146 ][MasterNode]                        [Info] : registering node 0 as ready, friendlyName: jerry
[  11.142s] [mstrr,3eb00] [            socketacceptor.cpp:77  ][NetUtils::SocketAcceptor]          [Info] : Accepting incoming connection
[  11.143s] [mstrr,36640] [                  endpoint.cpp:119 ][Proto2::Endpoint,id=sec_0]         [Info] : sending addMessage(msgId=0, msg='hello') (artificially delayed by 1509ms)
[  12.643s] [mstrr,36640] [         addmessagerequest.cpp:192 ][AddMessageRequest,id=0]            [Warn] : Recieved error from secondary node 0
[  12.643s] [mstrr,36640] [                  endpoint.cpp:119 ][Proto2::Endpoint,id=sec_0]         [Info] : sending addMessage(msgId=0, msg='hello') (artificially delayed by 1726ms)
[  12.652s] [mstrr,31640] [  outcomingrequestsmanager.cpp:150 ][Proto2::OutcomingRequestsManager,id=sec_0] [Warn] : Recieved response for request 1, but it is not marked as pending
[  14.144s] [mstrr,36640] [         addmessagerequest.cpp:192 ][AddMessageRequest,id=0]            [Warn] : Recieved error from secondary node 0
[  14.144s] [mstrr,36640] [                  endpoint.cpp:119 ][Proto2::Endpoint,id=sec_0]         [Info] : sending addMessage(msgId=0, msg='hello') (artificially delayed by 1605ms)
[  14.370s] [mstrr,31640] [  outcomingrequestsmanager.cpp:150 ][Proto2::OutcomingRequestsManager,id=sec_0] [Warn] : Recieved response for request 2, but it is not marked as pending
[  15.644s] [mstrr,36640] [         addmessagerequest.cpp:192 ][AddMessageRequest,id=0]            [Warn] : Recieved error from secondary node 0
[  15.644s] [mstrr,36640] [                  endpoint.cpp:119 ][Proto2::Endpoint,id=sec_0]         [Info] : sending addMessage(msgId=0, msg='hello') (artificially delayed by 1279ms)
[  15.749s] [mstrr,31640] [  outcomingrequestsmanager.cpp:150 ][Proto2::OutcomingRequestsManager,id=sec_0] [Warn] : Recieved response for request 3, but it is not marked as pending
[  16.924s] [mstrr,36640] [         addmessagerequest.cpp:181 ][AddMessageRequest,id=0]            [Info] : Recieved ok response from secondary node 0
[  16.924s] [mstrr,36640] [         addmessagerequest.cpp:147 ][AddMessageRequest,id=0]            [Info] : marking request as success
[  16.924s] [mstrr,36640] [         addmessagerequest.cpp:62  ][AddMessageRequest,id=0]            [Info] : all secondaries answered, destroying request






root@pop-os:/# secondary-node --http-port 6006 --master-comm-port 4000 --master-ip 127.0.0.1 --name jerry
[   0.000s] [jerry,acb00] [                      main.cpp:87  ]                                    [Info] : Master internal communication, ip=127.0.0.1, port=4000
[   0.000s] [jerry,acb00] [             secondarynode.cpp:50  ][SecondaryNode]                     [Info] : reconnecting to master node
[   0.000s] [jerry,acb00] [                      main.cpp:96  ]                                    [Info] : Using 3 http threads
[   0.000s] [jerry,acb00] [                      main.cpp:100 ]                                    [Info] : Listening for http requests on port 6006
[   0.000s] [jerry,acb00] [             secondarynode.cpp:77  ][SecondaryNode]                     [Info] : connected to master node, establising endpoint
[   0.010s] [jerry,acb00] [             mastersession.cpp:69  ][MasterSession]                     [Info] : asking master for messages, starting from messageId=0
[   0.010s] [jerry,acb00] [                  endpoint.cpp:136 ][Proto2::Endpoint,id=master]        [Info] : sending getMessages(startMsgId=0) (artificially delayed by 1738ms)
[   1.510s] [jerry,acb00] [             mastersession.cpp:89  ][MasterSession]                     [Warn] : Failed to recieve messages from master, retrying
[   1.510s] [jerry,acb00] [             mastersession.cpp:69  ][MasterSession]                     [Info] : asking master for messages, starting from messageId=0
[   1.510s] [jerry,acb00] [                  endpoint.cpp:136 ][Proto2::Endpoint,id=master]        [Info] : sending getMessages(startMsgId=0) (artificially delayed by 1242ms)
[   1.748s] [jerry,acb00] [  outcomingrequestsmanager.cpp:150 ][Proto2::OutcomingRequestsManager,id=master] [Warn] : Recieved response for request 1, but it is not marked as pending
[   2.752s] [jerry,acb00] [             mastersession.cpp:82  ][MasterSession]                     [Info] : Successfully retrieved messages from master, filling storage
[   2.753s] [jerry,acb00] [             secondarynode.cpp:109 ][SecondaryNode]                     [Info] : master session operational
[   2.753s] [jerry,acb00] [             mastersession.cpp:97  ][MasterSession]                     [Info] : sending friendly name to master
[   2.753s] [jerry,acb00] [                  endpoint.cpp:170 ][Proto2::Endpoint,id=master]        [Info] : sending secondaryNodeReady(secondaryName='jerry') (artificially delayed by 1045ms)
[   3.798s] [jerry,acb00] [             mastersession.cpp:110 ][MasterSession]                     [Info] : recieved ack from master on secondaryNodeReady request
[   9.568s] [jerry,acb00] [                  endpoint.cpp:231 ][Proto2::Endpoint,id=master]        [Info] : incoming_addMessage(msgId=0, msg='hello')
[  11.286s] [jerry,acb00] [                  endpoint.cpp:231 ][Proto2::Endpoint,id=master]        [Info] : incoming_addMessage(msgId=0, msg='hello')
[  11.286s] [jerry,acb00] [                   storage.cpp:10  ][Storage]                           [Warn] : failed to insert message with id 0, already present
[  12.665s] [jerry,acb00] [                  endpoint.cpp:231 ][Proto2::Endpoint,id=master]        [Info] : incoming_addMessage(msgId=0, msg='hello')
[  12.665s] [jerry,acb00] [                   storage.cpp:10  ][Storage]                           [Warn] : failed to insert message with id 0, already present
[  13.840s] [jerry,acb00] [                  endpoint.cpp:231 ][Proto2::Endpoint,id=master]        [Info] : incoming_addMessage(msgId=0, msg='hello')
[  13.840s] [jerry,acb00] [                   storage.cpp:10  ][Storage]                           [Warn] : failed to insert message with id 0, already present
```
