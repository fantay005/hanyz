#ifndef __M35FTP_H__
#define __M35FTP_H__

#include <stdbool.h>

bool m35FtpConnect(const char *remoteFile, const char *host, int port, int timeoutMs);
bool m35FtpDownload(void);
bool m35FtpClose(void);

#endif
