//---------------------------------------------------------
//
// Function dtgStr()
// Returns a string containing current date time in the
// format YYYYMMDDHHMMSS
//
//---------------------------------------------------------
char * dtgStr() {
	static char text[20];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(text, sizeof(text)-1, "%Y%m%d%H%M%S", t);
	return text;
}
