#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// làm cho tiến trình cha đợi tiến trình con kết thúc rồi mới chạy tiếp!
int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed!\n");
        return 1;
    } else if (pid == 0) {
        printf("Hello from child process, pid = %d\n", getpid());
        return 0;
    } else {
        wait(NULL); // Doi tien trinh con ket thuc??
        printf("Hello from parent process, parent pid : %d, child pid: %d\n", getpid(), pid);
        return 0;
    }
}

int main__() {
    // fork() : tạo tiến trình con mới từ tiến trình cha hiện tại
    // fork() trong tiến trình cha trả về pid của tiến trình con
    // fork() trong tiến trình con trả về 0
    // pid < 0 -> fork() thất bại

    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed!\n");
        return 1;
    } else if (pid == 0) {
        printf("Hello from child process, pid = %d\n", getpid());
        return 0;
    } else {
        printf("Hello from parent process, parent pid : %d, child pid: %d\n", getpid(), pid);
        return 0;
    }
}

int main_() {
    // use system() to exec bash command
    /* system("pwd");

    chdir("/"); // equal system("cd /");

    system("pwd");
 */
    // execl(): execute and leave
    //* execl("/bin/ls","ls", "-l",0);
    // Cant reach here because execl terminated the prog
    //* printf("\n\n\n\n\n");
    //* system("ls -l");


    return 0;
}