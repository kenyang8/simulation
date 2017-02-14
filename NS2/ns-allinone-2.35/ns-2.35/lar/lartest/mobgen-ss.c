/****************************************************************************** 
 *   File Name: mobgen-ss.c 
 *   Purpose: The only main file for mobgen-ss 
 *   Original mobgen Author: Jeff Boleng <jeff@boleng.com> 
 *   Modified by: Nick Bauer 
 *   Date Created: June 2003 
 * 
 *   Copyright (C) 2004  Toilers Research Group -- Colorado School of Mines 
 * 
 *   Please see COPYRIGHT.TXT and LICENSE.TXT for copyright and license 
 *   details. 
 * 
 *   History: 
 *   Calculation of pause time updated, thanks to an error found by Kiran 
 *       Vadde (Arizona State University). -NBB 7/20/03 
 * 
 *   This program will generate mobility files using the steady-state 
 *       random waypoint model.  This program will work for any rectangular 
 * 
 *   simulation area and with any amount of pause time. -NBB 6/15/03 
 * 
 *   This program will also output QualNet mobility files. -NBB 5/28/03 
*******************************************************************************/  
  
#include <stdlib.h>  
#include <stdio.h>  
#include <time.h>  
#include <math.h>  
  
#undef rand48  
  
#define true  1  
#define false 0  
  
