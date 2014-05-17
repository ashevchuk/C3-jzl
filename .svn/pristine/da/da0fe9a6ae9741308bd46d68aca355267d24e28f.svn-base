#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVNAME "/dev/cpuid"   /* MAJOR: 239, MINOR: 0 */

int main(int argc, char *argv[])
{
	int fd;
	unsigned int encoded_cpuid;

	fd = open(DEVNAME, O_RDWR);
	if (fd < 0)
	{
		printf("Error opening %s\n", DEVNAME);
		exit(-1);
	}

	read(fd, &encoded_cpuid, 4);
	printf("encoded_cpuid=0x%x\n", encoded_cpuid);

	close(fd);
	return 0;
}
