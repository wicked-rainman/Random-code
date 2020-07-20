//-------------------------------------------------------------------------------------
// Function cdl2dArray
//
//  Purpose: Given a comma delimited string, load each value into a 2D array.
//  First parameter is the number of rows in the array
//  Second parameter is the number of columns in each row
//  Third parameter is the address of the array to be loaded
//  Fourth parameter is the comma delimited string.
//
//  Given char ary[5][100];
//  load_array(100,5,ary[0],"Temperature,Humidity,Pressure,WindSpeed,Rainfall");
//  Each array[n] gets comma delimited value "Temperature","Humidity"... Etc
//
// NOTES: Depends on function strpos
//        prototype for this function is: void cdl2dArray(int, int, char*, char*);
//
//-----------------------------------------------------------------------------------
void cdl2Array(int len, int fieldcount,char *ary ,char* fields) {
int k,i;
int startpos=0;
        for(k=0;k<fieldcount-1;k++) {
                i=strpos(",",fields+startpos);
                memset(ary+(k*len),0,len);
                strncpy(ary+(k*len),fields+startpos,i);
                startpos+=(i+1);
        }
        strcpy(ary+(k*len),fields+startpos);
}
