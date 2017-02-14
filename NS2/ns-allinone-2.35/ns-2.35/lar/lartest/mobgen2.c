/****************************************************************************** 
*   File Name: mobgen.c 
*   Purpose: Random Waypoint mobility model generator. 
*   Author: Jeff Boleng 
*   Date Created: 
* 
*   Copyright (C) 2004  Toilers Research Group -- Colorado School of Mines 
* 
*   Please see COPYRIGHT.TXT and LICENSE.TXT for copyright and license 
*   details. 
******************************************************************************/   
  
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
  char output;  
  
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
  
    fprintf(stdout, "#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");  
    fprintf(stdout, "#\tnumNodes   = %6d\n", numNodes);  
    fprintf(stdout, "#\tmaxX       = %9.2f\n", maxX);  
    fprintf(stdout, "#\tmaxY       = %9.2f\n", maxY);  
    fprintf(stdout, "#\tendTime    = %9.2f\n", endTime);  
    fprintf(stdout, "#\tspeedMean  = %9.2f\n", speedMean);  
    fprintf(stdout, "#\tspeedDelta = %9.2f\n", speedDelta);  
    fprintf(stdout, "#\tpauseMean  = %9.2f\n", pauseMean);  
    fprintf(stdout, "#\tpauseDelta = %9.2f\n", pauseDelta);  
    fprintf(stdout, "#\toutput     = %6c\n", output);  
    fprintf(stdout, "#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");  
  } else {  
    fprintf(stdout, "Usage:  mobgen <number of nodes>\n");  
    fprintf(stdout, "               <max-x> <max-y> <end time>\n");  
    fprintf(stdout, "               <speed mean> <speed delta>\n");  
    fprintf(stdout, "               <pause time> <pause time delta>\n");  
    fprintf(stdout, "               <'N' or 'G'>\n");  
    fprintf(stdout, "               'N' implies NS2 mobility file\n");  
    fprintf(stdout, "               'G' implies gnuplot path file\n");  
    return -1;  
  }  
  
  if (output == 'N')  
  {  
    fprintf(stdout, "# output format is NS2\n");  
  } else if (output == 'G') {  
    fprintf(stdout, "# output format is gnuplot\n");  
    if (numNodes != 1)  
    {  
      fprintf(stderr, "Gnuplot output is only possible with one mobile node.\n");  
      return -1;  
    } else {  
      fprintf(stdout, "plot \'-\' notitle with linespoints\n");  
    }  
  } else {  
    fprintf(stderr, "Unknown output type requested\n");  
    return -1;  
  }  
  
  #ifdef rand48  
    srand48((int)time(NULL));  
  #else  
    srand((int)time(NULL));  
  #endif  
  speedLow = speedMean-speedDelta;  
  pauseLow = pauseMean-pauseDelta;  
  speedRange = 2*speedDelta;  
  pauseRange = 2*pauseDelta;  
  
  moving = (int*)malloc(sizeof(int)*numNodes);  
  nextEvent = (double*)malloc(sizeof(double)*numNodes);  
  xLoc = (double*)malloc(sizeof(double)*numNodes);  
  yLoc = (double*)malloc(sizeof(double)*numNodes);  
  
  fprintf(stdout, "#\tInitial positions:\n");  
  for (i=0; i<numNodes; i++)  
  {  
    #ifdef rand48  
      moving[i] = (drand48()>=0.5)?true:false;  
      xLoc[i] = drand48()*maxX;  
      yLoc[i] = drand48()*maxY;  
    #else  
      moving[i] = (rand()>=(RAND_MAX/2))?true:false;  
      /* 
      fprintf(stdout, "$node_(%d) is ", i); 
      if (moving[i]) 
      { 
        fprintf(stderr, "moving\n");  
      } else { 
        fprintf(stderr, "not moving\n");  
      } 
      */  
      xLoc[i] = ((double)rand()/(double)RAND_MAX)*maxX;  
      yLoc[i] = ((double)rand()/(double)RAND_MAX)*maxY;  
    #endif  
    /*fprintf(stdout, "%d:  %s moving\n", i, ((moving[i])?"is not":"is"));*/  
    if (output == 'N')  
    {  
      fprintf(stdout, "$node_(%d) set X_ %.12f\n", i, xLoc[i]);  
      fprintf(stdout, "$node_(%d) set Y_ %.12f\n", i, yLoc[i]);  
      fprintf(stdout, "$node_(%d) set Z_ %.12f\n", i, 0.0);  
    } else if (output == 'G') {  
      xLoc[i] = maxX/2.0;  
      yLoc[i] = maxY/2.0;  
      fprintf(stdout, "%.12f %.12f\n", xLoc[i], yLoc[i]);  
    }  
  
    nextEvent[i] = 0.0;  
  }  
  if (output == 'N')  
  {  
    fprintf(stdout, "\n\n#\tMovements:\n");  
  }  
  
  lowest = endTime;  /* initialize high so all starting movements are 
                        scheduled and output */  
  while (lowest <= endTime)  
  {  
    /* schedule the next event */  
    if (moving[nextNode])  
    {  
      moving[nextNode] = false;  
      speed = 0.0;  
      if (output == 'N')  
      {  
        fprintf(stdout, "$ns_ at %.12f \"$node_(%d) setdest %.12f %.12f %.12f\"\n",  
                         nextEvent[nextNode], nextNode, xLoc[nextNode], yLoc[nextNode], speed);  
      }  
      #ifdef rand48  
        nextEvent[nextNode] += drand48()*pauseRange + pauseLow;  
      #else  
        nextEvent[nextNode] += ((double)rand()/(double)RAND_MAX)*pauseRange + pauseLow;  
      #endif  
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
      if (output == 'N')  
      {  
        fprintf(stdout, "$ns_ at %.12f \"$node_(%d) setdest %.12f %.12f %.12f\"\n",  
                         nextEvent[nextNode], nextNode, newX, newY, speed);  
      } else if (output == 'G') {  
        fprintf(stdout, "%.12f %.12f # node %d at %.10f speed=%.10f\n", newX, newY, nextNode, nextEvent[nextNode], speed);  
      }  
  
      dist = sqrt((newX-xLoc[nextNode])*(newX-xLoc[nextNode]) + (newY-yLoc[nextNode])*(newY-yLoc[nextNode]));  
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
  
  if (output == 'N')  
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
