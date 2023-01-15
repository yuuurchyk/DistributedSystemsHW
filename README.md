# DistributedSystemsHW

TODO: add problem description

## Docker Compose

```docker compose up``` launches the following containers:
* ```master``` (ip ```10.5.0.100```)
* ```bonnie``` (secondary 1, ip ```10.5.0.101```)
* ```clyde_``` (secondary 2, ip ```10.5.0.102```)

## API

### Master Node

* ```GET /messages```
* ```POST /newMessage``` with the following json body: ```{"message": "string", "writeConcern": unsigned int}```

### Secondary Node

* ```GET /messages```

## Implementation Details

* communication with secondary node happens via tcp socket (there is a single tcp connection associated with a partcular secondary where all the communication takes place)
* when sth is written through socket, a random delay between ```1000ms``` and ```2000ms``` is introduced. Note that response from the socket is considered timed out if there is no answer within ```1500ms```, so some requests will be resent (probably multiple times)
* secondary tries to reconnect to master if the connection is dropped
* when secondary connects to master, it queries the messages. After the messages have been recieved, secondary sends a ```SecondaryNodeReady``` request to master, which informs the master that this particular secondary has become operational

## Thread Model

### Master Node
* thread pool for secondary nodes internal communication (ideally 1 thread per socket)
* thread pool for http requests
* 1 thread for managing secondaries connect/disconnect

### Secondary Node
* 1 thread for communication with master internal communication
* thread pool for http requests

## CLI

```
>>> master-node --help
Options:
  -h [ --help ]           produce help message
  --name arg              friendly name of the node
  --http-port arg         http port
  --comm-port arg         internal communication port
  --http-workers arg (=3) number of http threads to use
  --comm-workers arg (=3) number of internal communication threads to use

>>> secondary-node --help
Options:
  -h [ --help ]           produce help message
  --name arg (=secondary) friendly name of the node
  --http-port arg         http port
  --http-workers arg (=3) number of http threads to use
  --master-ip arg         ip of the master node
  --master-comm-port arg  master internal communication port
```

## Example Usage (docker compose up)

