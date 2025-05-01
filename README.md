
# AuXion_Project_DAA
AuXion is a distributed auction platform where decentralized nodes bid, synchronize, and reach consensus in real time.

# Group Members:
Huzaifa Saleem   ----------------------------
Abdullah Asim  


# AuXion – Distributed Auction System

AuXion is a distributed computing project that simulates a decentralized auction system where multiple bidder nodes compete for items. The system uses a **consensus protocol** to determine the winning bid in the presence of failures, message delays, or network issues. It demonstrates key principles of **fault-tolerant distributed systems**, **leader election**, and **distributed coordination** using **C++**.

---

# Execution Flow

1) Peer Discovery Phase :
      * Node broadcast their presence using UDP on Port 9000 for 10 seconds.
      * Each Node broadcsat braodcast as well as listen in a separate thread and build a network map with node IDS,IPs and Ports.

2) TCP Connection Phase:
      * Each node starts a TCP server on a port derived from its UDP port (e.g UDP 8001 -> TCP 9001)
      * Node connect to each other as TCP clients forming peer_sockets
        
3) Role Selection:
     
     * The user selects a role:  Coordinator / Bidder
       
4) Auction Process:
     * The auction starts as in phase 2 and the coordinator broadcast the winner at the end.



# AuXion: Distributed Auction System

**Phase 3: 90% Implementation**

⬜ Converted from Centrilzed to Gull Time Peer to Peer Network.

# Future Enhancement:
        
         ^ Multiple Round Bidding
         ^ Leader Election in case of Coordinator Failure(Consensus Algorithm)
         ^ UI/UX 

# Note  :

   The code is valid on a devices having same LAN.


## Run
```bash
g++ main.cpp -o auxion
./auxion

