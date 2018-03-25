#ifndef BUFFERCHECK_H
#define BUFFERCHECK_H

bool PacketBufferCheck(char *b, int len);

void ErrorLog(const std::string& message, ...);
void PlayerLog(const std::string& message, ...);

#endif