```
clyde_  | [   0.000s] [clyde_,8cb00] [                      main.cpp:87  ]                                    [Info] : Master internal communication, ip=10.5.0.100, port=6000
clyde_  | [   0.000s] [clyde_,8cb00] [             secondarynode.cpp:51  ][SecondaryNode]                     [Info] : reconnecting to master node
clyde_  | [   0.000s] [clyde_,8cb00] [                      main.cpp:96  ]                                    [Info] : Using 3 http threads
clyde_  | [   0.000s] [clyde_,8cb00] [                      main.cpp:100 ]                                    [Info] : Listening for http requests on port 80
bonnie  | [   0.000s] [bonnie,ccb00] [                      main.cpp:87  ]                                    [Info] : Master internal communication, ip=10.5.0.100, port=6000
bonnie  | [   0.000s] [bonnie,ccb00] [             secondarynode.cpp:51  ][SecondaryNode]                     [Info] : reconnecting to master node
bonnie  | [   0.000s] [bonnie,ccb00] [                      main.cpp:96  ]                                    [Info] : Using 3 http threads
bonnie  | [   0.000s] [bonnie,ccb00] [                      main.cpp:100 ]                                    [Info] : Listening for http requests on port 80
bonnie  | [   0.000s] [bonnie,ccb00] [             secondarynode.cpp:65  ][SecondaryNode]                     [Info] : failed to connect to master, schedule reconnect
master  | [   0.000s] [master,d5b00] [                      main.cpp:86  ]                                    [Info] : Using 3 http threads
master  | [   0.000s] [master,d5b00] [                      main.cpp:95  ]                                    [Info] : Internal communication port=6000
master  | [   0.000s] [master,d5b00] [                      main.cpp:96  ]                                    [Info] : Using 3 internal communication threads
master  | [   0.000s] [master,d5b00] [                      main.cpp:117 ]                                    [Info] : Listening for http requests on port 80
master  | [   0.945s] [master,c9640] [            socketacceptor.cpp:107 ][NetUtils::SocketAcceptor]          [Info] : Accepting incoming connection
clyde_  | [   1.011s] [clyde_,8cb00] [             secondarynode.cpp:78  ][SecondaryNode]                     [Info] : connected to master node, establising endpoint
master  | [   0.945s] [master,ca640] [                masternode.cpp:67  ][MasterNode]                        [Info] : adding secondary: id=0
clyde_  | [   1.013s] [clyde_,8cb00] [             mastersession.cpp:68  ][MasterSession]                     [Info] : asking master for messages, starting from messageId=0
clyde_  | [   1.013s] [clyde_,8cb00] [                  endpoint.cpp:126 ][Proto::Endpoint,id=master]         [Info] : sending getMessages(startMsgId=0) (artificially delayed by 1879ms)
clyde_  | [   2.513s] [clyde_,8cb00] [  outcomingrequestsmanager.cpp:179 ][Proto::OutcomingRequestsManager,id=master] [Warn] : invalidatePendingRequest(requestId=0, reason=InvalidationReason::TIMEOUT)
clyde_  | [   2.514s] [clyde_,8cb00] [             mastersession.cpp:88  ][MasterSession]                     [Warn] : Failed to recieve messages from master, retrying
clyde_  | [   2.514s] [clyde_,8cb00] [             mastersession.cpp:68  ][MasterSession]                     [Info] : asking master for messages, starting from messageId=0
clyde_  | [   2.514s] [clyde_,8cb00] [                  endpoint.cpp:126 ][Proto::Endpoint,id=master]         [Info] : sending getMessages(startMsgId=0) (artificially delayed by 1065ms)
master  | [   2.826s] [master,c8640] [                  endpoint.cpp:239 ][Proto::Endpoint,id=sec_0]          [Info] : incoming_getMessages(startMsgId=0)
clyde_  | [   2.894s] [clyde_,8cb00] [  outcomingrequestsmanager.cpp:147 ][Proto::OutcomingRequestsManager,id=master] [Warn] : Recieved response for request 0, but it is not marked as pending
bonnie  | [   3.001s] [bonnie,ccb00] [             secondarynode.cpp:51  ][SecondaryNode]                     [Info] : reconnecting to master node
master  | [   2.971s] [master,c9640] [            socketacceptor.cpp:107 ][NetUtils::SocketAcceptor]          [Info] : Accepting incoming connection
master  | [   2.972s] [master,ca640] [                masternode.cpp:67  ][MasterNode]                        [Info] : adding secondary: id=1
bonnie  | [   3.001s] [bonnie,ccb00] [             secondarynode.cpp:78  ][SecondaryNode]                     [Info] : connected to master node, establising endpoint
bonnie  | [   3.003s] [bonnie,ccb00] [             mastersession.cpp:68  ][MasterSession]                     [Info] : asking master for messages, starting from messageId=0
bonnie  | [   3.003s] [bonnie,ccb00] [                  endpoint.cpp:126 ][Proto::Endpoint,id=master]         [Info] : sending getMessages(startMsgId=0) (artificially delayed by 1573ms)
master  | [   3.513s] [master,c8640] [                  endpoint.cpp:239 ][Proto::Endpoint,id=sec_0]          [Info] : incoming_getMessages(startMsgId=0)
clyde_  | [   3.580s] [clyde_,8cb00] [             mastersession.cpp:81  ][MasterSession]                     [Info] : Successfully retrieved messages from master, filling storage
clyde_  | [   3.580s] [clyde_,8cb00] [             secondarynode.cpp:116 ][SecondaryNode]                     [Info] : master session operational
clyde_  | [   3.580s] [clyde_,8cb00] [             mastersession.cpp:96  ][MasterSession]                     [Info] : sending friendly name to master
clyde_  | [   3.580s] [clyde_,8cb00] [                  endpoint.cpp:160 ][Proto::Endpoint,id=master]         [Info] : sending secondaryNodeReady(secondaryName='clyde_') (artificially delayed by 1191ms)
bonnie  | [   4.503s] [bonnie,ccb00] [  outcomingrequestsmanager.cpp:179 ][Proto::OutcomingRequestsManager,id=master] [Warn] : invalidatePendingRequest(requestId=0, reason=InvalidationReason::TIMEOUT)
bonnie  | [   4.503s] [bonnie,ccb00] [             mastersession.cpp:88  ][MasterSession]                     [Warn] : Failed to recieve messages from master, retrying
bonnie  | [   4.503s] [bonnie,ccb00] [             mastersession.cpp:68  ][MasterSession]                     [Info] : asking master for messages, starting from messageId=0
bonnie  | [   4.503s] [bonnie,ccb00] [                  endpoint.cpp:126 ][Proto::Endpoint,id=master]         [Info] : sending getMessages(startMsgId=0) (artificially delayed by 1624ms)
master  | [   4.546s] [master,c7640] [                  endpoint.cpp:239 ][Proto::Endpoint,id=sec_1]          [Info] : incoming_getMessages(startMsgId=0)
bonnie  | [   4.577s] [bonnie,ccb00] [  outcomingrequestsmanager.cpp:147 ][Proto::OutcomingRequestsManager,id=master] [Warn] : Recieved response for request 0, but it is not marked as pending
master  | [   4.705s] [master,c8640] [                  endpoint.cpp:275 ][Proto::Endpoint,id=sec_0]          [Info] : incoming_secondaryNodeReady(secondaryNodeName='clyde_')
master  | [   4.705s] [master,ca640] [                masternode.cpp:146 ][MasterNode]                        [Info] : registering node 0 as ready, friendlyName: clyde_
clyde_  | [   4.773s] [clyde_,8cb00] [             mastersession.cpp:109 ][MasterSession]                     [Info] : recieved ack from master on secondaryNodeReady request
bonnie  | [   6.004s] [bonnie,ccb00] [  outcomingrequestsmanager.cpp:179 ][Proto::OutcomingRequestsManager,id=master] [Warn] : invalidatePendingRequest(requestId=1, reason=InvalidationReason::TIMEOUT)
bonnie  | [   6.004s] [bonnie,ccb00] [             mastersession.cpp:88  ][MasterSession]                     [Warn] : Failed to recieve messages from master, retrying
bonnie  | [   6.004s] [bonnie,ccb00] [             mastersession.cpp:68  ][MasterSession]                     [Info] : asking master for messages, starting from messageId=0
bonnie  | [   6.004s] [bonnie,ccb00] [                  endpoint.cpp:126 ][Proto::Endpoint,id=master]         [Info] : sending getMessages(startMsgId=0) (artificially delayed by 1440ms)
master  | [   6.098s] [master,c7640] [                  endpoint.cpp:239 ][Proto::Endpoint,id=sec_1]          [Info] : incoming_getMessages(startMsgId=0)
bonnie  | [   6.128s] [bonnie,ccb00] [  outcomingrequestsmanager.cpp:147 ][Proto::OutcomingRequestsManager,id=master] [Warn] : Recieved response for request 1, but it is not marked as pending
master  | [   7.415s] [master,c7640] [                  endpoint.cpp:239 ][Proto::Endpoint,id=sec_1]          [Info] : incoming_getMessages(startMsgId=0)
bonnie  | [   7.445s] [bonnie,ccb00] [             mastersession.cpp:81  ][MasterSession]                     [Info] : Successfully retrieved messages from master, filling storage
bonnie  | [   7.446s] [bonnie,ccb00] [             secondarynode.cpp:116 ][SecondaryNode]                     [Info] : master session operational
bonnie  | [   7.446s] [bonnie,ccb00] [             mastersession.cpp:96  ][MasterSession]                     [Info] : sending friendly name to master
bonnie  | [   7.446s] [bonnie,ccb00] [                  endpoint.cpp:160 ][Proto::Endpoint,id=master]         [Info] : sending secondaryNodeReady(secondaryName='bonnie') (artificially delayed by 1753ms)
bonnie  | [   8.946s] [bonnie,ccb00] [  outcomingrequestsmanager.cpp:179 ][Proto::OutcomingRequestsManager,id=master] [Warn] : invalidatePendingRequest(requestId=3, reason=InvalidationReason::TIMEOUT)
bonnie  | [   8.946s] [bonnie,ccb00] [             mastersession.cpp:113 ][MasterSession]                     [Warn] : failed to recieve ack from master on secondaryNodeReady request, retrying
bonnie  | [   8.946s] [bonnie,ccb00] [             mastersession.cpp:96  ][MasterSession]                     [Info] : sending friendly name to master
bonnie  | [   8.946s] [bonnie,ccb00] [                  endpoint.cpp:160 ][Proto::Endpoint,id=master]         [Info] : sending secondaryNodeReady(secondaryName='bonnie') (artificially delayed by 1658ms)
master  | [   9.169s] [master,c7640] [                  endpoint.cpp:275 ][Proto::Endpoint,id=sec_1]          [Info] : incoming_secondaryNodeReady(secondaryNodeName='bonnie')
master  | [   9.169s] [master,ca640] [                masternode.cpp:146 ][MasterNode]                        [Info] : registering node 1 as ready, friendlyName: bonnie
bonnie  | [   9.199s] [bonnie,ccb00] [  outcomingrequestsmanager.cpp:147 ][Proto::OutcomingRequestsManager,id=master] [Warn] : Recieved response for request 3, but it is not marked as pending
bonnie  | [  10.446s] [bonnie,ccb00] [  outcomingrequestsmanager.cpp:179 ][Proto::OutcomingRequestsManager,id=master] [Warn] : invalidatePendingRequest(requestId=4, reason=InvalidationReason::TIMEOUT)
bonnie  | [  10.447s] [bonnie,ccb00] [             mastersession.cpp:113 ][MasterSession]                     [Warn] : failed to recieve ack from master on secondaryNodeReady request, retrying
bonnie  | [  10.447s] [bonnie,ccb00] [             mastersession.cpp:96  ][MasterSession]                     [Info] : sending friendly name to master
bonnie  | [  10.447s] [bonnie,ccb00] [                  endpoint.cpp:160 ][Proto::Endpoint,id=master]         [Info] : sending secondaryNodeReady(secondaryName='bonnie') (artificially delayed by 1462ms)
master  | [  10.575s] [master,c7640] [                  endpoint.cpp:275 ][Proto::Endpoint,id=sec_1]          [Info] : incoming_secondaryNodeReady(secondaryNodeName='bonnie')
master  | [  10.575s] [master,ca640] [                masternode.cpp:146 ][MasterNode]                        [Info] : registering node 1 as ready, friendlyName: bonnie
bonnie  | [  10.605s] [bonnie,ccb00] [  outcomingrequestsmanager.cpp:147 ][Proto::OutcomingRequestsManager,id=master] [Warn] : Recieved response for request 4, but it is not marked as pending
master  | [  11.880s] [master,c7640] [                  endpoint.cpp:275 ][Proto::Endpoint,id=sec_1]          [Info] : incoming_secondaryNodeReady(secondaryNodeName='bonnie')
master  | [  11.880s] [master,ca640] [                masternode.cpp:146 ][MasterNode]                        [Info] : registering node 1 as ready, friendlyName: bonnie
bonnie  | [  11.910s] [bonnie,ccb00] [             mastersession.cpp:109 ][MasterSession]                     [Info] : recieved ack from master on secondaryNodeReady request
master  | [  31.943s] [master,d5b00] [            socketacceptor.cpp:77  ][NetUtils::SocketAcceptor]          [Info] : Accepting incoming connection
master  | [  31.945s] [master,cd640] [                  endpoint.cpp:109 ][Proto::Endpoint,id=sec_1]          [Info] : sending addMessage(msgId=0, msg='sample message') (artificially delayed by 1635ms)
master  | [  31.945s] [master,cd640] [                  endpoint.cpp:109 ][Proto::Endpoint,id=sec_0]          [Info] : sending addMessage(msgId=0, msg='sample message') (artificially delayed by 1551ms)
master  | [  33.445s] [master,c7640] [  outcomingrequestsmanager.cpp:179 ][Proto::OutcomingRequestsManager,id=sec_1] [Warn] : invalidatePendingRequest(requestId=0, reason=InvalidationReason::TIMEOUT)
master  | [  33.445s] [master,c8640] [  outcomingrequestsmanager.cpp:179 ][Proto::OutcomingRequestsManager,id=sec_0] [Warn] : invalidatePendingRequest(requestId=0, reason=InvalidationReason::TIMEOUT)
master  | [  33.445s] [master,cd640] [         addmessagerequest.cpp:193 ][AddMessageRequest,id=0]            [Warn] : Recieved error from secondary node 1
master  | [  33.445s] [master,cd640] [         addmessagerequest.cpp:193 ][AddMessageRequest,id=0]            [Warn] : Recieved error from secondary node 0
master  | [  33.445s] [master,cd640] [                  endpoint.cpp:109 ][Proto::Endpoint,id=sec_1]          [Info] : sending addMessage(msgId=0, msg='sample message') (artificially delayed by 1175ms)
master  | [  33.445s] [master,cd640] [                  endpoint.cpp:109 ][Proto::Endpoint,id=sec_0]          [Info] : sending addMessage(msgId=0, msg='sample message') (artificially delayed by 1004ms)
clyde_  | [  33.563s] [clyde_,8cb00] [                  endpoint.cpp:221 ][Proto::Endpoint,id=master]         [Info] : incoming_addMessage(msgId=0, msg='sample message')
master  | [  33.496s] [master,c8640] [  outcomingrequestsmanager.cpp:147 ][Proto::OutcomingRequestsManager,id=sec_0] [Warn] : Recieved response for request 0, but it is not marked as pending
bonnie  | [  33.610s] [bonnie,ccb00] [                  endpoint.cpp:221 ][Proto::Endpoint,id=master]         [Info] : incoming_addMessage(msgId=0, msg='sample message')
master  | [  33.580s] [master,c7640] [  outcomingrequestsmanager.cpp:147 ][Proto::OutcomingRequestsManager,id=sec_1] [Warn] : Recieved response for request 0, but it is not marked as pending
clyde_  | [  34.516s] [clyde_,8cb00] [                  endpoint.cpp:221 ][Proto::Endpoint,id=master]         [Info] : incoming_addMessage(msgId=0, msg='sample message')
clyde_  | [  34.516s] [clyde_,8cb00] [                   storage.cpp:10  ][Storage]                           [Warn] : failed to insert message with id 0, already present
master  | [  34.450s] [master,cd640] [         addmessagerequest.cpp:182 ][AddMessageRequest,id=0]            [Info] : Recieved ok response from secondary node 0
bonnie  | [  34.650s] [bonnie,ccb00] [                  endpoint.cpp:221 ][Proto::Endpoint,id=master]         [Info] : incoming_addMessage(msgId=0, msg='sample message')
bonnie  | [  34.650s] [bonnie,ccb00] [                   storage.cpp:10  ][Storage]                           [Warn] : failed to insert message with id 0, already present
master  | [  34.621s] [master,cd640] [         addmessagerequest.cpp:182 ][AddMessageRequest,id=0]            [Info] : Recieved ok response from secondary node 1
master  | [  34.621s] [master,cd640] [         addmessagerequest.cpp:148 ][AddMessageRequest,id=0]            [Info] : marking request as success
master  | [  34.621s] [master,cd640] [         addmessagerequest.cpp:63  ][AddMessageRequest,id=0]            [Info] : all secondaries answered, destroying request
```
