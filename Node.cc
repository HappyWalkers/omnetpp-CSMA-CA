/*
 * Node.cc
 *
 *  Created on: Dec 24, 2020
 *      Author: mashedpotatoes
 */

#include "Node.h"

vector<Node *> Node::node_vector;

Define_Module(Node);
//
Node::Node()
{
    generate_data_packets_event =
    check_data_queue_event =
     check_channel_busy_event =
      send_RTS_broadcast_event =
     send_CTS_broadcast_event =
     received_count_event =
     random_backoff_event =
      send_data_event =
      get_RTS_event =
      get_CTS_event =
      startup_event=
      message_data_event=
      random_backoff_ACK_event=
      random_backoff_CTS_event=
      send_ACK_event=
      NULL;
}

Node::~Node()
{
    cancelAndDelete(generate_data_packets_event);
    cancelAndDelete(check_data_queue_event);
    cancelAndDelete(check_channel_busy_event);
    cancelAndDelete(send_RTS_broadcast_event);
    cancelAndDelete(send_CTS_broadcast_event);
    cancelAndDelete(received_count_event);
    cancelAndDelete(random_backoff_event);
    cancelAndDelete(send_data_event);
    cancelAndDelete(get_RTS_event);
    cancelAndDelete(get_CTS_event);
    cancelAndDelete(startup_event);
    cancelAndDelete(message_data_event);
    cancelAndDelete(random_backoff_ACK_event);
    cancelAndDelete(random_backoff_CTS_event);
    cancelAndDelete(send_ACK_event);
}

void Node::initialize()
{
    node_vector.push_back(this);
    generate_data_packets_event = new cMessage("generate_data_packets_event");
    check_data_queue_event = new cMessage("check_data_queue_event");
    check_channel_busy_event = new cMessage("check_channel_busy_event");
    send_RTS_broadcast_event = new cMessage("send_RTS_broadcast_event");
    send_CTS_broadcast_event = new cMessage("send_CTS_broadcast_event");
    received_count_event = new cMessage("received_count_event");
    random_backoff_event = new cMessage("random_backoff_event");
    send_data_event = new cMessage("send_data_event");
    get_RTS_event = new cMessage("get_RTS_event");
    get_CTS_event = new cMessage("get_CTS_event");
    startup_event=new cMessage("startup_event");
    message_data_event=new cMessage("message_data_event");
    random_backoff_CTS_event=new cMessage("random_backoff_CTS_event");
    random_backoff_ACK_event=new cMessage("random_backoff_ACK_event");
    send_ACK_event=new cMessage("send_ACK_event");

    generate_data_packets_event->setKind(id_generate_data_packets_event);
    check_data_queue_event->setKind(id_check_data_queue_event);
    check_channel_busy_event->setKind(id_check_channel_busy_event);
    send_RTS_broadcast_event->setKind(id_send_RTS_broadcast_event);
    send_CTS_broadcast_event->setKind(id_send_CTS_broadcast_event);
    received_count_event->setKind(id_received_count_event);
    random_backoff_event->setKind(id_random_backoff_event);
    send_data_event->setKind(id_send_data_event);
    get_RTS_event->setKind(id_get_RTS_event);
    get_CTS_event->setKind(id_get_CTS_event);
    startup_event->setKind(id_startup_event);
    message_data_event->setKind(message_data);
    random_backoff_CTS_event->setKind(id_random_backoff_event);
    random_backoff_ACK_event->setKind(id_random_backoff_event);
    send_ACK_event->setKind(id_send_ACK_event);

    this->x = par("x");
    this->y = par("y");
    this->retry_counter = 0;
    this->backoff_counter = 0;
    this->backoff_stage = 2;
    generated_packets = 0;
    sent_packets = 0;
    loss_packets = 0;
    this->check_data_queue_flag = true;
    this->num_data_queue=0;

    srand(this->getId());
    double sleep_time = (double)(rand() % 1000)/pow(10,6);
    this->scheduleAt(simTime()+sleep_time,startup_event);
}

void Node::startup()
{
    start_time = SIMTIME_DBL(simTime());

    this->scheduleAt(simTime(), generate_data_packets_event); //start to generate data

    this->scheduleAt(simTime(), check_data_queue_event); //start to check data queue is empty or not
}

void Node::finish()
{
    if (this->getId() - 2 == node_num-1)
    {
        end_time = SIMTIME_DBL(simTime());
        double running_time = end_time - start_time;
        cout << "number of node: " << node_num << endl;
        cout<<"lamda: "<<lamda<<endl;
        cout << "number of generated packets: " << generated_packets << endl;
        cout << "number of sent packets: " << sent_packets << endl;
        cout << "number of lost packets: " << loss_packets << endl;
        cout << "running time: " << running_time << endl;
        cout << "through output: " << sent_packets / running_time << endl;
    }
}

