#ifndef TIME_H
#define TIME_H

struct time {
	// ss mm hh
	unsigned char s:6;
	unsigned char m:6; // 6 bits for 64 mins
	unsigned char h:5; // 5 bits for 32 hours
};

#endif