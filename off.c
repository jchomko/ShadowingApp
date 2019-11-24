
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {

        printf("Opening...\n");

        int fd = open("/dev/ttyUSB0", O_RDWR);

        if(fd < 0) {
                printf("Failed to open port!\n");
                exit(0);
        }

        printf("off\n");
        int state = ~TIOCM_DTR;

        ioctl(fd, TIOCMSET, &state);

        sleep(10);

        close(fd);

}

