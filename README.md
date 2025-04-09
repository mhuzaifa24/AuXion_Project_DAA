# AuXion_Project_DAA
AuXion is a distributed auction platform where decentralized nodes bid, synchronize, and reach consensus in real time.

#AuXion – Distributed Auction System

AuXion is a distributed computing project that simulates a decentralized auction system where multiple bidder nodes compete for items. The system uses a **consensus protocol** to determine the winning bid in the presence of failures, message delays, or network issues. It demonstrates key principles of **fault-tolerant distributed systems**, **leader election**, and **distributed coordination** using **C++**.

---

## Key Features

- **Multiple Distributed Nodes**: Simulated using multithreading (or processes) to represent bidders and coordinators.
- **Distributed Bidding Protocol**: Bidders submit bids without relying on a centralized auctioneer.
- **Leader Election**: Uses a consensus algorithm (e.g., **Bully Algorithm**) to elect a coordinator in case of failure.
- **Fault Tolerance**: Handles node failures and re-elects coordinators when needed.
- **Live Auction Monitoring**: Tracks real-time bidding and decision-making between nodes.

---
