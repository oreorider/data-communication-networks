/*
    NXC Lab Torrent-esque P2P File Sharing System Assignment (2022 Fall) 


    //////////////////////    INTRODUCTION    ///////////////////////////

    In this assignment, you will implement a torrent-esque P2P file-sharing system.
    This system implements a custom network protocol for file sharing.

    Our torrent system partitions a file into blocks and distributes them to peers.
    The torrents are referenced using a hash value of the file in our torrent network.
    Management of the torrent information and data is not the focus of this assignment, and thus 
    it is implemented for you. You can find the details in torrent_functions.h.


    The network protocol is implemented as follows:

    - A peer first requests a torrent info from a seeder using 
    the hash value of the torrent, using REQUEST_TORRENT command.

    - The seeder responds with the torrent info, using PUSH_TORRENT command.
    The requester receives the torrent info, creates a new torrent_file struct and adds it to its global torrent list. (Refer to torrent_functions.h)
    The seeder is also added to the peer list of the new torrent.

    - The requester requests block info of the seeder using REQUEST_BLOCK_INFO command.
    The seeder responds with the block info of the torrent, using PUSH_BLOCK_INFO command.

    - With the block info of the seeder, the requester can now check which block the seeder has.
    The requester then requests its missing blocks from the seeder using REQUEST_BLOCK command.
    The seeder responds with the requested block, using PUSH_BLOCK command.

    - At random intervals, the requester can request a peer list from a random peer(seeder) using REQUEST_PEERS command.
    The random peer responds with its peer list of the torrent, using PUSH_PEERS command.
    This way, the requester can discover new peers of the torrent.

    - The requester can also request block info of the newly discovered peer using REQUEST_BLOCK_INFO command,
    and request missing blocks from those new peers using REQUEST_BLOCK command.

    - Of course, being a P2P system, any requester can also act as a seeder, and any seeder can also act as a requester.

    The requester routines are implemented in client_routines() function, while the seeder routines are implemented in server_routines() function.
    Please refer to the comments in network_functions.h and the code structure of the skeleton function for more details.


    //////////////////////    PROGRAM DETAILS    ///////////////////////////

    To compile: 
    (X86 LINUX)           gcc -o main main.c torrent_functions.o network_functions.o
    (Apple Silicon MAC)   gcc -o main main.c torrent_functions_MAC.o network_functions_MAC.o (Use brew to install gcc on MAC)

    First, test the program by running the following commands in two different terminals:
        ./main 127.0.0.1 1
        ./main 127.0.0.1 2
    They should connect to each other and exchange torrent information and data.

    Run the same commands AFTER compiling the program with silent mode disabled 
    (silent_mode = 0; in main function)
    You should see detailed information about the program's operation.

    These are implemented using TA's model implementations.

    For this assignment, you must implement the following functions and 
    achieve the same functionalities as the TA's implementation...

    - request_torrent_from_peer         (Already implemented for you!)
    - push_torrent_to_peer              (Already implemented for you!)
    - request_peers_from_peer           (Points: 5)
    - push_peers_to_peer                (Points: 5)
    - request_block_info_from_peer      (Points: 5)
    - push_block_info_to_peer           (Points: 5)
    - request_block_from_peer           (Points: 5)
    - push_block_to_peer                (Points: 5)
    - server_routine                    (Points: 40)
    - client_routine                    (Points: 30)

    Total of 100 points.
    Points will be deducted if the functions are not implemented correctly. 
    (Crash/freezing, wrong behavior, etc.)

    Helper functions are provided in network_functions.h and torrent_functions.h, and you are free to use them for your implementation.
    You can of course implement additional functions if you need to.

    TA's model implementations of the above functions are declared in network_functions.h with _ans suffix.
    Use them to test your implementation, but make sure to REMOVE ALL _ans functions from your implementation before submitting your code.

    Reference network_functions.h file to get more information about the functions and the protocol.
    Functions for torrent manipulation are declared & explained in torrent_functions.h.

    Your hand-in file will be tested for its functionalities using the same main function but with your implementation of the above functions.
    (server_routine_ans() and client_routine_ans() in main() will be replaced with your implementation of server_routine() and client_routine() respectively.)

    Your program will be tested with more than 2 peers in the system. You can use ports 12781 to 12790 for testing. 
    Change the port number in the main() function's listen_socket() and request_from_hash() functions.
    listen_socket()'s input port defines the running program's port, 
    while request_from_hash()'s port defines the port of the peer(seeder) which the program requests the torrent from.

    //////////////////////    CLOSING REMARKS    ///////////////////////////

    The program can be tested with multiple machines, and with arbitrary files up to 128MiB. (And can even be used to share files with your friends!) 
    Just input ./main <IP address of the seeder machine> <Peer mode> and replace the corresponding file name/path and hash values in the main function.
    However, being a P2P system, additional port management for firewall and NAT port forwarding to open your listen port to outside connections may be required.
    The details for NAT port forwarding will be covered in the lecture. Google "P2P NAT hole punching" if you are curious about how P2P systems work with NAT.

    PLEASE READ network_functions.h AND torrent_functions.h CAREFULLY BEFORE IMPLEMENTING YOUR CODE.
    ALSO, PLEASE CAREFULLY READ ALL COMMENTS, AS THEY CONTAIN IMPORTANT INFORMATION AND HINTS.

    JS Park.
*/


