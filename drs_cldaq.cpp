/********************************************************************\

  Name:         drs_exam.cpp
  Created by:   Stefan Ritt

  Contents:     Simple example application to read out a DRS4
                evaluation board

  $Id: drs_exam.cpp 21308 2014-04-11 14:50:16Z ritt $

\********************************************************************/

#include <math.h>

#ifdef _MSC_VER

#include <windows.h>

#elif defined(OS_LINUX)

#define O_BINARY 0

#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DIR_SEPARATOR '/'

#endif

#include <sstream>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <stdlib.h>

#include "strlcpy.h"
#include "DRS.h"

#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>

#include "movestage.cc"


using namespace std;

/*------------------------------------------------------------------*/

int main(int argc, char **argv)
{
   unsigned int N_evts = atoi(argv[1]);
   string filename = argv[4]; //"/home/lab-cpt03/LabData/Linus/180508/waveform_2pulses_0.dat";
   vector<int> active_channels = {1, 2, 3};
   double trigger_level = stod(argv[2]);
   double delay = stod(argv[3]);
   //unsigned int len = strlen(filename);

   //const char fn = *filename;

   unsigned int scans = atoi(argv[5]);
   int MR = atoi(argv[6]);

  // char file[50];

   int i, nBoards;
   DRS *drs;
   DRSBoard *b;
   FILE  *f;


   /* do initial scan */
   drs = new DRS();

   cout << "Looking for the board" << endl;
   for (i=0 ; i<drs->GetNumberOfBoards() ; i++) {
      b = drs->GetBoard(i);
      printf("Found DRS4 evaluation board, serial #%d, firmware revision %d\n",
         b->GetBoardSerialNumber(), b->GetFirmwareVersion());
   }

   /* exit if no board found */
   nBoards = drs->GetNumberOfBoards();
   if (nBoards == 0) {
      printf("No DRS4 evaluation board found\n");
      return 0;
   }

   /* continue working with first board only */
   b = drs->GetBoard(0);

   cout << "Initializing board" << endl;
   b->Init();

   /* set sampling frequency */
   b->SetFrequency(5, true);

   /* enable transparent mode needed for analog trigger */
   b->SetTranspMode(1);

   /* set input range to -0.5V ... +0.5V */
   b->SetInputRange(0);

   /* use following line to set range to 0..1V */
   //b->SetInputRange(0.5);

   /* use following line to turn on the internal 100 MHz clock connected to all channels  */
   //b->EnableTcal(1);

   /* use following lines to enable hardware trigger on CH1 at 50 mV positive edge */
   cout << "Board type: " << b->GetBoardType() << endl;

   //b->EnableTrigger(0, 1);         // analog trigger, lemo
   b->EnableTrigger(1, 0);           // enable hardware trigger
   b->SetTriggerSource(1<<0);           // use CH1 as source

   b->SetTriggerLevel(trigger_level);            // 0.05 V
   // b->SetTriggerPolarity(false);        // positive edge

   /* use following lines to set individual trigger elvels */
   //b->SetIndividualTriggerLevel(1, 0.1);
   //b->SetIndividualTriggerLevel(2, 0.2);
   //b->SetIndividualTriggerLevel(3, 0.3);
   //b->SetIndividualTriggerLevel(4, 0.4);
   //b->SetTriggerSource(15);

   b->SetTriggerDelayNs(delay);             // zero ns trigger delay

   /* use following lines to enable the external trigger */
   //if (b->GetBoardType() == 8) {     // Evaluaiton Board V4
   //   b->EnableTrigger(1, 0);           // enable hardware trigger
   //   b->SetTriggerSource(1<<4);        // set external trigger as source
   //} else {                          // Evaluation Board V3
   //   b->EnableTrigger(1, 0);           // lemo on, analog trigger off
   // }

   /* open USB to control motor */



   for (unsigned int k=0 ; k<scans ; k++) {
/*
     string s = to_string(k);

     int sizeOf = s.size();
     int i = 0;


     while(i<sizeOf)
     {
        filename[i+len]=s[i];

        i++;
     }
*/
     //sprintf(filename, "%s%d.dat",filename, k);
/*
     stringstream strs;
     strs << k;
     string temp_str = strs.str();
     char* char_type = (char*) temp_str.c_str();
     filename[(len-5)] = *char_type;
*/
     //printf("%s \n", filename
     string aux_fname = filename + to_string(k) + ".dat";

     cout << aux_fname.c_str() << endl;
     /* open file to save waveforms */
     f = fopen(aux_fname.c_str(), "w");
     if (f == NULL) {
        perror("ERROR: Cannot open file \"data.txt\"");
        return 1;
     }
     fprintf(f, "B#%04d", b->GetBoardSerialNumber());
     fprintf(f, "CMASK");
     char buffer = 0x0;
     for(auto c : active_channels) {
       buffer += (int)(pow(2,c-1));
     }
     fwrite(&buffer, sizeof(char), 1 , f);
     fprintf(f, "NE");
     fwrite(&N_evts, sizeof(unsigned int), 1, f);

     float time_array[8][1024];
     float wave_array[8][1024];



       for (unsigned int j=0 ; j<N_evts ; j++) {

          /* start board (activate domino wave) */
          b->StartDomino();
          //printf("Waiting for trigger...");
          //fflush(stdout);
          while (b->IsBusy());

          /* read all waveforms */
          b->TransferWaves(0, 8);
          fprintf(f, "Evt");
          fwrite(&j, sizeof(unsigned int), 1, f);

          for(auto c : active_channels) {
            b->GetTime(0, 2*(c-1), b->GetTriggerCell(0), time_array[c-1]);
            fwrite(time_array[c-1], sizeof(float), 1024, f);
            b->GetWave(0, 2*(c-1), wave_array[c-1]);
            fwrite(wave_array[c-1], sizeof(float), 1024, f);
          }

          /* print some progress indication */
          if(j%500==0) {cout << "Evt " << j << endl;}

          // cout << "Event " << j << " read successfully" << endl;
          //  printf("\rEvent #%d read successfully\n", j);
       }


       movestage(MR);

       fclose(f);

     }



   /* delete DRS object -> close USB connection */
   delete drs;
}
