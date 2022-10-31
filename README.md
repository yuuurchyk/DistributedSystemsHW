# DistributedSystemsHW

## Progress So Far

* **boost::log**: setup thread-safe logger
* **boost::asio**: simple tcp echo server
* custom tcp protocol definition and tests
* **intel::tbb** concurrent vector for data storage
* basic protocol communication via tcp socket (compose file does not work yet)
* secondary node logic
* master node logic

## Approach

* master and secondary nodes communicate via single tcp channel using custom protocol (there is always one connection between master and secondary node and a dedicated thread on master and secondary node that sends responses/reacts to requests)
* both master and secondary nodes have a pool of worker threads, which:
    * read and process incoming http requests
    * parse requests, forming response frame (custom protocol), schedule them for sending via tcp channel
* master node is considered active, since it connects to all secondary nodes
* in case the connection between master and secondary node is dropped, master node schedules reconnect every 3 seconds

**P.S.** I haven't finished the docker compose file, because the task alone was quite difficult for me, please have mercy :)

For now, all 3 nodes communicate via localhost (all 3 nodes use different ports). In order to test, you should build the dockerfile:

```bash
>>> docker build --tag sample --build-arg ENABLE_CMAKE=true --build-arg ENABLE_BOOST=true --build-arg ENABLE_TBB=true --build-arg ENABLE_NINJA=true --target deploy .
```

**Note:** dockerfile will take a long time to build, because all the 3rd-party libraries are built from sources

Then you should run 3 executables within the container:
```bash
>>> master
>>> secondary --id 1
>>> secondary --id 2
```

The ports may be found in [constants.h](./libs/constants/include/constants/constants.h) file.

The definition of the frame of the custom protocol may be fonud in [frame.h](./libs/protocol/include/protocol/frame.h)

## HTTP API

* ```GET /messages``` (for both master and secodary nodes). Returns json list of messages stored on the node
* ```POST /addmessage``` (for master node only). Body should be plain text. Returns 200 only after confirmation from all secondary nodes that the message was replicated

## Point for Future Improvements

* use websockets instead of custom tcp protocol
* implement log level (i.e. have a concurrent priority queue associated with websockets channel)
