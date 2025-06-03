//---------------------------------------------------------------------------------------------------------
// Purpose:
// Program to listen to dump1090-fa output port 30003 using ncat then reads that data in via pipe.
// Reads a file of hex idents for planes that are of interest,
// Records when any one plane was first seen, then outputs a record to a log file when
// the plane goes out of range.
// When the plane has departed the area, message also sent via pushover.net
// Wanted plane details (hex ident and description) are held in a linklist.
// When the plane departs, it's linklist entry is zero'd out.
//
// Run as a system service:
// [Unit]
// Description=Local plane spotting  service
// After=mariadb.service
// Requires=network.target
// [Service]
// Type=idle
// ExecStart=/usr/local/sbin/spotplanes.sh 
// Restart=always
// RestartSec=60
// [Install]
// WantedBy=multi-user.target
// 
// and this is the shell script:
// #!/bin/bash
// /usr/bin/ncat 127.0.0.1 30003 | /usr/local/bin/planespot -i /usr/local/share/PlaneIdents.txt -o /var/log/planes.log
//
// This is typical format/content of the -i (ident) file:
//
// 43C5EE Airbus A400M Atlas C1
// 407685 Airbus EC-145 (Helicopter)
// 43C5DE Airbus A-400M 
// 43C5E1 Airbus A-400M 
// 43C6F8 Airbus Voyager KC3 
// 43C6B8 Boeing C-17 Globemaster
// 43C5DB Airbus A400M Atlas
// 43C5DF Airbus A-400M 
// 4076C3 AUTOGYRO Cavalon 
// 43C92C AIRBUS HELICOPTERS EC-145 
// 406D67 Alisport Silent 
// 43C8F6 AIRBUS HELICOPTERS EC-135/635 
// 407279 ROBINSON R-44 Raven
// 43C174 C-17 Globemaster 3
//
// dump1090-fa port 30003 data format:
//
//	Field 1: Message type 	 (MSG, STA, ID, AIR, SEL or CLK)
//	Field 2: Transmission Type 	 MSG sub types 1 to 8. Not used by other message types.
//	Field 3: Session ID 	 Database Session record number
//	Field 4: AircraftID 	 Database Aircraft record number
//	Field 5: HexIdent 	 Aircraft Mode S hexadecimal code
//	Field 6: FlightID 	 Database Flight record number
//	Field 7: Date message generated 	  As it says
//	Field 8: Time message generated 	  As it says
//	Field 9: Date message logged 	  As it says
//	Field 10: Time message logged 	  As it says
//	Field 11: Callsign 	 An eight digit flight ID - can be flight number or registration (or even nothing).
//	Field 12: Altitude 	 Mode C altitude. Height relative to 1013.2mb (Flight Level). Not height AMSL..
//	Field 13: GroundSpeed 	 Speed over ground (not indicated airspeed)
//	Field 14: Track 	 Track of aircraft (not heading). Derived from the velocity E/W and velocity N/S
//	Field 15: Latitude 	 North and East positive. South and West negative.
//	Field 16: Longitude 	 North and East positive. South and West negative.
//	Field 17: VerticalRate 	 64ft resolution
//	Field 18: Squawk 	 Assigned Mode A squawk code.
//	Field 19: Alert (Squawk change) 	 Flag to indicate squawk has changed.
//	Field 20: Emergency 	 Flag to indicate emergency code has been set
//	Field 21: SPI (Ident) 	 Flag to indicate transponder Ident has been activated.
//	Field 22: IsOnGround 	 Flag to indicate ground squat switch is active
//
// EG:
//	         1         2         3         4         5         6         7        8         9
//	1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
//	MSG,5,1,1,424C62,1,2025/05/19,09:24:49.124,2025/05/19,09:24:49.139,MBIZZ   ,30425,,,,,,,0,,0,
//	MSG,6,1,1,A913D6,1,2025/05/19,09:24:49.200,2025/05/19,09:24:49.246,UAL74   ,,,,,,,5244,0,0,0,
//	MSG,5,1,1,3C4592,1,2025/06/02,12:39:47.937,2025/06/02,12:39:47.954,BOX451  ,37000,,,,,,,0,,0,
//---------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#define MAX_FILENAME_LEN 100
#define MAX_NOTES_LEN 60
#define PLANE_MAX_AGE 3
#define OUTPUT_RECORD_LENGTH 200
#define PUSHOVER_RECORD_LENGTH 300