int main(int argc, char *argv[])  
{  
  int i;  
  int numNodes=0;  
  int nextNode=0;  
  double maxX=0.0, maxY=0.0;  
  double endTime=0.0, lowest;  
  double speedMean=0.0, speedDelta=0.0;  
  double pauseMean=0.0, pauseDelta=0.0;  
  int *moving;  
  double *nextEvent;  
  double *xLoc, *yLoc;  
  double newX, newY, speed, dist;  
  double speedLow, pauseLow;  
  double speedRange, pauseRange;  
  double u1, u2, r, x1, x2, y1, y2, x, y; //steady-state initial position  
  double u, v0, v1, s;           //steady-state initial velocity  
  char output;  
  double expectedPauseTime,expectedTravelTime,probabilityPaused,a,b,pauseTime;  
  double log1, log2, t1, t2;  
  
  double pauseTime2; //I need this for QualNet output -NBB  
  
  if (argc == 10)  
  {  
    numNodes   = atoi(argv[1]);  
    maxX       = atof(argv[2]);  
    maxY       = atof(argv[3]);  
    endTime    = atof(argv[4]);  
    speedMean  = atof(argv[5]);  
    speedDelta = atof(argv[6]);  
    pauseMean  = atof(argv[7]);  
    pauseDelta = atof(argv[8]);  
    output     = argv[9][0];  
  
    if(numNodes<=0)  
    {  
      fprintf(stderr,"Error: number of nodes must be greater than 0\n");  
      return -1;  
    }  
    if((maxX<=0)||(maxY<=0))  
    {  
      fprintf(stderr,"Error: max-x and max-y must be greater than 0\n");  
      return -1;  
    }  
    if(endTime<0)  
    {  
      fprintf(stderr,"Error: end time must be greater than or equal to 0\n");  
      return -1;  
    }  
    if(speedMean<=0)  
    {  
      fprintf(stderr,"Error: speed mean must be greater than 0\n");  
      return -1;  
    }  
    if((speedDelta>=speedMean)||(speedDelta<0))  
    {  
      fprintf(stderr,"Error: speed delta must be greater than or equal to 0 and less than speed mean\n");  
      return -1;  
    }  
    if(pauseMean<0)  
    {  
      fprintf(stderr,"Error: pause mean must be greater than or equal to 0\n");  
      return -1;  
    }  
    if((pauseDelta>pauseMean)||(pauseDelta<0))  
    {  
      fprintf(stderr,"Error: pause delta must be greater than or equal to 0 and less than or equal to pause mean\n");  
      return -1;  
    }  
  
    fprintf(stdout, "#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");  
    fprintf(stdout, "#\tSteady-state Random Waypoint Model\n");  
    fprintf(stdout, "#\tnumNodes   = %6d\n", numNodes);  
    fprintf(stdout, "#\tmaxX       = %9.2f\n", maxX);  
    fprintf(stdout, "#\tmaxY       = %9.2f\n", maxY);  
    fprintf(stdout, "#\tendTime    = %9.2f\n", endTime);  
    fprintf(stdout, "#\tspeedMean  = %9.4f\n", speedMean);  
    fprintf(stdout, "#\tspeedDelta = %9.4f\n", speedDelta);  
    fprintf(stdout, "#\tpauseMean  = %9.2f\n", pauseMean);  
    fprintf(stdout, "#\tpauseDelta = %9.2f\n", pauseDelta);  
    fprintf(stdout, "#\toutput     = %6c\n", output);  
    fprintf(stdout, "#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");  
  } else {  
    fprintf(stdout, "Usage:  mobgen-ss <number of nodes>\n");  
    fprintf(stdout, "               <max-x> <max-y> <end time>\n");  
    fprintf(stdout, "               <speed mean> <speed delta>\n");  
    fprintf(stdout, "               <pause time> <pause time delta>\n");  
    fprintf(stdout, "               <'N' or 'G' or 'Q'>\n");  
    fprintf(stdout, "               'N' implies NS2 mobility file\n");  
    fprintf(stdout, "               'G' implies gnuplot path file\n");  
    fprintf(stdout, "               'Q' implies QualNet mobility file\n");  
    return -1;  
  }  
  
  if (output == 'N')  
  {  
    fprintf(stdout, "# output format is NS2\n");  
  } else if (output == 'G') {  
    fprintf(stdout, "# output format is gnuplot\n");  
    if (numNodes != 1)  
    {  
      fprintf(stderr,  
              "Gnuplot output is only possible with one mobile node.\n");  
      return -1;  
    } else {  
      fprintf(stdout, "plot \'-\' notitle with linespoints\n");  
    }  
  } else if (output == 'Q') {  
    fprintf(stdout, "# output format is QualNet\n");  
  } else {  
    fprintf(stderr, "Unknown output type requested\n");  
    return -1;  
  }  
  
  #ifdef rand48  
    srand48((int)time(NULL));  
  #else  
    srand((int)time(NULL));  
  #endif  
  speedLow = speedMean - speedDelta;  
  pauseLow = pauseMean - pauseDelta;  
  speedRange = 2*speedDelta;  
  pauseRange = 2*pauseDelta;  
  
  //calculate the steady-state probability that a node is initially paused  
  expectedPauseTime=pauseMean;  
  a=maxX;  
  b=maxY;  
  v0=speedMean - speedDelta;  
  v1=speedMean + speedDelta;  
  log1=b*b/a*log(sqrt((a*a)/(b*b) + 1) + a/b);  
  log2=a*a/b*log(sqrt((b*b)/(a*a) + 1) + b/a);  
  expectedTravelTime=1.0/6.0*(log1 + log2);  
  expectedTravelTime+=1.0/15.0*((a*a*a)/(b*b) + (b*b*b)/(a*a)) -   
                      1.0/15.0*sqrt(a*a + b*b)*((a*a)/(b*b) + (b*b)/(a*a) - 3);  
  if(speedDelta==0.0)  
    expectedTravelTime/=speedMean;  
  else  
    expectedTravelTime*=log(v1/v0)/(v1 - v0);  
  probabilityPaused=expectedPauseTime/(expectedPauseTime + expectedTravelTime);  
  
  //fprintf(stderr,"Speed Range: (%f, %f)\nPause Time Range: (%f, %f)\nNetwork Dimensions: (%f, %f)\nexpectedTravelTime: %f\nexpectedPauseTime: %f\n\nInitial Values:\nSpeed: X-Location: Y-Location: Pause Time:\n",speedLow,speedLow+speedRange,pauseLow,pauseLow+pauseRange,maxX,maxY,expectedTravelTime,expectedPauseTime);  
  //these are used for steady-state initial pause times  
  t1=pauseLow;  
  t2=pauseLow + pauseRange;  
  
  moving = (int*)malloc(sizeof(int)*numNodes);  
  nextEvent = (double*)malloc(sizeof(double)*numNodes);  
  xLoc = (double*)malloc(sizeof(double)*numNodes);  
  yLoc = (double*)malloc(sizeof(double)*numNodes);  
  
  fprintf(stdout, "#\tInitial positions:\n");  
  for (i=0; i<numNodes; i++)  
  {  
    //steady-state initial positions  
    r=0;  
    u1=1;  
    while(u1>=r)  
    {  
      #ifdef rand48  
        x1=drand48()*maxX;  
        x2=drand48()*maxX;  
        y1=drand48()*maxY;  
        y2=drand48()*maxY;  
        u1=drand48();  
      #else  
        x1=((double)rand()/(double)RAND_MAX)*maxX;  
        x2=((double)rand()/(double)RAND_MAX)*maxX;  
        y1=((double)rand()/(double)RAND_MAX)*maxY;  
        y2=((double)rand()/(double)RAND_MAX)*maxY;  
        u1=((double)rand()/(double)RAND_MAX);  
      #endif  
      //r is a ratio of the length of the randomly chosen path over  
      //the length of a diagonal across the simulation area  
      r=sqrt(((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))/  
             (maxX*maxX + maxY*maxY));  
      //u1 is a uniform random number between 0 and 1  
    }  
    //initially the node has travelled a proportion u2 of the path from  
    //(x1,y1) to (x2,y2)  
    u2=((double)rand()/(double)RAND_MAX);  
    x=u2*x1 + (1 - u2)*x2;  
    y=u2*y1 + (1 - u2)*y2;  
    xLoc[i]=x;  
    yLoc[i]=y;  
  
    //steady-state initial speeds  
    #ifdef rand48  
      u=drand48();  
    #else  
      u=(double)rand()/(double)RAND_MAX;  
    #endif  
    if(u<probabilityPaused) //node initially paused  
    {  
      //calculate initial node pause time  
      #ifdef rand48  
        u = drand48();  
      #else  
        u = ((double)rand()/(double)RAND_MAX);  
      #endif  
      if(pauseDelta!=0.0)  
      {  
        if(u < (2*t1/(t1+t2)) )  
        {  
          pauseTime=u*(t1+t2)/2;  
          //fprintf(stdout, "# Case 1 u: %f ", u);  
        }  
        else  
        {  
          // there is an error in equation 20 in the Tech. Report MCS-03-04  
          // this error is corrected in the TMC 2004 paper and below  
          pauseTime=t2-sqrt((1-u)*(t2*t2 - t1*t1));  
          //fprintf(stdout, "# Case 2 u: %f ", u);  
        }  
      }  
      else  
        pauseTime=u*pauseMean;  
      //fprintf(stdout,"# Initial Pause Time: %f\n",pauseTime);  
      moving[i]=false;  
      s=0.0;  
    }  
    else //node initially moving  
    {  
      pauseTime=0.0;  
      //calculate initial node speed  
      v0=speedLow;  
      v1=speedLow + speedRange;  
      #ifdef rand48  
        u=drand48();  
      #else  
        u=((double)rand()/(double)RAND_MAX);  
      #endif  
      s=pow(v1,u)/pow(v0,u - 1);  
      moving[i]=true;  
      //fprintf(stdout, "# MOVING\n");  
    }  
  
    //pauseTime2 is the time paused or the time to reach the next waypoint  
    dist = sqrt((x2 - x)*(x2 - x)+(y2 - y)*(y2 - y));  
    if(moving[i]) pauseTime2=dist/s;  
    else pauseTime2=pauseTime;  
  
    //fprintf(stderr, "%f %f %f %f\n", s, xLoc[i], yLoc[i], pauseTime);  
  
    nextEvent[i]=0.0; //steady-state  
    if (output == 'N')  
    {  
      fprintf(stdout, "$node_(%d) set X_ %.12f\n", i, xLoc[i]);  
      fprintf(stdout, "$node_(%d) set Y_ %.12f\n", i, yLoc[i]);  
      fprintf(stdout, "$node_(%d) set Z_ %.12f\n", i, 0.0);  
      //steady-state  
      if(endTime!=0.0)  
        fprintf(stdout, "$ns_ at %.12f \"$node_(%d) setdest %.12f %.12f %.12f\"\n",  
                      nextEvent[i], i, x2, y2, s);        
    } else if (output == 'G') {  
      //steady-state  
      fprintf(stdout, "%.12f %.12f # node %d at %.10f speed=%.10f\n", x2, y2, i, nextEvent[i], s);  
    } else if (output == 'Q') {  
      fprintf(stdout, "%d 0S (%.12f, %.12f, %.12f)\n", i+1, xLoc[i], yLoc[i], 0.0);  
      if(!moving[i])  
      {  
        x2=x;  
        y2=y;  
      }  
      fprintf(stdout, "%d %.12fS (%.12f, %.12f, %.12f)\n", i+1, nextEvent[i]+pauseTime2, x2, y2, 0.0);  
    }  
  
    if(moving[i]) //node initially moving  
    {  
      //steady-state  
      xLoc[i] = x2;  
      yLoc[i] = y2;  
      nextEvent[i]+=dist/s;  
    }  
    else //node initially paused  
      nextEvent[i]+=pauseTime;  
  }  
  
  if ((output == 'N')||(output == 'Q'))  
  {  
    if(endTime!=0.0)  
      fprintf(stdout, "\n\n#\tMovements:\n");  
  }  
  
  lowest = endTime;  /* initialize high so all starting movements are 
                        scheduled and output */  
  if(endTime==0.0) lowest=endTime+1.0;  
  while (lowest <= endTime)  
  {  
    /* schedule the next event */  
    if (moving[nextNode])  
    {  
      moving[nextNode] = false;  
      speed = 0.0;  
  
      // I had to do the following calculation before output for QualNet -NBB  
      #ifdef rand48  
        pauseTime2 = drand48()*pauseRange + pauseLow;  
      #else  
        pauseTime2 = ((double)rand()/(double)RAND_MAX)*pauseRange + pauseLow;  
      #endif  
  
      if (output == 'N')  
      {  
        fprintf(stdout,   
                "$ns_ at %.12f \"$node_(%d) setdest %.12f %.12f %.12f\"\n",  
                nextEvent[nextNode], nextNode, xLoc[nextNode], yLoc[nextNode],  
                speed);  
      } else if (output == 'Q') {  
        fprintf(stdout, "%d %.12fS (%.12f, %.12f, %.12f)\n", nextNode+1,   
                nextEvent[nextNode]+pauseTime2, xLoc[nextNode], yLoc[nextNode],  
                0.0);  
      }  
      nextEvent[nextNode]+=pauseTime2;  
  
    } else /* not moving */ {  
      moving[nextNode] = true;  
      #ifdef rand48  
        speed = drand48()*speedRange + speedLow;  
        newX = drand48()*maxX;  
        newY = drand48()*maxY;  
      #else  
        speed = ((double)rand()/(double)RAND_MAX)*speedRange + speedLow;  
        newX = ((double)rand()/(double)RAND_MAX)*maxX;  
        newY = ((double)rand()/(double)RAND_MAX)*maxY;  
      #endif  
  
      //I had to move the next line in front of the QualNet output -NBB  
      dist = sqrt((newX - xLoc[nextNode])*(newX - xLoc[nextNode]) +   
                  (newY - yLoc[nextNode])*(newY - yLoc[nextNode]));  
  
      if (output == 'N')  
      {  
        fprintf(stdout,   
                "$ns_ at %.12f \"$node_(%d) setdest %.12f %.12f %.12f\"\n",  
                nextEvent[nextNode], nextNode, newX, newY, speed);  
      } else if (output == 'G') {  
        fprintf(stdout, "%.12f %.12f # node %d at %.10f speed=%.10f\n", newX,   
                newY, nextNode, nextEvent[nextNode], speed);  
      } else if (output == 'Q') {  
        fprintf(stdout, "%d %.12fS (%.12f, %.12f, %.12f)\n", nextNode+1,   
                nextEvent[nextNode]+dist/speed, newX, newY, 0.0);  
      }  
  
      xLoc[nextNode] = newX;  
      yLoc[nextNode] = newY;  
      nextEvent[nextNode] += dist/speed;  
    } /* if (moving[nextNode]) */  
  
    /* find new lowest */  
    lowest = endTime + 1.0;  
    for (i=0; i<numNodes; i++)  
    {  
      if (nextEvent[i] <= lowest)  
      {  
        lowest = nextEvent[i];  
        nextNode = i;  
      } /* if (nextEvent[i] <= lowest) */  
    } /* for (i=0; i<numNodes; i++) */  
  } /* while (lowest <= endTime) */  
  
  if ((output == 'N')||(output == 'Q'))  
  {  
    fprintf(stdout, "\n\n\n");  
  } else if (output == 'G') {  
    fprintf(stdout, "e\n\n");  
  }  
  
  free(nextEvent);  
  free(xLoc);  
  free(yLoc);  
  
  return 0;  
}  