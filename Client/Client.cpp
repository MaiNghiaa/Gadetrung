// Đây là chương trình minh họa tương tác client-server sử dụng socket cho ứng dụng dựa trên menu.
// Nó bao gồm các hàm để in menu, đọc dữ liệu từ file, và xử lý đầu vào từ người dùng để xác thực và lựa chọn menu.

// Bao gồm các thư viện tiêu chuẩn để nhập/xuất và xử lý socket
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080        // Định nghĩa số cổng sử dụng
#define MAX_DATA_SIZE 1024 // Định nghĩa kích thước tối đa cho dữ liệu trong buffer

// Hàm in menu dựa trên cấp độ menu
void print_menu(int menu_level) {
    switch (menu_level) {
        case 1:
            printf("\n===== Main Menu =====\n");
            printf("1. Hiển thị thông tin đăng nhập\n");
            printf("2. Đổi mật khẩu\n");
            printf("3. Option 3\n");
            printf("4. Option 4\n");
            printf("5. Thoát\n");
            break;
        case 2:
            printf("\n===== Option 3 Menu =====\n");
            printf("1. Thiết lập thông số\n");
            printf("2. Bắt đầu\n");
            printf("3. Thoát\n");
            break;
        default:
            break;
    }
}

// Hàm đọc dữ liệu từ file vào buffer
void read_data_from_file(const char *filename, char *buffer, size_t buffer_size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Lỗi mở file");
        return;
    }

    // Đọc dữ liệu từ file vào buffer
    size_t total_read = 0;
    char line[MAX_DATA_SIZE];
    while (fgets(line, sizeof(line), file)) {
        // Nối dòng vào buffer (kiểm tra tràn buffer)
        size_t line_len = strlen(line);
        if (total_read + line_len < buffer_size) {
            strcat(buffer, line);
            total_read += line_len;
        } else {
            printf("Vượt quá kích thước buffer, dữ liệu có thể bị cắt.\n");
            break;
        }
    }

    fclose(file);
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char username[100], password[100], new_password[100], buffer[2048] = {0};
    int menu_choice;
    int submenu_choice;


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Lỗi tạo socket \n"); // In thông báo lỗi nếu không tạo được socket
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Chuyển đổi địa chỉ IPv4 và IPv6 từ dạng văn bản sang dạng nhị phân
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n Địa chỉ không hợp lệ/ Không được hỗ trợ \n"); // In thông báo lỗi nếu địa chỉ không hợp lệ
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Kết nối thất bại \n"); // In thông báo lỗi nếu kết nối không thành công
        return -1;
    }

    printf("Nhập tên đăng nhập: ");
    scanf("%s", username);
    printf("Nhập mật khẩu: ");
    scanf("%s", password);

    send(sock, username, strlen(username), 0);
    send(sock, password, strlen(password), 0);

    // Nhận kết quả xác thực
    read(sock, buffer, 1024);
    if (strcmp(buffer, "1") == 0) {
        printf("Đăng nhập thành công.\n");

        while (1) {
            // Nhận và in menu từ server
            read(sock, buffer, 1024);
            printf("%s", buffer);

            // Gửi lựa chọn menu cho server
            printf("Nhập lựa chọn của bạn (1-5): ");
            scanf("%d", &menu_choice);
            send(sock, &menu_choice, sizeof(int), 0);

            if (menu_choice == 2) {
                // Option 2: Đổi mật khẩu
                printf("Nhập mật khẩu mới: ");
                scanf("%s", new_password);
                 // Gửi mật khẩu mới cho server
                send(sock, new_password, strlen(new_password), 0);
                send(sock, "\n", 1, 0); // Gửi ký tự xuống dòng để phân tách
                // Nhận kết quả từ server
                read(sock, buffer, 1024);
                if (strcmp(buffer, "1") == 0) {
                    printf("Đổi mật khẩu thành công.\n");
                } else {
                    printf("Không thể đổi mật khẩu.\n");
                }
            } else if (menu_choice == 3) {
                // Option 3: Submenu
                while (1) {
                    print_menu(2); // In submenu cho Option 3

                    // Gửi lựa chọn submenu cho server
                    printf("Nhập lựa chọn của bạn (1-3): ");
                    scanf("%d", &submenu_choice);
                    send(sock, &submenu_choice, sizeof(int), 0);

                    if (submenu_choice == 1) {
                        printf("Client chọn Option 1 trong submenu của Option 3.\n");
                        // Xử lý Thiết lập thông số

                        int feeding_time;
                        float food_amount, Tmin, T, Tmax;

                        // Nhập thời gian cho ăn
                        printf("Nhập thời gian cho ăn (giờ từ nửa đêm): ");
                        scanf("%d", &feeding_time);

                        // Nhập lượng thức ăn
                        printf("Nhập lượng thức ăn (kg mỗi lần cho ăn): ");
                        scanf("%f", &food_amount);

                        // Nhập Tmin, T, Tmax
                        printf("Nhập Tmin, T, Tmax (đơn vị Celsius):\n");
                        printf("Tmin: ");
                        scanf("%f", &Tmin);
                        printf("T: ");
                        scanf("%f", &T);
                        printf("Tmax: ");
                        scanf("%f", &Tmax);

                        // Gửi các tham số này đến server
                        send(sock, &feeding_time, sizeof(int), 0);
                        send(sock, &food_amount, sizeof(float), 0);
                        send(sock, &Tmin, sizeof(float), 0);
                        send(sock, &T, sizeof(float), 0);
                        send(sock, &Tmax, sizeof(float), 0);

                        read(sock, buffer, 1024);
                        printf("%s\n", buffer);

                    } else if (submenu_choice == 2) {
                        printf("Bắt đầu...\n");

                    } else if (submenu_choice == 3) {
                        printf("Thoát submenu...\n");
                        break;
                    } else {
                        printf("Lựa chọn không hợp lệ trong submenu.\n");
                    }
                }
            } else if (menu_choice == 5) {
                printf("Thoát...\n");
                break;
            }
        }
    } else {
        printf("Đăng nhập thất bại.\n");
    }

    // Đóng kết nối
    // close(sock);
    return 0;
} //Client.cpp
