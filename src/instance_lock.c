#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>

static int lock_fd = -1;

int
lock_gui(void)
{
    char path[256];
    const char *home = getenv("HOME");

    if (!home)
        home = "/tmp";

    snprintf(path,
             sizeof(path),
             "%s/.config/nl-ease.lock",
             home);

    lock_fd = open(path, O_CREAT | O_RDWR, 0644);

    if (lock_fd < 0)
    {
        perror("open");
        return 0;
    }

    if (flock(lock_fd, LOCK_EX | LOCK_NB) != 0)
    {
        close(lock_fd);
        lock_fd = -1;

        fprintf(stderr, "Another nl-ease instance is already running.\n");
        return 0;
    }

    return 1;
}

void
unlock_gui(void)
{
    if (lock_fd >= 0)
    {
        flock(lock_fd, LOCK_UN);
        close(lock_fd);
        lock_fd = -1;
    }
}