void Node::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        switch (msg->getKind())
        {
        case id_startup_event:
            startup();
            break;
        case id_generate_data_packets_event:
            generate_data_packets();
            break;
        case id_check_data_queue_event:
            check_data_queue();
            break;
        case id_check_channel_busy_event:
            check_channel_busy();
            break;
        case id_send_RTS_broadcast_event:
            send_RTS_broadcast();
            break;
//        case id_send_CTS_broadcast_event:
//            send_CTS_broadcast(msg);
//            break;
        case id_random_backoff_event:
            random_backoff();
            break;
        case id_send_data_event:
            send_data();
            break;
        case id_send_ACK_event:
            send_ACK(msg);
            break;
        }
    }
    else
    {
        switch (msg->getKind())
        {
        case RTS_data:
            get_RTS(msg);
            break;
        case CTS_data:
            get_CTS(msg);
            break;
        case  message_data:
            get_data(msg);
            break;
        case ACK_data:
            get_ACK(msg);
            break;

        }
    }
}

//generate a message into data Queue and send a data_generation_event to the node self
void Node::generate_data_packets()
{
    if (this->data_queue.empty() || (this->data_queue.size() < queue_length))//if dataQueue is not full, generate a message and push into dataQueue
    {
        Message *data = new Message(); //generate a message
        data->setSource_id(this->getId()-2);
        data->setKind(message_data);
        data->setGeneration_time(simTime().dbl());
        this->data_queue.push(data); //push into dataQueue
        num_data_queue++;
        generated_packets++;

        bubble("generate a packet");
    }
    double time =exponential(1 / lamda)/10000;                            //this line means "zhishu" distribution, the relationship between the exponential distribution and poisson distribution: http://www.ruanyifeng.com/blog/2015/06/poisson-distribution.html
//    double time =exponential(1 / lamda);
    this->scheduleAt(simTime() + time, generate_data_packets_event); //send data_generation_event to the node self with poisson distribution
}

void Node::check_data_queue()
{
    if (check_data_queue_flag == true)
    {
        if (this->data_queue.empty() == true)
        {
            this->scheduleAt(simTime() + DIFS_time, check_data_queue_event);
            bubble("data queue is empty");
        }
        else //if there is data to send, then goto check channel
        {
            check_data_queue_flag=false;//when deal with a data, stop to check the data queue to avoid  sending another data
            this->scheduleAt(simTime(), check_channel_busy_event);
        }
    }
    else //when deal with a data, pooling to try until the flag==true
    {
        this->scheduleAt(simTime() + DIFS_time, check_data_queue_event);
    }
}

void Node::check_channel_busy()
{
//    send_num_same_time++;
//    cout<< send_num_same_time <<endl;
//    if(send_num_same_time>1)//if there are more than one nodes try to send RTS
//    {
//        busy=true;
//    }
    if (busy == false) //if channel is not busy, goto send RTS
    {
//        send_num_same_time--;

        bubble("checking channel: channel is not busy");
        this->scheduleAt(simTime(), send_RTS_broadcast_event);

        backoff_stage=2;//reset the back-off stage
    }
    else //if channel is busy, then goto random back-off
    {
        backoff_counter=(int)uniform(1,cw_min*pow(2,backoff_stage));
        this->scheduleAt(simTime(), random_backoff_event);
    }
}

void Node::send_RTS_broadcast()
{
    bubble("sending RTS");
    send_num_same_time++;
//    busy = true; //channel is busy

    int id_dest = rand() % node_num;
    while(id_dest==this->getId()-2)//the destination can not be the node self
    {
        id_dest = rand() % node_num;
    }
    destination_id_tmp=id_dest;

    Message *RTS = new Message();
    RTS->setSource_id(this->getId());
    RTS->setDestination_id(destination_id_tmp);
    RTS->setKind(RTS_data);
    RTS->setGeneration_time(simTime().dbl());

    send(RTS, "out", destination_id_tmp);

    backoff_counter=(int)uniform(1,cw_min*pow(2,backoff_stage));
    this->scheduleAt(simTime()+DIFS_time, random_backoff_CTS_event);//assume that no CTS, if get CTS, cancel it

}

void Node::send_CTS_broadcast(cMessage* msg)
{
    bubble("sending CTS");

    busy = true; //channel is busy

    int id_dest = msg->getSenderModuleId();

    for (int i = 0; i < node_num; i++) //broadcast
    {
        if (i != this->getId() - 2) //except the node self
        {
            Message *CTS = new Message();
            CTS->setSource_id(this->getId()-2);
            CTS->setDestination_id(id_dest);
            CTS->setKind(CTS_data);
            CTS->setGeneration_time(simTime().dbl());

            send(CTS, "out", i);
        }
    }
}

