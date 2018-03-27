#ifndef HELPERS_H
#define HELPERS_H

int char_to_decimal(char letter);
int hex_to_decimal_4bit(char seq[4]);
int hex_to_decimal_time(char seq[4]);
int stream_parser(char raw[BUFF_MAX]);
unsigned int BLE_parse(const char *inFile, int mode);
void makeCSV(unsigned int size);
void cleanup();

#endif
