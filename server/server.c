//
//  main.c
//  computernetwork
//
//  Created by mac on 2023/11/5.
//

#ifndef server_h
#define server_h

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#endif /* server_h */

//primitives
#define PORT 6666
#define STARTPACKETID 0XFFFF
#define ENDPACKETID 0XFFFF
#define CLIENTID 0XFF
#define TIMEOUT 5

//packet types
#define DATATYPE 0XFFF1
#define ACKTYPE 0xFFF2
#define REJTYPE 0XFFF3

//reject sub code
#define OUTOFSEQUENCE 0XFFF4
#define LENGTHMISMATCH 0XFFF5
#define PACKETENDMISSING 0XFFF6
#define DUPLICATEPACKET 0XFFF7

//define data packet
struct data_packet{
    uint16_t start_id;
    uint8_t cli_id;
    uint16_t type;
    uint8_t seg_no;
    uint8_t len;
    char payload[255];
    uint16_t end_id;
};

//define ack packet format
struct ack_packet{
    uint16_t start_id;
    uint8_t cli_id;
    uint16_t type;
    uint8_t rec_seg_no;
    uint16_t end_id;
};

//define reject packet format
struct rej_packet{
    uint16_t start_id;
    uint8_t cli_id;
    uint16_t type;
    uint16_t rej_sub;
    uint8_t rec_seg_no;
    uint16_t end_id;
};


// Create Acknowledgement Packet
struct ack_packet create_ack(struct data_packet dp) {
    struct ack_packet ack;
    ack.start_id = dp.start_id;
    ack.cli_id = dp.cli_id;
    ack.type = ACKTYPE;
    ack.rec_seg_no = dp.seg_no;
    ack.end_id = dp.end_id;
    return ack;
}

// Creating Reject Packet
struct rej_packet create_rej(struct data_packet dp) {
    struct rej_packet rej;
    rej.start_id = rej.start_id;
    rej.cli_id = dp.cli_id;
    rej.rec_seg_no = dp.seg_no;
    rej.type = REJTYPE;
    rej.end_id = dp.end_id;
    return rej;
}


void error(const char *msg){
    perror(msg);
    exit(1);
}



int main(int argc, const char * argv[]) {
    // insert code here...
    
    int sock, n;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    struct data_packet dp;
    struct ack_packet ack;
    struct rej_packet rej;
    
    //Create a socket
    printf("---------------Create the socket-------------\n");
    sock = socket(AF_INET, SOCK_DGRAM, 0);;
    if(sock<0)
        error("ERROR: opening socket");
    
    //Bind the socket
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    
    if( bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
        error("ERROR: binding socket");
    }
    printf("-------Server Initialized Successfully-------\n");
    
    //Create an array to track received datapacket.
    int received[10];
    for(int i = 0;i<10;i++){
        received[i]=0;
    }
    int next_packet_no = 1;
    
    while(1){
        cli_len = sizeof(cli_addr);
        n = recvfrom(sock, &dp, sizeof(struct data_packet), 0, (struct sockaddr *)&cli_addr, &cli_len);
        if(n<0) error("ERROR: receive from client");
        
        
        //ERROR HANDLING:
        int length = strlen(dp.payload);
        
        //Case 1: Duplicate Packet
        if(received[dp.seg_no-1]==1){
            rej = create_rej(dp);
            rej.rej_sub=DUPLICATEPACKET;
            sendto(sock,&rej,sizeof(struct rej_packet), 0, (struct sockaddr *)&cli_addr,cli_len);
            printf("*******RECEIVED A DUPLICATE PACKET*******\n");
        }
        //Case 2: Length Mismatch
        else if(dp.len != length){
            rej = create_rej(dp);
            rej.rej_sub=LENGTHMISMATCH;
            sendto(sock,&rej,sizeof(struct rej_packet), 0, (struct sockaddr *)&cli_addr,cli_len);
            printf("*******RECEIVED PACKET HAS A MISMATCH LENGTH*******\n");
        }
        //Case 3: Out of Sequence
        else if(dp.seg_no != next_packet_no){
            rej = create_rej(dp);
            rej.rej_sub=OUTOFSEQUENCE;
            sendto(sock,&rej,sizeof(struct rej_packet), 0, (struct sockaddr *)&cli_addr,cli_len);
            printf("*******RECEIVED PACKET IS OUT OF SEQUENCE*******\n");
        }
        //Case 4: Packet Missing
        else if(dp.end_id != ENDPACKETID){
            rej = create_rej(dp);
            rej.rej_sub=PACKETENDMISSING;
            sendto(sock,&rej,sizeof(struct rej_packet), 0, (struct sockaddr *)&cli_addr,cli_len);
            printf("*******RECEIVED PACKET IS MISSING END PACKET ID*******\n");
        }
        else {
            if(dp.seg_no ==11){
                sleep(10);
            }
            //SEND ACK:
            ack = create_ack(dp);
            n=sendto(sock, &ack, sizeof(struct ack_packet), 0, (const struct sockaddr *)&cli_addr, cli_len);
            printf("Packet received and acknowledged\n");
            if(n<0){
                error("SEND ERROR");
            }
        }
        received[dp.seg_no-1]++;
        next_packet_no++;
        if(next_packet_no==12){
            printf("-------FINISHED RECEIVING ALL PACKETS. SERVER CLOSING-------\n");
            exit(1);
        }
        printf("-------New packet below-------\n");
    }
}