#include <stdio.h>
#include <time.h> 

#include "torrent_functions.h"
#include "network_functions.h"

#define SLEEP_TIME_MSEC 5000

/////////////////////////// START OF YOUR IMPLEMENTATIONS ///////////////////////////

// Message protocol: "REQUEST_TORRENT [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
int request_torrent_from_peer(char *peer, int port, unsigned int torrent_hash)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND REQUEST_TORRENT to peer %s:%d, Torrent %x\n"
        , peer, port, torrent_hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    sprintf(buf, "REQUEST_TORRENT %d %x %x", listen_port, id_hash, torrent_hash);
    send_socket(sockfd, buf, STRING_LEN);
    close_socket(sockfd);
    //doesn't read incoming data?
    return 0;
}

// Message protocol: "PUSH_TORRENT [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"[TORRENT_INFO]
int push_torrent_to_peer(char *peer, int port, torrent_file *torrent)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND PUSH_TORRENT to peer %s:%d, Torrent %x\n"
        , peer, port, torrent->hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    torrent_info info;
    copy_torrent_to_info (torrent, &info);
    sprintf(buf, "PUSH_TORRENT %d %x %x", listen_port, id_hash, info.hash); //create protocol message
    send_socket(sockfd, buf, STRING_LEN);                                   //send protocol message
    send_socket(sockfd, (char *)&info, sizeof(torrent_info));               //send actual torrent data
    close_socket(sockfd);
    return 0;
}

// Message protocol: "REQUEST_PEERS [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
int request_peers_from_peer(char *peer, int port, unsigned int torrent_hash)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND REQUEST_PEERS to peer %s:%d, Torrent %x \n"
            , peer, port, torrent_hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    sprintf(buf, "REQUEST_PEERS %d %x %x", listen_port, id_hash, torrent_hash);
    send_socket(sockfd, buf, STRING_LEN);
    close_socket(sockfd);

    // TODO: Implement (5 Points)
    return 0;
}

// Message protocol: "PUSH_PEERS [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [TORRENT_NUM_PEERS]"[PEER_IPS][PEER_PORTS]
int push_peers_to_peer(char *peer, int port, torrent_file *torrent)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND PUSH_PEERS list to peer %s:%d, Torrent %x \n"
        , peer, port, torrent->hash);
    // TODO: Implement (5 Points)
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    //printf("0\n");
    unsigned int torrent_hash = torrent->hash;
    //printf("0.5\n");
    int torrent_num_peers = torrent->num_peers;
    //printf("0.8\n");
    int peer_idx = get_peer_idx(torrent, peer, port);
    //printf("1\n");
    char temp_peer_ip [MAX_PEER_NUM][STRING_LEN];//2d array of char
    char temp_peer_port [sizeof(int) * MAX_PEER_NUM];//array of int

    for(int i=0; i<MAX_PEER_NUM; i++){
        if( i != peer_idx){//make temp arrays hold data excluding peer/port
            //printf("2\n");
            strcpy(temp_peer_ip[i], torrent->peer_ip[i]);
            //printf("3\n");
            temp_peer_port[i] = torrent->peer_port[i];
        }
    }
    
    char buf[STRING_LEN] = {0};
    sprintf(buf, "PUSH_PEERS %d %x %x %d", listen_port, id_hash, torrent_hash, torrent_num_peers);
    send_socket(sockfd, buf, STRING_LEN);
    //printf("4\n");
    send_socket(sockfd, (char*)temp_peer_ip, MAX_PEER_NUM * STRING_LEN);
    //printf("5\n");
    send_socket(sockfd, temp_peer_port, sizeof(int) * MAX_PEER_NUM);
    //printf("6\n");
    close_socket(sockfd);
    return 0;
}

