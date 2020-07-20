//
// Return the number of second since midnight. 
// Offset for +BST Etc....
//
int SecondsPastMidnight(int offset) {
time_t now;
struct tm *info;
int secs;
        now=time(NULL);
        info=gmtime(&now);
        secs= (3600*info->tm_hour)+(60*info->tm_min)+info->tm_sec;
        secs+=(offset*3600);
        return secs;
}
