#include "open_file.h"

using namespace std;

#define READ_BUF_SIZE 10

int main()
{
    ngxplus::OpenFile file;
    string buf;
    u_char read_buf[READ_BUF_SIZE] = {0};
    ssize_t n;
    long offset;

    // ngx_log_stderr >> logs/test_open_file
    // freopen("logs/test_open_file", "w", stderr);

    if (!file.init("logs/test_open_file", O_APPEND|O_RDWR, NGX_FILE_CREATE_OR_OPEN, NGX_FILE_DEFAULT_ACCESS)) {
        return -1;
    }

    // after write, the offset point to the end
    while (cin >> buf) {
        buf.append("\n");
        if (file.write((u_char*)buf.c_str(), buf.size()) == -1) {
            break;
        }
        if (!file.ftell(&offset)) {
            return -1;
        }
        printf("offset is %ld after cin\n", offset);
    }

    if (!file.fseek(-READ_BUF_SIZE, SEEK_END)) {
        return -1;
    }
    if (!file.ftell(&offset)) {
        return -1;
    }
    printf("offset is %ld after cin\n", offset);

    if ((n = file.read(read_buf, sizeof(read_buf) - 1)) > 0) {
        printf("read %ld bytes: \"%s\" from %s\n", n, (char*)read_buf, file.get_name().c_str());
    }

    file.close();
    return 0;
}
