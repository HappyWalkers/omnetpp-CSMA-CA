network HybridMAC
{
    parameters:
//        double width;
//        double height;
        int dataNodeNum;
//        int queueSize;
//        double rounds;
//
//        double CW_checkInterval;
//        int CWmin;
//        double DIFS_checkInterval;
//        double DIFS;
//        double data_packet_transmission_time;
//        double conflict_time;
//        int retryLimit;
//        int backoffStageLimit;
//        double lamda;
//        double packetSize;
//        double networkSpeed;

    submodules:
        node[dataNodeNum] : Node {
        }
    connections allowunconnected:
        for i=0..dataNodeNum-1 , for j=0..dataNodeNum-1{
        	node[i].out[j] --> node[j].in[i] if i!=j;
        }
}