// Message protocol: "REQUEST_BLOCK_INFO [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
int request_block_info_from_peer(char *peer, int port, unsigned int torrent_hash)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND REQUEST_BLOCK_INFO to peer %s:%d, Torrent %x\n"
        , peer, port, torrent_hash);
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    sprintf(buf, "REQUEST_BLOCK_INFO %d %x %x", listen_port, id_hash, torrent_hash);
    send_socket(sockfd, buf, STRING_LEN);
    close_socket(sockfd);
    // TODO: Implement (5 Points)
    return 0;
}

// Message protocol: "PUSH_BLOCK_INFO [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"[MY_BLOCK_INFO]
int push_block_info_to_peer(char *peer, int port, torrent_file *torrent)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND PUSH_BLOCK_INFO to peer %s:%d, Torrent %x\n"
        , peer, port, torrent->hash);

    // TODO: Implement (5 Points)
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    //torrent_info info;
    //copy_torrent_to_info(torrent, &info);
    sprintf(buf, "PUSH_BLOCK_INFO %d %x %x",listen_port, id_hash, torrent->hash);
    send_socket(sockfd, buf, STRING_LEN);
    send_socket(sockfd, torrent->block_info, MAX_BLOCK_NUM);
    printf("==============================\n");
    print_torrent_info(torrent);
    /*
    for(int i=0; i<torrent->block_num; i++){
        printf("|%c|", torrent->block_info[i]);
    }*/
    printf("\n===============================\n");
    close_socket(sockfd);
    return 0;
}

// Message protocol: "REQUEST_BLOCK [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [BLOCK_INDEX]"
int request_block_from_peer(char *peer, int port, torrent_file *torrent, int block_idx)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND REQUEST_BLOCK %d to peer %s:%d, Torrent %x\n"
       , block_idx, peer, port , torrent->hash);
       /*
       for(int i=0; i<MAX_BLOCK_NUM; i++){
            printf("|%c|", torrent->block_info[i]);
        }*/
    // TODO: Implement (5 Points)
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    torrent_info info;
    copy_torrent_to_info(torrent, &info);
    sprintf(buf, "REQUEST_BLOCK %d %x %x %d", listen_port, id_hash, info.hash, block_idx);
    send_socket(sockfd, buf, STRING_LEN);
    close_socket(sockfd);
    return 0;
}

// Message protocol: "PUSH_BLOCK [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [BLOCK_INDEX]"[BLOCK_DATA]
int push_block_to_peer(char *peer, int port, torrent_file *torrent, int block_idx)
{
    if (silent_mode == 0)
        printf ("INFO - COMMAND PUSH_BLOCK %d to peer %s:%d, Torrent %x\n"
       , block_idx, peer, port , torrent->hash);
    // TODO: Implement (5 Points)
    // Hint: You can directly use the pointer to the block data for the send buffer of send_socket() call.
    int sockfd = connect_socket(peer, port);
    if (sockfd < 0) 
    {
        return -1;
    }
    char buf[STRING_LEN] = {0};
    torrent_info info;
    copy_torrent_to_info(torrent, &info);
    sprintf(buf, "PUSH_BLOCK %d %x %x %d", listen_port, id_hash, info.hash, block_idx);
    send_socket(sockfd, buf, STRING_LEN);
    send_socket(sockfd, torrent->block_ptrs[block_idx], torrent->block_size);
    return 0;
}

