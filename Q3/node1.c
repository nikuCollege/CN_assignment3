#include <stdio.h>

extern struct rtpkt {
  int sourceid;
  int destid;
  int mincost[4];
};

extern int TRACE;
extern int YES;
extern int NO;
extern float clocktime;

struct distance_table 
{
  int costs[4][4];
} dt1;

static int mincost[4] = {1, 0, 1, 999};  // Direct costs from node 1

void rtinit1() 
{
    printf("rtinit1 called at time %f\n", clocktime);
    
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(i == 1) dt1.costs[i][j] = 0;
            else if(j == 1) dt1.costs[i][j] = 999;
            else if(i == j) dt1.costs[i][j] = mincost[i];
            else dt1.costs[i][j] = 999;
        }
    }
    
    struct rtpkt pkt;
    creatertpkt(&pkt, 1, 1, mincost);
    pkt.destid = 0; tolayer2(pkt);
    pkt.destid = 2; tolayer2(pkt);
    printf("Sent initial packets to nodes 0 and 2\n");
    
    printdt1(&dt1);
}

void rtupdate1(struct rtpkt *rcvdpkt)
{
    printf("rtupdate1 called at time %f, received packet from node %d\n", 
           clocktime, rcvdpkt->sourceid);
           
    int updated = NO;
    int src = rcvdpkt->sourceid;
    
    for(int i = 0; i < 4; i++) {
        int newcost = mincost[src] + rcvdpkt->mincost[i];
        if(newcost < dt1.costs[i][src]) {
            dt1.costs[i][src] = newcost;
            updated = YES;
        }
    }
    
    if(updated) {
        for(int i = 0; i < 4; i++) {
            int min = 999;
            for(int j = 0; j < 4; j++) {
                if(dt1.costs[i][j] < min) min = dt1.costs[i][j];
            }
            if(min != mincost[i]) {
                mincost[i] = min;
                struct rtpkt pkt;
                creatertpkt(&pkt, 1, 1, mincost);
                pkt.destid = 0; tolayer2(pkt);
                pkt.destid = 2; tolayer2(pkt);
                printf("Sent updated packets to nodes 0 and 2\n");
            }
        }
    }
    
    printdt1(&dt1);
    printf("Distance table %supdated\n", updated ? "" : "not ");
}

void printdt1(struct distance_table *dtptr)
{
    printf("             via   \n");
    printf("   D1 |  0   2 \n");
    printf("  ----|-----------\n");
    printf("     0|  %3d %3d\n", dtptr->costs[0][0], dtptr->costs[0][2]);
    printf("dest 2|  %3d %3d\n", dtptr->costs[2][0], dtptr->costs[2][2]);
    printf("     3|  %3d %3d\n", dtptr->costs[3][0], dtptr->costs[3][2]);
}

void linkhandler1(int linkid, int newcost)
{
    printf("linkhandler1 called at time %f, link to %d cost changed to %d\n",
           clocktime, linkid, newcost);
           
    int oldcost = mincost[linkid];
    mincost[linkid] = newcost;
    
    for(int i = 0; i < 4; i++) {
        if(i != 1) {
            dt1.costs[i][linkid] = (dt1.costs[i][linkid] - oldcost) + newcost;
        }
    }
    
    struct rtpkt pkt;
    creatertpkt(&pkt, 1, 1, mincost);
    pkt.destid = 0; tolayer2(pkt);
    pkt.destid = 2; tolayer2(pkt);
    printf("Sent updated packets to nodes 0 and 2\n");
    
    printdt1(&dt1);
}