struct plane {
  char callsign[9];
  char hexident[7];
  int firstseen_year;
  int firstseen_month;
  int firstseen_day;
  int firstseen_hour;
  int firstseen_min;
  int firstseen_sec;
  int lastseen_year;
  int lastseen_month;
  int lastseen_day;
  int lastseen_hour;
  int lastseen_min;
  int lastseen_sec;
  char Notes[MAX_NOTES_LEN];
  struct plane *previous;
  struct plane *next;
 };

struct plane *head=NULL;
struct plane* load_plane_linklist(char *);
struct plane* FindWantedPlane(char *);

int main(int argc, char *argv[]) {
	int k,age,opt,aircraft_age, tmins,amins;
	size_t buflen;
	char buffer[500];
	char Year[5],Month[3],Day[3],Hour[3],Min[3],Sec[3],HexIdent[7];
	time_t nowtime;
	struct tm *timeinfo;
	struct plane *WantedPlane=NULL;
	struct plane *planerec=NULL;
	char infile[MAX_FILENAME_LEN+1];
	char outfile[MAX_FILENAME_LEN+1];
	char output_record[OUTPUT_RECORD_LENGTH];
	char pushover_cmd[PUSHOVER_RECORD_LENGTH];
	FILE *ofile;
	FILE *pushover;

	//-----------------------------------------------------------------------
  //
	//	Process command line. argc=5, -i <filename> -o <filename>
  //
	//-----------------------------------------------------------------------
  if(argc!=5) {
    fprintf(stderr,"%s: Arguments shoud be -i <filename>\n",argv[0]);
    exit(0);
  }
  while((opt = getopt(argc,argv, "i:o:")) != -1) {
    switch(opt) {
      case 'i': {
          if(strlen(optarg)>=MAX_FILENAME_LEN) exit(0);
          else strncpy(infile,optarg,MAX_FILENAME_LEN);
          break;
      }
			case 'o': {
				if(strlen(optarg)>=MAX_FILENAME_LEN) exit(0);
				else strncpy(outfile,optarg,MAX_FILENAME_LEN);
				break;
		}
      default : {
        printf("Option %c is invalid\n",opt);
        exit(0);
      }
    }
  }

	//-----------------------------------------------------------------------
	//	Finished command line parse. Load wanted plane linklist from -i file
	//	and populate time struct.
	//-----------------------------------------------------------------------
	head = load_plane_linklist(infile);
  nowtime=time(NULL);
  timeinfo = localtime(&nowtime);
	age=timeinfo->tm_min;

	//-----------------------------------------------------------------------
	// Now start reading stdout from netcat
	//-----------------------------------------------------------------------

	while((bool)1) {
		(void) fgets(buffer, 500, stdin);
		buflen=strlen(buffer);
		buffer[buflen-1]= (char) 0x0;
		memcpy(HexIdent,buffer+10,6); HexIdent[6]= (char) 0x0;
		WantedPlane=FindWantedPlane(HexIdent);
		if(WantedPlane) {
			//-----------------------------------------------------------------------
			//	See if the netcat record contains a callsign value. If it
			//	does, then copy it in.
			//-----------------------------------------------------------------------
			if(buffer[67] !=',') {
					memcpy(WantedPlane->callsign,buffer+67,8); 
					WantedPlane->callsign[8] = (char) 0x0;
			}
			memcpy(Year,buffer+19,4); Year[4]= (char) 0x0;
			memcpy(Month,buffer+24,2); Month[2]= (char) 0x0;
			memcpy(Day,buffer+27,2); Day[2]= (char) 0x0;
			memcpy(Hour,buffer+30,2); Hour[2]= (char) 0x0;
			memcpy(Min,buffer+33,2); Min[2]= (char) 0x0;
			memcpy(Sec,buffer+36,2); Sec[2]= (char) 0x0;
      //-----------------------------------------------------------------------
      // If first seen year is zero, this is a new sighting
      //-----------------------------------------------------------------------
      if(WantedPlane->firstseen_year == 0) {
        WantedPlane->firstseen_year = atoi(Year);
        WantedPlane->firstseen_month = atoi(Month);
        WantedPlane->firstseen_day = atoi(Day);
        WantedPlane->firstseen_hour = atoi(Hour);
        WantedPlane->firstseen_min = atoi(Min);
        WantedPlane->firstseen_sec = atoi(Sec);
				WantedPlane->lastseen_year = atoi(Year);
				WantedPlane->lastseen_month = atoi(Month);
				WantedPlane->lastseen_day = atoi(Day);
				WantedPlane->lastseen_hour = atoi(Hour);
				WantedPlane->lastseen_min = atoi(Min);
				WantedPlane->lastseen_sec = atoi(Sec);
				ofile = fopen(outfile,"a");
        fprintf(ofile,"Spotted: %s at %04d/%02d/%02d %02d:%02d:%02d %s\n",
          WantedPlane->hexident,
          WantedPlane->lastseen_year,WantedPlane->lastseen_month,WantedPlane->lastseen_day,
          WantedPlane->lastseen_hour,WantedPlane->lastseen_min,WantedPlane->lastseen_sec,WantedPlane->Notes);
				fclose(ofile);
      }
			else {
        WantedPlane->lastseen_year = atoi(Year);
        WantedPlane->lastseen_month = atoi(Month);
        WantedPlane->lastseen_day = atoi(Day);
        WantedPlane->lastseen_hour = atoi(Hour);
        WantedPlane->lastseen_min = atoi(Min);
        WantedPlane->lastseen_sec = atoi(Sec);
			  //ofile = fopen(outfile,"a");
				//fprintf(ofile,"Update: \"%s\" %s %04d/%02d/%02d %02d:%02d:%02d at %04d/%02d/%02d %02d:%02d:%02d %s\n",
				//	WantedPlane->callsign,WantedPlane->hexident,
				//	WantedPlane->firstseen_year,WantedPlane->firstseen_month,WantedPlane->firstseen_day,
				//	WantedPlane->firstseen_hour,WantedPlane->firstseen_min,WantedPlane->firstseen_sec,
				//	WantedPlane->lastseen_year,WantedPlane->lastseen_month,WantedPlane->lastseen_day,
				//	WantedPlane->lastseen_hour,WantedPlane->lastseen_min,WantedPlane->lastseen_sec,WantedPlane->Notes);
				//fclose(ofile);
			}
		}

		//-----------------------------------------------------------------------
		//	If the plan hasn't been seen for PLANE_MAX_AGE mins, then output it
		//	then zero out the record. 
		//-----------------------------------------------------------------------
    nowtime=time(NULL);
    timeinfo = localtime(&nowtime);
		//-----------------------------------------------------------------------
		//	Only run once for each minute tick
		//-----------------------------------------------------------------------

		if(timeinfo->tm_min !=age) {
			age=timeinfo->tm_min;
			planerec = head;
			while(true) {
				if(planerec->firstseen_year !=0) {
					if(planerec->lastseen_min > timeinfo->tm_min) aircraft_age= (60 - planerec->lastseen_min)+timeinfo->tm_min;
					else aircraft_age = timeinfo->tm_min - planerec->lastseen_min;
					if(aircraft_age >=PLANE_MAX_AGE) {
						ofile = fopen(outfile,"a");
						snprintf(output_record,OUTPUT_RECORD_LENGTH,"Flight: %s %s Seen %04d/%02d/%02d,%02d:%02d:%02d Lost %04d/%02d/%02d,%02d:%02d:%02d (%s)",
							planerec->hexident,planerec->callsign,
              planerec->firstseen_year,planerec->firstseen_month,planerec->firstseen_day,
              planerec->firstseen_hour,planerec->firstseen_min,planerec->firstseen_sec,
              planerec->lastseen_year,planerec->lastseen_month,planerec->lastseen_day,
              planerec->lastseen_hour,planerec->lastseen_min,planerec->lastseen_sec,
              planerec->Notes);
						fprintf(ofile,"%s\n",output_record);
						fclose(ofile);
						snprintf(pushover_cmd,PUSHOVER_RECORD_LENGTH,"curl -s \
								-F \"token=xxxxxxxxxxxxxxxxxxxxxxxxxxxxx\" \
								-F \"user=yyyyyyyyyyyyyyyyyyyyyyyyyyyyy\" \
								-F  \"message=%s\" \
								https://api.pushover.net/1/messages.json",output_record);
						pushover = popen(pushover_cmd,"r");
						pclose(pushover);
						planerec->firstseen_year=0;
					}
				}
				if(planerec->next == NULL) break;
				planerec=planerec->next;
			}
		}
	}
}