int server_routine (int sockfd, char* my_ip, int my_portnumber)
{
    //printf("enter server routine\n");
    struct sockaddr_in client_addr;
    socklen_t slen = sizeof(client_addr);
    int newsockfd;
    while ((newsockfd = accept_socket(sockfd, &client_addr, &slen, max_listen_time_msec)) >= 0)
    {
        printf("socket accept\n");
        char buf[STRING_LEN] = {0};                                             // Buffer for receiving commands
        recv_socket(newsockfd, buf, STRING_LEN);                                // Receive data from socket
        char peer_ip[INET_ADDRSTRLEN];                                             // Buffer for saving peer IP address
        inet_ntop( AF_INET, &client_addr.sin_addr, peer_ip, INET_ADDRSTRLEN );     // Convert IP address to string
        if (silent_mode == 0)
            printf ("INFO - SERVER: Received command %s ", buf);

        // TODO: Parse command (HINT: Use strtok, strcmp, strtol, stroull, etc.) (5 Points)
        char *cmd;
        char *pEnd;
        cmd = strtok(buf, " ");
        int peer_port = strtol(strtok(NULL, " "), NULL, 10);
        unsigned int peer_id_hash = strtoul(strtok(NULL, " "), NULL, 16);
        if (silent_mode == 0)
            printf ("from peer %s:%d\n", peer_ip, peer_port);

        // TODO: Check if command is sent from myself, and if it is, ignore the message. (HINT: use id_hash) (5 Points)
        if(id_hash == peer_id_hash) return 0;

        // Take action based on command.
        // Dont forget to close the socket, and reset the peer_req_num of the peer that have sent the command to zero.
        // If the torrent file for the given hash value is not found in the torrent list, simply ignore the message. (Except for PUSH_TORRENT command)
        // If the torrent file exists, but the message is from an unknown peer, add the peer to the peer list of the torrent.
        if (strcmp(cmd, "REQUEST_TORRENT") == 0) 
        {
            // A peer requests a torrent info!
            // Peer's Message: "REQUEST_TORRENT [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
            // HINT: You might want to use get_torrent(), push_torrent_to_peer(), or add_peer_to_torrent().
            close_socket(newsockfd);
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            torrent_file *torrent = get_torrent(torrent_hash);
            if (torrent != NULL) 
            {
                push_torrent_to_peer(peer_ip, peer_port, torrent);             // Send torrent to peer (HINT: This opens a new socket to client...)
                if (get_peer_idx (torrent, peer_ip, peer_port) < 0) //if peer does not have the torrent it is requesting
                {
                    add_peer_to_torrent(torrent, peer_ip, peer_port, NULL);//update the peers block information
                    //torrent->num_peers++;    
                }
                torrent->peer_req_num [get_peer_idx (torrent, peer_ip, peer_port)] = 0;
            }
            else if(torrent == NULL) return -1;
        }
        else if (strcmp(cmd, "PUSH_TORRENT") == 0) 
        {
            // A peer sends a torrent info!
            // Peer's Message: "PUSH_TORRENT [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"[TORRENT_INFO]
            // Hint: You might want to use get_torrent(), copy_info_to_torrent(), init_torrent_dynamic_data(), add_torrent(), or add_peer_to_torrent().
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            torrent_file *torrent = get_torrent(torrent_hash);//checks global torrent list to see if torrent with torrent_hash exists
            if (torrent == NULL) //if torrent not currently in global_torrent_list
            {
                torrent = (torrent_file *)calloc(1, sizeof(torrent_file));
                torrent_info info;
                recv_socket(newsockfd, (char *)&info, sizeof(torrent_info));//read torrent_info
                copy_info_to_torrent(torrent, &info);//creat torrent struct
                
                init_torrent_dynamic_data (torrent);//initialize torrent with struct
                add_torrent(torrent);//add to list
                add_peer_to_torrent(torrent, peer_ip, peer_port, info.block_info);//add the peer who sent torrent to list
                printf("===============torrent requested, pushed, received, initialized, added to peerlist===========\n");
                //print_torrent_info(torrent);
                //printf("==============================\n");
                torrent->peer_req_num [get_peer_idx (torrent, peer_ip, peer_port)] = 0;//
            }
            close_socket(newsockfd);
        }
        // Refer to network_functions.h for more details on what to send and receive.
        else if (strcmp(cmd, "REQUEST_PEERS") == 0) 
        {
            close_socket(newsockfd);
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            torrent_file* torrent = get_torrent(torrent_hash);
            
            if(torrent != NULL){
                push_peers_to_peer(peer_ip, peer_port, torrent);//give peer list
                if(get_peer_idx(torrent, peer_ip, peer_port) == -1){//if requester not in peer list
                    add_peer_to_torrent(torrent, peer_ip, peer_port, NULL);//add to peer list
                }
            }
            // A peer requests a list of peers!
            // Peer's Message: "REQUEST_PEERS [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
            // Hint: You might want to use  get_torrent(), push_peers_to_peer(), or add_peer_to_torrent().
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "PUSH_PEERS") == 0) 
        {
            // A peer sends a list of peers!
            // Peer's Message: "PUSH_PEERS [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [TORRENT_NUM_PEERS]"[PEER_IPS][PEER_PORTS]
            // Hint: You might want to use get_torrent(), or add_peer_to_torrent().
            //       Dont forget to add the peer who sent the list(), if not already added.
            // TODO: Implement (5 Points)
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            int num_peers = strtol(strtok(NULL, " "), NULL, 10);
            torrent_file *torrent = get_torrent(torrent_hash);
            char temp_PEER_IPS[MAX_PEER_NUM * STRING_LEN];
            char PEER_IPS[MAX_PEER_NUM][STRING_LEN];
            //char temp_peer_ip[STRING_LEN];
            char temp_PEER_PORTS[sizeof(int) * MAX_PEER_NUM];
            //int PEER_PORTS[MAX_PEER_NUM];
            //char temp_peer_port[sizeof(int)];
            
            recv_socket(newsockfd, temp_PEER_IPS, MAX_PEER_NUM * STRING_LEN);//read PEER_IPS
            recv_socket(newsockfd, temp_PEER_PORTS, sizeof(int) * MAX_PEER_NUM);//read PEER_PORTS
            memcpy(PEER_IPS, temp_PEER_IPS, MAX_PEER_NUM * STRING_LEN);//remake PEER_IPS into 2d array
            int PEER_PORTS[MAX_PEER_NUM];//remake PEER_PORTS into array of integers

            //extern uint8_t *bytes;
            //uint32_t myInt1 = bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);

            for(int i=0; i<sizeof(int) * MAX_PEER_NUM; i+=4){
                int insert = temp_PEER_PORTS[i] + (temp_PEER_PORTS[i+1]<<8) + (temp_PEER_PORTS[i+2]<<16) + (temp_PEER_PORTS[i+3]<<24);
                PEER_PORTS[i/4]=insert;
            }
            
            for(int i=0; i<num_peers; i++){
                if((strcmp(PEER_IPS[i], my_ip) != 0) && (PEER_PORTS[i] != my_portnumber)){//dont add peer if same ip/port as myself
                    if(get_peer_idx(torrent, peer_ip, peer_port) < 0){//if peer ip/port not already in list
                        add_peer_to_torrent(torrent, PEER_IPS[i], PEER_PORTS[i], NULL);//add peer to list
                    }
                }
            }
            close_socket(newsockfd);
        }
        else if (strcmp(cmd, "REQUEST_BLOCK_INFO") == 0)//peer asks to see what blocks I have
        {
            close_socket(newsockfd);
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            torrent_file* torrent = get_torrent(torrent_hash);
            if(torrent != NULL){//torrent exists
                printf("============torrent exists=============\n");
                print_torrent_info(torrent);
                printf("=======================================\n");
                if(get_peer_idx(torrent, peer_ip, peer_port) == -1){//if peer not in ip/port list
                    add_peer_to_torrent(torrent, peer_ip, peer_port, NULL);//add peer to list
                    printf("ADD PEER\n");
                    //torrent->num_peers++;
                }
                push_block_info_to_peer(peer_ip, peer_port, torrent);//send peer block info
            }
            // A peer requests your block info!
            // Peer's Message: "REQUEST_BLOCK_INFO [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"
            // Hint: You might want to use  get_torrent(), push_block_info_to_peer(), or add_peer_to_torrent().
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "PUSH_BLOCK_INFO") == 0)//peer gives me list of blocks they own for a torrent file
        {
            char temp_BLOCK_INFO[MAX_BLOCK_NUM];
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            torrent_file *torrent = get_torrent(torrent_hash);
            if(torrent != NULL){
                recv_socket(newsockfd, temp_BLOCK_INFO, MAX_BLOCK_NUM);//receive data
                int peer_in_list = get_peer_idx(torrent, peer_ip, peer_port);

                if(peer_in_list == -1){//if the peer that sent me the list is NOT in my peerlist
                    add_peer_to_torrent(torrent, peer_ip, peer_port, temp_BLOCK_INFO);//add peer to my ip, port lists, with BLOCK_INFO
                }
                /*
                for(int i=0; i<MAX_BLOCK_NUM; i++){
                    torrent->peer_block_info[get_peer_idx(torrent, peer_ip, peer_port)][i] = temp_BLOCK_INFO[i];//update my peer_block_info
                }*/
                else{//if peer already in peerlist
                    //printf("==================SOURCE OF ERROR=================\n");
                    /*for(int i=0; i<MAX_BLOCK_NUM; i++){
                        printf("|%c|", temp_BLOCK_INFO[i]);
                    }*/
                    
                    //printf("\n");
                    //print_torrent_info(torrent);
                    //printf("============================\n");
                    update_peer_block_info(torrent, peer_ip, peer_port, temp_BLOCK_INFO);//update peer_block_info
                    print_torrent_info(torrent);
                }
            }
            close_socket(newsockfd);
            // A peer sends its block info!
            // Peer's Message: "PUSH_BLOCK_INFO [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH]"[BLOCK_INFO]
            // Hint: You might want to use get_torrent(), update_peer_block_info(), or add_peer_to_torrent().
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "REQUEST_BLOCK") == 0) 
        {
            close_socket(newsockfd);
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            int block_index = strtol(strtok(NULL, " "), NULL, 10);
            torrent_file *torrent = get_torrent(torrent_hash);
            if(torrent != NULL){
                if(get_peer_idx(torrent, peer_ip, peer_port) == -1){//if the peer that is requesting the block is not in my list
                    add_peer_to_torrent(torrent, peer_ip, peer_port, NULL);//add peer to my ip/port list
                }
                push_block_to_peer(peer_ip, peer_port, torrent, block_index);
            }
            
            // A peer requests a block data!
            // Peer's Message: "REQUEST_BLOCK [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [BLOCK_INDEX]"
            // Hint: You might want to use get_torrent(), push_block_to_peer(), or add_peer_to_torrent().
            // TODO: Implement (5 Points)
        }
        else if (strcmp(cmd, "PUSH_BLOCK") == 0) 
        {
            unsigned int torrent_hash = strtoul(strtok(NULL, " "), NULL, 16);
            torrent_file *torrent = get_torrent(torrent_hash);
            char temp_BLOCK_DATA[torrent->block_size];
            int block_index = strtol(strtok(NULL, " "), NULL, 10);
            recv_socket(newsockfd, temp_BLOCK_DATA, torrent->block_size);//receive block data

            char *block_start_pointer = torrent->block_ptrs[block_index];//get block starting pointer
            for(int i=0; i<torrent->block_size; i++){
                block_start_pointer[i] = temp_BLOCK_DATA[i];//set bytes from starting block to BLOCK_DATA
            }
            torrent->downloaded_block_num++;//increment downloaded block num
            
            int peer_in_list = get_peer_idx(torrent, peer_ip, peer_port);
            if(peer_in_list == -1){//if peer that gave me the chunk not in list already
                add_peer_to_torrent(torrent, peer_ip, peer_port, NULL); //add to list
            }

            torrent->block_info[block_index] = 1;//signify that I have this block now
            torrent->peer_block_info[get_peer_idx(torrent, peer_ip, peer_port)][block_index] = 1;//signify that the peer who gave me this block also has it
            
            

            save_torrent_into_file(torrent, torrent->name);
            print_torrent_info(torrent);
            //save_torrent_into_file(torrent, )
            // A peer sends a block data!
            // Peer's Message: "PUSH_BLOCK [MY_LISTEN_PORT] [MY_ID_HASH] [TORRENT_HASH] [BLOCK_INDEX]"[BLOCK_DATA]
            // Hint: You might want to use get_torrent(), save_torrent_into_file(), update_peer_block_info(), or add_peer_to_torrent().
            //       You can directly use the pointer to the block data for the receive buffer of recv_socket() call.
            // TODO: Implement (5 Points)
        }
        else
        {
            if (silent_mode == 0)
                printf ("ERROR - SERVER: Received unknown command %s\n", cmd);
            close_socket(newsockfd);
        }
    }
    return 0;
}

