// Function load2dArray
//
//  Given char ary[5][100];
//  load_array(100,5,ary[0],"Temperature,Humidity,Pressure,WindSpeed,Rainfall");
//   Each array[n] gets comma delimited value
//-----------------------------------------------------------------------------------
void load_array(int len, int fieldcount,char *ary ,char* fields) {
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
