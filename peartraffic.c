#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dlfcn.h>
#include <string.h>

ssize_t (*real_sendto)(int socket, const void *buffer, size_t length, int flags,
		       const struct sockaddr *dest_addr, socklen_t dest_len);
ssize_t (*real_recvfrom)(int socket, void *buffer, size_t length, int flags,
			 struct sockaddr *address, socklen_t *address_len);



ssize_t (*real_recv)(int fd,void*buf,size_t len,int flags);

ssize_t (*real_send)(int fd,const void *buf,size_t len,int flags);

ssize_t  (*real_sendmsg)(int sockfd, const struct msghdr *msg, int flags);

ssize_t (*real_sendfile)(int out_fd, int in_fd, off_t *offset, size_t count);

ssize_t (*real_write)(int fd, const void *buf, size_t count);
ssize_t (*real_read)(int fd, void *buf, size_t count);

int (*real_sendmmsg)(int sockfd,  struct mmsghdr *msgvec, unsigned int vlen,
             unsigned int flags);

unsigned short old_port, new_port;


// strace ping  pear.hk

void _init(void)
{
        printf("Pear init ... \r\n");
	char *old_port_env;
	if (!(old_port_env = getenv("OLD_PORT"))) {
		fprintf(stderr, "getenv(OLD_PORT) failed\n");
		exit(42);
	}
      
	old_port = htons(atoi(old_port_env));

	char *new_port_env;
	if (!(new_port_env = getenv("NEW_PORT"))) {
		fprintf(stderr, "getenv(NEW_PORT) failed\n");
		exit(42);
	}
	new_port = htons(atoi(new_port_env));

	const char *err;
	real_sendto = dlsym(RTLD_NEXT, "sendto");
	if ((err = dlerror()) != NULL) {
		fprintf(stderr, "dlsym(sendto) failed: %s\n", err);
		exit(42);
	}

	real_recvfrom = dlsym(RTLD_NEXT, "recvfrom");
	if ((err = dlerror()) != NULL) {
		fprintf(stderr, "dlsym(recvfrom): %s\n", err);
		exit(42);
	}
        real_recv=dlsym(RTLD_NEXT, "recv");
        real_send=dlsym(RTLD_NEXT, "send");

        real_sendmsg=dlsym(RTLD_NEXT, "sendmsg");
        real_sendfile=dlsym(RTLD_NEXT, "sendfile");
        real_write=dlsym(RTLD_NEXT,"write");
        real_read=dlsym(RTLD_NEXT,"read");
        real_sendmmsg=dlsym(RTLD_NEXT,"sendmmsg");

        printf("Pear init ... \r\n");
}

// telnet use the system api
#if 0
int sendmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
                    unsigned int flags)
{
  //int  n=real_sendmmsg( sockfd, msgvec, vlen,flags);
  // printf("\r\n sendmmsg %d... buf: %s\r\n",n); 
  return 0;
}
#endif 0

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
   ssize_t n = real_sendmsg(sockfd,msg,flags);
   printf("\r\n sendmsg %d... buf: %s\r\n",n); 
   return n;
}
// Segmentation fault (core dumped)
#if 0
ssize_t read(int fd, void *buf, size_t count)
{
   ssize_t n = real_read(fd,buf,count);
   printf("\r\n read %d\r\n",n);
   return n;
}
#endif

#if 1
ssize_t write(int fd, const void *buf, size_t count)
{
   
   ssize_t n = real_write(fd,buf,count);
   printf("\r\n write %d\r\n",n);
   return n;
}
#endif

ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
   printf("\r\n sendfile\r\n");
   return 0;
}

ssize_t recv(int fd,void*buf,size_t len,int flags)
{
  

   ssize_t n = real_recv(fd,buf,len,flags);
   printf("\r\n recv %d... \r\n",n); 
   return n;
}

ssize_t send(int fd,const void *buf,size_t len,int flags)
{
   
   ssize_t n = real_send(fd,buf,len,flags);
   printf("\r\n send...%d \r\n",n);
   return n;
}


ssize_t sendto(int socket, const void *buffer, size_t length, int flags,
	       const struct sockaddr *dest_addr, socklen_t dest_len)
{
        printf("Pear sendto ... \r\n");
	static struct sockaddr_in *dest_addr_in;
	dest_addr_in = (struct sockaddr_in *)dest_addr;
	if (dest_addr_in->sin_port == old_port) {
		static struct sockaddr_storage new_dest_addr;
		memcpy(&new_dest_addr, dest_addr, dest_len);

		static struct sockaddr_in *new_dest_addr_in;
		new_dest_addr_in = (struct sockaddr_in *)&new_dest_addr;
		new_dest_addr_in->sin_port = new_port;

		return real_sendto(socket, buffer, length, flags,
				   (struct sockaddr *)&new_dest_addr, dest_len);
	}

	return real_sendto(socket, buffer, length, flags, dest_addr, dest_len);
}

ssize_t recvfrom(int socket, void *buffer, size_t length, int flags,
		 struct sockaddr *address, socklen_t *address_len)
{
       
	static struct sockaddr_in *address_in;
	address_in = (struct sockaddr_in *)address;

	if (address_in != NULL && address_in->sin_port == old_port) {
		address_in->sin_port = new_port;
	}

	ssize_t res;
	res = real_recvfrom(socket, buffer, length, flags, address,
			    address_len);

	if (address_in != NULL && address_in->sin_port == new_port) {
		address_in->sin_port = old_port;
	}
         printf("Pear recvfrom ... %d\r\n",res);
	return res;
}
