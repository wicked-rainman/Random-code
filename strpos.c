//----------------------------------------------------
// strpos
// Looks for (string) needle in string (haystack) and
// returns the start position of needle in haystack or
// -1 if not found.
//----------------------------------------------------
int strpos(char *needle, char *haystack) {
char *offset;
        offset=strstr(haystack,needle);
        if(offset !=NULL) {
                return offset-haystack;
        }
        return -1;
}
