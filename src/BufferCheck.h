#ifndef BUFFERCHECK_H
#define BUFFERCHECK_H

bool PacketBufferCheck(char *b, int len);

void ErrorLog(char *message, ...);
void PlayerLog(char *message, ...);

#endif