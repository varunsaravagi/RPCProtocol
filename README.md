# RPCProtocol

This project  builds an RPC system to allow remote file operations (open, read, write, ..).  The RPC abstraction is made to look as close to local file operations as possible. When a command such as "cat foo" is executed, instead of opening and printing the contents of a local file, the contents of file foo on the remote server machine are accessed and displayed.
A brief description of design is there in the src folder.
