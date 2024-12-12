## Network File System

### Introduction

The Network File System (NFS) project entails the creation of a distributed file system, emphasizing seamless communication between Clients, Naming Servers (NM), and Storage Servers (SS). Through phased initialization, the project establishes a foundation for efficient file operations, such as reading, writing, and deleting, orchestrated by the Naming Server. Advanced features include concurrent client access, error handling, and redundancy/replication strategies, providing a hands-on experience in building resilient networked file systems.

### Control Flow

The initialization process commences with the establishment of the naming server, fixing designated ports at 1234 and 2345. Subsequently, the storage server undergoes initialization in accordance with the provided paths. The naming server then establishes a connection with the storage server, thereby completing the initialization phase for both entities.

Following this, the client interfaces with the naming server. The user specifies the desired operation using a predetermined format and transmits the corresponding request to the naming server. The naming server evaluates the received request to ascertain whether it pertains to read, write, get, create, delete, or copy operations.

In the event of a read operation, the naming server conducts a search for the specified file within the designated directory across all naming servers. If the file is located, the naming server transmits the port information of the pertinent storage server to the client. Conversely, if the file is not found, the naming server issues an acknowledgment, which may assume values such as 'NOT_FOUND,' 'NOT_ACCESSIBLE,' or 'INVALID_COMMAND.' In case of an error code, the client raises an error and proceeds to the subsequent request. Conversely, upon receiving a port from the naming server, the client establishes a connection with the designated storage server. Subsequently, the storage server executes the specified task and communicates an acknowledgment to the client indicating the success or failure of the operation. The client prints the received acknowledgment, terminates the connection with the storage server, forwards the acknowledgment to the naming server, and progresses to the next request.

For create, delete, or copy commands, the naming server identifies and extracts the storage server specified by the client. The request is then forwarded to the identified storage server, which executes the task and transmits the resulting acknowledgment to the naming server. The naming server, in turn, relays the same acknowledgment to the client.

In the case of an 'exit' command, the client disconnects from the naming server, concluding the interaction.

### Error Handling

The project incorporates a robust error handling mechanism, which is implemented at the client, naming server, and storage server levels. The client, naming server, storage server raise an error as given in that of `defs.h`.

### How to Run

The project is executed using the following commands:
`make nm ss clt`

Then create instances of naming server, storage server, and client using the following commands:
`./nm`
`./ss`
`./clt`

### Features
- Allows the user to read, write, get metadata about the files, and create, delete directories or files.
- Search has been optimized by the use of tries. 
- There is an acknowledgement system in place to ensure that the user is aware of the status of the operation.

### Assumptions

- The user inputs the commands in the specified format. For read/write/get, it is <operation> <name> <relative_path>. For create/delete/copy, it is <operation> file/dir <name> <relative_path>
- Redundancy is not implemented. Alive thread, that is complimentary to redundancy, is also not implemented.
- The commands supported are reading, writing, getting metadata about the files, and creating, deleting directories or files, and exit. 
- Creating a file or directory automatically gives access to the user who created it.
- Deleting a file or directory automatically removes access to the user who created it.
- Deleting a directory involves all the contents inside it. 

### Conclusion

The project provides a hands-on experience in building a distributed file system, which is a fundamental component of modern-day computing. The project emphasizes the importance of seamless communication between clients, naming servers, and storage servers, which is achieved through the use of sockets and acknowledgements. The project also highlights the significance of error handlings, which are essential for the development of robust distributed systems. The project also provides a platform for the implementation of advanced features, such as concurrent client access, which is a key component of modern-day file systems.