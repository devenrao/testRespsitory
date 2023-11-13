#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include <cerrno>

int main()
{
	const char* scom_path = "/dev/scom1";
	int fd = open(scom_path, O_RDWR | O_SYNC);
	if (fd < 0) {
		printf("Unable to open %s \n", scom_path);
		return -1;
	}
	printf("getscom sucessfully opened fd %d \n", fd);

	lseek(fd, 0, SEEK_SET);
	printf("getscom seek success for fd %d \n", fd);

	uint64_t value = 0;
	uint32_t addr = 0x50001ull;
	//uint32_t addr = 0x000D0011;
	printf("getscom pread fd %d addr = 0x%x \n", fd, addr); 
	int rc = pread(fd, &value, 8, addr);
	if (rc < 0) {
		printf("getscom pread fd %d addr = 0x%x errno=%d errstr = %s\n", 
			fd, addr, errno, strerror(errno));
		rc = errno;
		return rc;
	}
	printf("getscom sucessfully read data = 0x%016" PRIx64 " \n", value);

   	return 0;
}
//dd if=/dev/scom311 bs=8 skip=40960 count=1 | hexdump
