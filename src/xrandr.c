#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <sys/wait.h>

static char output_name[64] = {0};

static void detect_output(void)
{
    FILE *fp = popen("xrandr | grep ' connected'", "r");
    if (!fp) return;

    if (fgets(output_name, sizeof(output_name), fp)) {
        char *space = strchr(output_name, ' ');
        if (space) *space = '\0';
    }
    pclose(fp);
}

static int
run_xrandr(const char *output,
           const char *gamma)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        execlp("xrandr",
               "xrandr",
               "--output",
               output,
               "--gamma",
               gamma,
               NULL);

        perror("execlp");
        _exit(127);
    }

    int status;

    waitpid(pid, &status, 0);

    return status;
}

void xrandr_set_temperature(int temp)
{
    setlocale(LC_NUMERIC, "C");

    if (output_name[0] == '\0') {
        detect_output();
    }

    if (output_name[0] == '\0') {
        printf("No output detected\n");
        return;
    }

    char gamma[64];
    float ratio = (temp - 1000.0f) / (6500.0f - 1000.0f);

    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    float r = 1.0f;
    float g = 0.5f + 0.5f * ratio;
    float b = 0.2f + 0.8f * ratio * ratio;

    snprintf(gamma,
             sizeof(gamma),
             "%.2f:%.2f:%.2f",
             r, g, b);

    run_xrandr(output_name, gamma);
}

void xrandr_reset(void)
{
    output_name[0] = '\0';     // <- force redetect here
    detect_output();

    if (output_name[0] == '\0') {
        fprintf(stderr, "xrandr_reset: No output detected\n");
        return;
    }

    run_xrandr(output_name, "1:1:1");
}