int client_routine ()
{
    // Iterate through global torrent list and request missing blocks from peers.
    // Please DO Check network_functions.h for more information and required functions.
    
    //printf("num torrents %d\n", num_torrents);
    for (int i = 0; i < num_torrents; i++)
    {
        //printf("enter client routine loop\n");
        torrent_file *torrent = global_torrent_list[i];
        
        //print_torrent_info(torrent);
        for(int block_idx = 0; block_idx < torrent->block_num; block_idx++){
            //printf("|%c|\n", torrent->block_info[block_idx]);
            if(torrent->block_info[block_idx] == 0){
                //printf("something\n");
                for(int peer_idx = 0; peer_idx < torrent->num_peers; peer_idx++){
                    if(torrent->peer_block_info[peer_idx][block_idx] == 1){//if peer_idx has the block i want to request
                        request_block_from_peer(torrent->peer_ip[peer_idx], torrent->peer_port[peer_idx], torrent, block_idx);//request
                        torrent->peer_req_num[peer_idx]++;//increment peer_req_num
                    }
                    else{//request from peer_idx for block_idx fail
                        torrent->peer_req_num[peer_idx]++;
                        if(torrent->peer_req_num[peer_idx] > PEER_EVICTION_REQ_NUM){
                            remove_peer_from_torrent(torrent, torrent->peer_ip[peer_idx], torrent->peer_port[peer_idx]);
                        }
                    }
                }
            }
            else{
                //printf("block %d is already download\n", block_idx);
            }
        }
        // TODO:    Implement block request of the blocks that you don't have, to peers who have. (10 Points)
        //          Make sure that no more than 1 block data requests are sent to a same peer at once.
        // Hint:    Use peer_reqs array to keep track of which peers have been requested for a block.
        //          Use request_block_from_peer() to request a block from a peer. Increment peer_req_num for the peer.
        //          If request_block_from_peer() returns -1, the request failed. 
        //          If the request has failed AND peer_req_num is more than PEER_EVICTION_REQ_NUM, 
        //          evict the peer from the torrent using remove_peer_from_torrent().
    }
    
    // Iterate through global torrent list on "peer_update_interval_msec" and 
    // request block info update from all peers on the selected torrent.
    // Also, select a random peer and request its peer list, to get more peers.
    static unsigned int update_time_msec = 0;
    static unsigned int update_torrent_idx = 0;
    if (update_time_msec == 0)
    {
        update_time_msec = get_time_msec ();
        //printf("update interval is %d\n", peer_update_interval_msec);
    }
    if (update_time_msec + peer_update_interval_msec < get_time_msec () )
    {
        //printf("enter\n");
        update_time_msec = get_time_msec ();
        update_torrent_idx++;
        if (update_torrent_idx >= num_torrents)
        {
            update_torrent_idx = 0;
        }
        //printf("111\n");
        torrent_file *torrent = global_torrent_list[update_torrent_idx];
        for(int peer_idx = 0; peer_idx < torrent->num_peers; peer_idx++){
            printf("REQUESTING BLOCK INFO IN CLIENT ROUTINE\n");
            request_block_info_from_peer(torrent->peer_ip[peer_idx],torrent->peer_port[peer_idx], torrent->hash);
        }
        printf("222\n");
        if(torrent->num_peers != 0){
            int r = rand() % torrent->num_peers;
            //printf("333\n");
            printf("REQUESTING PEERLIST FROM %s IN CLIENT ROUTINE\n", torrent->peer_ip[r]);
            request_peers_from_peer(torrent->peer_ip[r], torrent->peer_port[r], torrent->hash);
        }
    }
    return 0;
}

