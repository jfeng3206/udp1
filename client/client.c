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
#define TIMEOUT 3
#define FILENAME "datapacket.txt";


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
    uint16_t start_packet_id;
    uint8_t client_id;
    uint16_t type;
    uint8_t segment_no;
    uint8_t len;
    char payload[255];
    uint16_t end_packet_id;
};

//define ack packet format
struct ack_packet{
    uint16_t start_packet_id;
    uint8_t client_id;
    uint16_t type;
    uint8_t received_segment_no;
    uint16_t end_packet_id;
};

//define reject packet format
struct rej_packet{
    uint16_t start_packet_id;
    uint8_t client_id;
    uint16_t type;
    uint16_t reject_sub_code;
    uint8_t received_segment_no;
    uint16_t end_packet_id;
};


// Create Data Packet
struct data_packet create_dp(void) {
    struct data_packet dp;
    dp.start_packet_id = STARTPACKETID;
    dp.client_id = CLIENTID;
    dp.type=DATATYPE;
    dp.end_packet_id = ENDPACKETID;
    return dp;
}

//Error method
void error(const char *msg){
    perror(msg);
    exit(1);
}


int main(void) {
    int sock, len;
    socklen_t addr_len;
    struct sockaddr_in serv_addr, from;
    struct data_packet dp;
    struct rej_packet received;
    FILE *file;
    char line[255];
    int n = 0;
    int retry = 0;
    int seg_num = 1;

    //Create a socket
    printf("---------------Create the socket-------------\n");
    sock = socket(AF_INET, SOCK_DGRAM, 0);;
    if(sock<0)
        error("ERROR: opening socket");
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    
    //Read datapacket from datapacket.txt
    file = fopen("datapacket.txt", "rt");
    if (file == NULL) {
        error("ERROR: input file");
    }
    dp = create_dp();
    while (fgets(line, sizeof(line), file) != NULL){
        n=0;
        retry = 0;
        dp.segment_no=seg_num;
        strcpy(dp.payload,line);
        dp.len = strlen(dp.payload);
        
        //Case 1: Duplicate Packet
        if(seg_num == 7){
            dp.segment_no=5;
        }
        //Case 2: Length Mismatch
        else if(seg_num==8){
            dp.len++;
        }
        //Case 3: Out of Sequence
        if(seg_num == 9){
            dp.segment_no=dp.segment_no+10;
        }
        //Case 4: Packet Missing
        if(seg_num == 10){
            dp.end_packet_id=0;
        }
        //Case 5: ACK Timer
        if(seg_num == 11){
            dp.end_packet_id=ENDPACKETID;
        }
        
        
        while(n<=0 &&  retry <3){
            //send data packet to server
            addr_len=sizeof(struct sockaddr_in);
            n=sendto(sock,&dp,sizeof(struct data_packet),0,(struct sockaddr *)&serv_addr,addr_len);
            if (n < 0) error("Sendto");
            
            //set timer
            struct timeval tv;
            tv.tv_sec = TIMEOUT;
            tv.tv_usec = 0;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
            
            //recieving response
            n = recvfrom(sock,&received,sizeof(struct rej_packet),0,(struct sockaddr *)&from, &addr_len);
            
            if (n <= 0 ) {
                printf("******Failed to receive a response. Retrying..*****\n");
                retry ++;
            }else if (received.type== ACKTYPE  ) {
                printf("ACK response received\n");
            }else if(received.type == REJTYPE){
                printf("******Reject response received******\n");
                printf("******The Type is: %x******\n" , received.reject_sub_code);
                if(received.reject_sub_code==LENGTHMISMATCH){
                    printf("******Length Mismatch******\n");
                }
                if(received.reject_sub_code==OUTOFSEQUENCE){
                    printf("******Packet Out of Sequence******\n");
                }
                if(received.reject_sub_code==PACKETENDMISSING){
                    printf("******Packet End Missing******\n");
                }
                if(received.reject_sub_code==DUPLICATEPACKET){
                    printf("******Duplicate Packet******\n");
                }
            }
        }
        if (retry >=3){
            printf("******SERVER NOT RESPONDING*****\n");
            exit(0);
        }
        seg_num++;
        if(seg_num>11){
            printf("------All datapackets have been sent------\n");
            exit(0);
        }
        printf("------New packet below------\n");
    }
}
