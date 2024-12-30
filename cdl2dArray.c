//-------------------------------------------------------------------------------------
// Function cdl2dArray
//
//  Purpose: Given a comma delimited string, load each value into a 2D array.
//  First parameter is the length of the maximum field length in the array
//  Second parameter is the number of fields in the array
//  Third parameter is the address of the array to be loaded
//  Fourth parameter is the comma delimited input string.
//
//  Given char ary[5][100];
//  cdl2Array(100,5,ary[0],"Temperature,Humidity,Pressure,WindSpeed,Rainfall");
//  Each array[n] gets the comma delimited values out of fields 
//  for "Temperature","Humidity"... Etc
//
// NOTES: prototype for this function is: void cdl2dArray(int, int, char*, char*);
//
//-----------------------------------------------------------------------------------
int cdl2Array(int FieldLen, int FieldCount,char *WordArray ,char* InputLine) {
    int InputLineLength=0;
    int k;
    int ArrayPos=0;
    int WordCount=0;
    memset(WordArray,0,FieldCount*FieldLen);
    InputLineLength=strlen(InputLine);
    for(k=0;k<InputLineLength;k++) {
        if(WordCount>=FieldCount) return -1;
        if(InputLine[k]==',') {
            WordCount++;
            ArrayPos=0;
        }
        else {
            if(ArrayPos>=FieldLen) return -1;
            strncpy(WordArray+((WordCount*FieldLen)+ArrayPos++),InputLine+k,1);
        }
    }
    return WordCount+1;
}