struct plane* load_plane_linklist(char *infile) {
	int k,l,ifile_line_len;
	char ifile_line[150];
	FILE *ifile;
	struct plane *planerec;
	struct plane *tail;
  if(!(bool) (ifile = fopen(infile,"r"))) {
    fprintf(stderr,"Could not open %s for reading\n",infile);
    return NULL;
  }

	//-----------------------------------------------------------------------
  //
  // Populate the linklist with wanted plane details
  //
	//-----------------------------------------------------------------------
  while(fgets(ifile_line,80,ifile)) {
    ifile_line[strlen(ifile_line)-1] = (char) 0x0;
    ifile_line_len=strlen(ifile_line);

    if(!(bool) (planerec = malloc(sizeof(struct plane)))) {
      fprintf(stderr,"Malloc of link list head failed\n");
      return NULL;
    }
    l=0;
    for(k=0;k<ifile_line_len;k++) {
      if(ifile_line[k]==' ') break;
      planerec->hexident[l++]=ifile_line[k];
    }
    planerec->hexident[l]=0x0;

    while((ifile_line[k]== ' ') && (k<ifile_line_len)) k++;

    l=0;
    for(;k<ifile_line_len;k++) planerec->Notes[l++]=ifile_line[k];
    planerec->Notes[l]=0x0;
		planerec->firstseen_year = 0; planerec->firstseen_month = 0; planerec->firstseen_day = 0;
		planerec->firstseen_hour = 0; planerec->firstseen_min = 0; planerec->firstseen_sec = 0;
		planerec->lastseen_year = 0; planerec->lastseen_month = 0; planerec->lastseen_day = 0;
		planerec->lastseen_hour = 0; planerec->lastseen_min = 0; planerec->lastseen_sec = 0;
		//printf("Added: %s,%s\n",planerec->hexident,planerec->Notes);
		//-----------------------------------------------------------------------
    //
    //Finished extracting data from the line. Now add fields to linklist
    //
		//-----------------------------------------------------------------------

    if(head == NULL) {
      head = planerec;
      planerec->previous = NULL;
      planerec->next = NULL;
    }
    else {
      planerec->previous = tail;
      tail->next = planerec;
      planerec->next=NULL;
    }
    tail = planerec;
  }
  fclose(ifile);
  return head;
}

struct plane * FindWantedPlane(char *hexident) {
	struct plane *planerec;
	planerec = head;
	while(true) {
		if(!(bool) memcmp(planerec->hexident, hexident,6)) return planerec;
		if(planerec->next == NULL) return NULL;
		planerec = planerec->next;
	}
}
