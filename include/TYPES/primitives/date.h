#ifndef DATE_H
#define DATE_H

struct date {
	// dd mm yy
	unsigned char day:5;   // 5 bits for 32 days
	unsigned char month:4; // 4 bits for 16 months 
	unsigned char year:7;  // 7 bits for 128 years {will be added to 2000}
}; // 16 bits

#endif