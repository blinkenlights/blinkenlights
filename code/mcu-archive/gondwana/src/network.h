#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int                   setup_socket   (struct sockaddr_in *addr);
const unsigned char * get_frame      (int                 socket, 
                                      struct sockaddr_in *addr);
const unsigned char * get_test_frame (void);


#endif /* _NETWORK_H_ */
