/*
 * Node.h
 *
 *  Created on: Dec 24, 2020
 *      Author: mashedpotatoes
 */
#ifndef NODE_H
#define NODE_H

#include <string>
#include <omnetpp.h>
#include <vector>
#include <queue>
#include <unistd.h>
#include <message_m.h>
#include <cmath>

using namespace std;
using namespace omnetpp;

#define SIFS_time (10*pow(10,-6))
#define DIFS_time (50*pow(10,-6))
#define data_transmission_time (1000*pow(10,-6))
#define slot_time (20*pow(10,-6))//back-off unit time
#define cw_min 1
#define lamda 0.5//the parameter of poisson distribution (1/lamda is the parameter of expotential distribution)
#define node_num 3
#define queue_length 1000//if too long, busy with generate packets
#define retry_limit 7
#define backoff_stage_limit 8

#define id_generate_data_packets_event 1
#define id_check_data_queue_event 2
#define id_check_channel_busy_event 3
#define id_send_RTS_broadcast_event 4
#define id_send_CTS_broadcast_event 13
#define id_received_count_event 5
#define id_random_backoff_event 6
#define id_send_data_event 7
#define id_get_RTS_event 8
#define id_get_CTS_event 9
#define id_startup_event 14
#define id_send_ACK_event 16

#define message_data 10
#define RTS_data 11
#define CTS_data 12
#define ACK_data 15


bool busy=false;//flag of channel status (true means busy, false means not busy)
int send_num_same_time=0;//prevent sending RTS at the same time
double start_time;
double end_time;

int generated_packets;
int sent_packets;
int loss_packets;

class Node:public cSimpleModule
{
private:
    double x;
    double y;
    int destination_id_tmp;
    int retry_counter;
    int backoff_stage;
    int backoff_counter;
    bool check_data_queue_flag;//when the flag==true,  polling to check if there is data to send
    queue <Message*>  data_queue;//the queue to store all unsent data
    int num_data_queue;

    void startup();
    void generate_data_packets();
    void check_data_queue();
    void check_channel_busy();
    void send_RTS_broadcast();
    void send_CTS_broadcast(cMessage* msg);
    void received_count();
    void random_backoff();
    void send_data();
    void get_data(cMessage* msg);
    void get_RTS(cMessage* msg);
    void get_CTS(cMessage* msg);
    void send_ACK(cMessage* msg);
    void get_ACK(cMessage* msg);

    cMessage* generate_data_packets_event;
    cMessage* check_data_queue_event;
    cMessage* check_channel_busy_event;
    cMessage* send_RTS_broadcast_event;
    cMessage* send_CTS_broadcast_event;
    cMessage* received_count_event;
    cMessage* random_backoff_event;
    cMessage* send_data_event;
    cMessage* get_RTS_event;
    cMessage* get_CTS_event;
    cMessage* startup_event;
    cMessage* message_data_event;
    cMessage* send_ACK_event;

    cMessage* random_backoff_CTS_event;
    cMessage* random_backoff_ACK_event;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

public:
    static vector <Node*> node_vector;//the vector to store all nodes

    Node();
    ~Node();
};

#endif //NODE_H
