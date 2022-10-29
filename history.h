#ifndef history_h
#define history_h

char* path;

int SaveLine(char* line);
int ReadLines(int count_l);
char* GetHistoryLine(int count_l);
#endif 
