5. Concurrent IO Models

## Thread Based Concurrency

We need to handle multiple request as client can hold connection as long as it wants. So there is need to handle multiple connections simultaneously, becouse while her serve is waiting on one client, it cannot do anything with the other clients.

# Reading and Writing

As we know read and writing block the threads I/O which leads to concurrency

# Threads aren' enough.

The mordern server app use eventt loops to handle concurrentt IO without creating new threads. 

The drawbacks of threads based IO.

1. Memeory usage -> Many threads mean many stacks. Stacks are used for local variables and functiona calls, memory usage per threads is hard to control.
2. Overhead -> Staekess clinets like PHP app will createt many short-lived connections, adding overhead to both latency and CPU usage.

6. ## Event Loop

1. Learning the Event loop working 
2. Usage of any daa structure with choice sometime a hashmap is more complex and space or time consuming to use insted of that try array.