/////////////////////////// END OF YOUR IMPLEMENTATIONS ///////////////////////////

int is_ip_valid(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return (result != 0);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    // Input parsing
    // Enter IP of the seeder in the first argument. Select peer mode (1 or 2) in the second argument.
    if (argc != 3) 
    {
        printf ("Invalid number of arguments. Usage: ./peer <SEEDER IP> <MODE 1 or 2>\n");
        return 0;
    }    
    int mode = atoi(argv[2]);
    char *seeder_ip = argv[1];
    if (mode != 1 && mode != 2)
    {
        printf ("Invalid mode. Usage: ./peer <SEEDER IP> <MODE 1 or 2>\n");
        return 0;
    }
    if (is_ip_valid(seeder_ip) == 0)
    {
        printf ("Invalid IP address. Usage: ./peer <SEEDER IP> <MODE 1 or 2>\n");
        return 0;
    }
    if (mode == 1)
    {
        printf ("INFO - Running in peer mode 1. Will connect to seeder %s:%d\n", seeder_ip, DEFAULT_PORT + 1);
    }
    else
    {
        printf ("INFO - Running in peer mode 2. Will connect to seeder %s:%d\n", seeder_ip, DEFAULT_PORT);
    }


    silent_mode = 0; // Set to 0 to enable debug messages.
    unsigned int start_time = 0, counter = 0;

    unsigned int hash_1 = 0x279cf7a5; // Hash for snu_logo_torrent.png
    unsigned int hash_2 = 0x9b7a2926; // Hash for music_torrent.mp3
    unsigned int hash_3 = 0x3dfd2916; // Hash for text_file_torrent.txt
    unsigned int hash_4 = 0xa53e35f4; // Hash for test.pdf

    // Peer 1 - (Port 12781)
    if (mode == 1)
    {
        // Make some files into torrents
        make_file_into_torrent("text_file_torrent.txt", "text_file.txt");           // HASH: 0x3dfd2916
        //printf("made textfile1 torrent \n");
        make_file_into_torrent("test_torrent.pdf", "test.pdf");                     // HASH: 0xa53e35f4
        printf("make textfile2 torrent \n");
        // Initialize listening socket
        int sockfd = listen_socket(DEFAULT_PORT);
        if (sockfd < 0) 
        {
            return -1;
        }
        // Wait 5 seconds before starting
        for (int countdown = 0; countdown > 0; countdown--) 
        {
            printf("Starting in %d seconds...\r", countdown);
            fflush(stdout);
            sleep_ms(1000);
        }
        while (1) 
        {
            // Run server & client routines concurrently
            server_routine_ans(sockfd, seeder_ip, DEFAULT_PORT + mode - 1);     // Your implementation of server_routine() should be able to replace server_routine_ans() in this line.
            client_routine_ans();           // Your implementation of client_routine() should be able to replace client_routine_ans() in this line.
            server_routine_ans(sockfd, seeder_ip, DEFAULT_PORT + mode - 1);     // Your implementation of server_routine() should be able to replace server_routine_ans() in this line.

            // Take some action every "SLEEP_TIME_MSEC" milliseconds
            if (start_time == 0 || start_time + SLEEP_TIME_MSEC < get_time_msec())
            {
                printf("action\n");
                if (1 == counter) 
                {
                    
                    // Request torrents from seeder (peer 2), using their hash values
                    request_from_hash (hash_1, seeder_ip, DEFAULT_PORT + 1); // Hash for snu_logo_torrent.png
                    printf("===============request snulogo=======================\n");
                    request_from_hash (hash_2, seeder_ip, DEFAULT_PORT + 1); // Hash for music_torrent.mp3
                    //printf("request music\n");
                }
                start_time = get_time_msec();
                //print_all_torrents();
                print_torrent_status ();
                printf("===============request snulogo=======================\n");
                counter++;
            }
        }
        close_socket(sockfd);
    }
    // Peer 2 - (Port 12782)
    else if (mode == 2)
    {
        // Make some files into torrents
        make_file_into_torrent("snu_logo_torrent.png", "snu_logo.png");         // HASH: 0x279cf7a5
        make_file_into_torrent("music_torrent.mp3", "music.mp3");               // HASH: 0x9b7a2926 (source: https://www.youtube.com/watch?v=9PRnPdgNhMI)
        // Initialize listening socket
        int sockfd = listen_socket(DEFAULT_PORT + 1);
        if (sockfd < 0) 
        {
            return -1;
        }
        // Wait 5 seconds before starting
        for (int countdown = 5; countdown > 0; countdown--) 
        {
            printf("Starting in %d seconds...\r", countdown);
            fflush(stdout);
            sleep_ms(1000);
        }
        while (1) 
        {
            // Run server & client routines concurrently
            server_routine_ans(sockfd, seeder_ip, DEFAULT_PORT + mode - 1);     // Your implementation of server_routine() should be able to replace server_routine_ans() in this line.
            client_routine_ans();           // Your implementation of client_routine() should be able to replace client_routine_ans() in this line.
            server_routine_ans(sockfd, seeder_ip, DEFAULT_PORT + mode - 1);     // Your implementation of server_routine() should be able to replace server_routine_ans() in this line.

            // Take some action every "SLEEP_TIME_MSEC" milliseconds
            if (start_time == 0 || start_time + SLEEP_TIME_MSEC < get_time_msec())
            {
                if (1 == counter) 
                {
                    // Request torrents from seeder (peer 1), using their hash values
                    request_from_hash (hash_3, seeder_ip, DEFAULT_PORT); // Hash for text_file_torrent.txt
                    request_from_hash (hash_4, seeder_ip, DEFAULT_PORT); // Hash for test.pdf
                    printf("=========================request test.pdf=============================\n");
                }
                start_time = get_time_msec();
                // print_all_torrents();
                print_torrent_status ();
                printf("=========================request test.pdf=============================\n");
                counter++;
            }
        }
        close_socket(sockfd);
    }
    return 0;
}