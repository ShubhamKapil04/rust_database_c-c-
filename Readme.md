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

7. ## Key-value Server

1. A KV store with just get, set, del commands

len msg1 len msg2 
_ 4B ... _ 4B ...

A redis request is a list of string, just like a Linux Command. Representing a list as a chuks of bytes is a task of deserialization.

nstr len str1 len str1 ... len strn
4b   4B ..... 4B....

nstr is the number of items in the list, followed by each item.

7. 2. ## Handling requests

what to do?

3 steps to handle a request 
1. Parse the command
2. Process the command and generate a response
3. Append the response to the output buffer.


