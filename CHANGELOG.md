Initial version 0.0.0 - 2023-03-20 [f819ce8] 
- Added Licence
- Added Readme

Version 1.0.0 - 2023-03-20 [7ecba7f]  
- Argument parsing done  
- Created sockets for TCP/UDP  
- Made connection for TCP  
- Data exchange for TCP  
- Data exchange for UDP without encoding/decoding  
- Reaction for CTRL-C for TCP 

Version 1.1.0 - 2023-03-20 [70de58d]  
- Multi-platforming  
- Fixed interruption of comunication with TSP server after pressed keyboard shortcut  
- Encoding and Decoding messages for UDP 
- Fixed interuption  of comunication with TSP server after key-word "BYE"
- Reaction for CTRL-C for TCP 
- Added data output for UDP

Version 1.2.0 - 2023-03-21 [e20b0c0]
- Written comments and header
- Added timeout for UDP
- Fixed function `message_to_binary`
- Added check for correct working 'fgets'
- Fixed data output for UDP
- Added Makefile

Version 1.3.0 - 2023-03-21 [4cd913f]
- Added check for returning operation code for UDP
- Fixed Makefile