void Node::random_backoff()
{
    bubble("random back-off");

    if(busy == false && backoff_counter != 0){//if the channel is not busy, loop to wait
        backoff_counter--;
        if(backoff_counter != 0)//loop to wait
        {
            this->scheduleAt(simTime() + slot_time, random_backoff_event);
        }
        else//loop end
        {
            backoff_stage++;
            if(backoff_stage>backoff_stage_limit)//over the limit, drop it
            {
                bubble("over the retry limit, drop a data");
                backoff_stage=2;
                this->data_queue.pop();
                loss_packets++;
                num_data_queue--;
                busy=false;
            }
            check_data_queue_flag=true;
            this->scheduleAt(simTime(), check_data_queue_event);
        }

    }
    else if(busy == true && backoff_counter != 0)
    {
        bubble("channel is busy, random back-off suspend");

         this->scheduleAt(simTime() + data_transmission_time, random_backoff_event);
    }

    // while(backoff_counter!=0)
    // {
    //     if(busy==false)//if the channel is not busy, then loop
    //     {
    //         usleep((int)slot_time*pow(10,6));
    //         backoff_counter--;
    //     }
    //     else//if the channel is busy, then suspend
    //     {
    //         usleep((int)data_transmission_time*pow(10,6));
    //     }

    // }
}

void Node::get_RTS(cMessage *msg)
{
    Message* msg_src=(Message*)msg;
    if (msg_src->getDestination_id()== this->getId()-2) //when the dest receive the RTS, it give back a CTS
    {
        // cout<<SIFS_time<<endl;
        // this->scheduleAt(msg->getArrivalTime()+ SIFS_time, send_CTS_broadcast_event);
        sleep(SIFS_time);
        send_CTS_broadcast(msg);
    }
    else //if not the dest, drop it
    {
        delete (msg);
    }
}

void Node::get_CTS(cMessage *msg)
{
    Message* msg_src=(Message*)msg;
    if(msg_src->getDestination_id()==this->getId())//the dest get the CTS, then goto send data
    {
        if(send_num_same_time==1)
        {
            bubble("get a CTS");
            this->cancelEvent(random_backoff_CTS_event);
            this->scheduleAt(simTime()+SIFS_time+data_transmission_time,send_data_event);//add the data transmission time in here
            backoff_stage=2;//reset the back-off stage
        }
        else
        {
            bubble("collision happened");
            send_num_same_time=0;
            busy=false;
        }
    }
    else//if other nodes get the CTS, then keep silence for some time
    {
        bubble("keep silence");
        sleep(data_transmission_time);//?????????????????????
    }
}

void Node::send_data()
{
    bubble("sending data");

    send(this->data_queue.front(),"out",destination_id_tmp);

    backoff_counter=(int)uniform(1,cw_min*pow(2,backoff_stage));
    this->scheduleAt(simTime()+DIFS_time, random_backoff_ACK_event);//assume that no ACK, if get ACK, cancel it
}

void Node::get_data(cMessage* msg)
{
    bubble("get data");
    msg->setKind(id_send_ACK_event);
    this->scheduleAt(simTime()+SIFS_time,msg);
}

void Node::send_ACK(cMessage* msg)
{
    Message* msg_src=(Message*)msg;

    Message *ACK = new Message();
    ACK->setSource_id(this->getId()-2);
    ACK->setDestination_id(msg_src->getSource_id());
    ACK->setKind(ACK_data);
    ACK->setGeneration_time(simTime().dbl());

    sleep(SIFS_time);
    bubble("sending ACK");
    send(ACK, "out", msg_src->getSource_id());
}

void Node::get_ACK(cMessage* msg)
{
    if(send_num_same_time==1)
    {
        bubble("get ACK");
        this->data_queue.pop();
        sent_packets++;
        num_data_queue--;
        this->cancelEvent(random_backoff_ACK_event);

        backoff_stage=2;//reset back-off stage
        busy=false;//set the channel is not busy
        send_num_same_time--;


        if(data_queue.empty()==true)//if the data queue is empty, then start the next data_send
        {
            check_data_queue_flag=true;
            this->scheduleAt(simTime()+DIFS_time, check_data_queue_event);
        }
        else//if the data queue is not empty, means there are still some data to send, so random back-off
        {
            backoff_counter=(int)uniform(1,cw_min*pow(2,backoff_stage));
            this->scheduleAt(simTime()+DIFS_time, random_backoff_event);
        }
    }
    else
    {
        bubble("collision happened");
        send_num_same_time=0;
        busy=false;
    }

}
