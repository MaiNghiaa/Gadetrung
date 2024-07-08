#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define MAX_DATA_SIZE 1024

// Hàm xác thực người dùng
int authenticate_user(char *username, char *password) {
    FILE *fp;
    char line[100], uname[50], pass[50];

    fp = fopen("accounts.txt", "r");
    if (fp == NULL) {
        perror("Lỗi mở tệp");
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%s %s", uname, pass);
        if (strcmp(uname, username) == 0 && strcmp(pass, password) == 0) {
            fclose(fp);
            return 1; // Xác thực thành công
        }
    }

    fclose(fp);
    return 0; // Xác thực thất bại
}


// Hàm cập nhật mật khẩu
int update_password(char *username, char *new_password) {
    FILE *fp, *temp_fp;
    char line[100], uname[50], pass[50];
    int updated = 0;

    fp = fopen("accounts.txt", "r");
    temp_fp = fopen("temp.txt", "w");

    if (fp == NULL || temp_fp == NULL) {
        perror("Lỗi mở tệp");
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%s %s", uname, pass);
        if (strcmp(uname, username) == 0) {
            fprintf(temp_fp, "%s %s\n", uname, new_password);
            updated = 1;
        } else {
            fprintf(temp_fp, "%s %s\n", uname, pass);
        }
    }

    fclose(fp);
    fclose(temp_fp);

    // Xóa tệp gốc và đổi tên tệp tạm thành tệp gốc
    remove("accounts.txt");
    rename("temp.txt", "accounts.txt");

    return updated; // Trả về 1 nếu cập nhật mật khẩu thành công, 0 nếu không thành công
}

// Hàm gửi menu cho client
void send_menu(int client_socket, int menu_level) {
    char menu[1024];
    switch (menu_level) {
        case 1: // Menu chính
            snprintf(menu, sizeof(menu), "\n===== Menu =====\n");
            strcat(menu, "1. Hiển thị thông tin đăng nhập\n");
            strcat(menu, "2. Đổi mật khẩu\n");
            strcat(menu, "3. Option 3\n");
            strcat(menu, "4. Option 4\n");
            strcat(menu, "5. Thoát\n");
            break;
        case 2: // Menu con cho Option 3
            snprintf(menu, sizeof(menu), "\n===== Option 3 Menu =====\n");
            strcat(menu, "1. Thiết lập thông số\n");
            strcat(menu, "2. Bắt đầu\n");
            strcat(menu, "3. Thoát\n");
            break;
        default:
            break;
    }

    send(client_socket, menu, strlen(menu), 0);
}

// Hàm xử lý yêu cầu từ client
void handle_client(int client_socket, char *username, char *password) {
    char buffer[1024] = {0};
    int menu_choice;
    int submenu_choice;

    // Gửi menu cho client
    send_menu(client_socket, 1);

    while (1) {
        // Nhận lựa chọn menu từ client
        recv(client_socket, &menu_choice, sizeof(int), 0);

        // Xử lý lựa chọn menu
        switch (menu_choice) {
            case 1: {
                printf("1.\n");
                char response_message[256];
                snprintf(response_message, sizeof(response_message), "Option 1 selected.\nUsername: %s\nPassword: %s", username, password);
                send(client_socket, response_message, strlen(response_message), 0);
                break;
            }
            case 2: {
                printf("2.\n");
                // Yêu cầu mật khẩu mới từ client
                char new_password[50];
                recv(client_socket, buffer, 1024, 0);
                sscanf(buffer, "%s", new_password);
                memset(buffer, 0, sizeof(buffer));

                // Cập nhật mật khẩu trong tệp accounts.txt
                if (update_password(username, new_password)) {
                    printf("Cập nhật mật khẩu thành công cho người dùng: %s\n", username);
                    send(client_socket, "1", 1, 0); // Thông báo thành công cho client
                } else {
                    printf("Không thể cập nhật mật khẩu cho người dùng: %s\n", username);
                    send(client_socket, "0", 1, 0); // Thông báo thất bại cho client
                }
                break;
            }
            case 3: {
                printf("Client chọn Option 3.\n");
                // Xử lý Option 3 - Menu con
                send_menu(client_socket, 2); // Gửi menu con cho Option 3
                while (1) {
                    recv(client_socket, &submenu_choice, sizeof(int), 0);

                    switch (submenu_choice) {
                        case 1: {
                            printf("Client chọn Option 1 trong submenu Option 3.\n");
                            // Xử lý Thiết lập thông số
                            int thoi_diem;
                            float W, Tmin, Tmax, T;

                            // Nhận tham số từ client
                            recv(client_socket, &thoi_diem, sizeof(int), 0);
                            recv(client_socket, &W, sizeof(float), 0);
                            recv(client_socket, &Tmin, sizeof(float), 0);
                            recv(client_socket, &Tmax, sizeof(float), 0);
                            recv(client_socket, &T, sizeof(float), 0);

                            // Mở tệp để ghi các tham số
                            FILE *fp;
                            fp = fopen("thietlapthongso.txt", "a");
                            if (fp == NULL) {
                                perror("Lỗi mở tệp");
                                send(client_socket, "0", strlen("0"), 0); // Thông báo thất bại cho client
                                break;
                            }

                            // Ghi các tham số vào tệp
                            fprintf(fp, "Thời điểm: %d, W: %.2f, Tmin: %.2f, Tmax: %.2f, T: %.2f\n", thoi_diem, W, Tmin, Tmax, T);
                            fclose(fp);
                            char verification_message[1024];
                            snprintf(verification_message, sizeof(verification_message), "Lưu thông số thành công.\nThời điểm: %d\nW: %.2f\nTmin: %.2f\nTmax: %.2f\nT: %.2f\n", thoi_diem, W, Tmin, Tmax, T);
                            send(client_socket, verification_message, strlen(verification_message), 0); // Thông báo thành công và gửi lại dữ liệu cho client
                            break;
                        }
                        case 2: {
                            printf("Client chọn Option 2 trong submenu Option 3.\n");

                            // Mở tệp thietlapthongso.txt để đọc dòng đầu tiên
                            FILE *fp1 = fopen("thietlapthongso.txt", "r");
                            if (fp1 == NULL) {
                                perror("Lỗi mở thietlapthongso.txt");
                                send(client_socket, "0", strlen("0"), 0); // Thông báo thất bại cho client
                                break;
                            }

                            // Đọc dòng đầu tiên từ thietlapthongso.txt
                            char first_line[256];
                            fgets(first_line, sizeof(first_line), fp1);
                            fclose(fp1);

                            // Phân tích thoi_diem, W, Tmin, Tmax, T từ dòng đầu tiên
                            int thoi_diem;
                            float W, Tmin, Tmax, T;
                            sscanf(first_line, "Thời điểm: %d, W: %f, Tmin: %f, Tmax: %f, T: %f", &thoi_diem, &W, &Tmin, &Tmax, &T);

                            // Gửi các giá trị đã phân tích cho client
                            char response_message[1024];
                            snprintf(response_message, sizeof(response_message), "Dữ liệu từ thietlapthongso.txt:\nThời điểm: %d\nW: %.2f\nTmin: %.2f\nTmax: %.2f\nT: %.2f\n", thoi_diem, W, Tmin, Tmax, T);
                            send(client_socket, response_message, strlen(response_message), 0);
                            break;
                        }
                        case 3: {
                            printf("Client chọn Option 3 trong submenu Option 3.\n");
                            // Thoát submenu Option 3
                            send_menu(client_socket, 1); // Gửi lại menu chính cho client
                            break;
                        }
                        default:
                            break;
                    }
                    if (submenu_choice == 3) {
                        break;
                    }
                }
                break;
            }
            case 4:
                printf("Client chọn Option 4.\n");
                // Xử lý Option 4
                send(client_socket, "Option 4 selected", strlen("Option 4 selected"), 0);
                break;
            case 5:
                printf("Client chọn Option 5 (Thoát).\n");
                // Thoát chương trình
                printf("Client đã thoát.\n");
                close(client_socket);
                return;
            default:
                break;
        }
    }
}

// Hàm chính
int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    // Tạo socket server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Lỗi tạo socket");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Liên kết địa chỉ server với socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Lỗi liên kết địa chỉ");
        exit(EXIT_FAILURE);
    }

    // Chờ kết nối từ client
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Lỗi chờ kết nối");
        exit(EXIT_FAILURE);
    }

    printf("Server đang chờ kết nối từ client trên cổng %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        // Chấp nhận kết nối từ client
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Lỗi chấp nhận kết nối");
            exit(EXIT_FAILURE);
        }

        printf("Kết nối mới từ %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Xử lý yêu cầu từ client
        char username[50], password[50];
        recv(client_socket, username, sizeof(username), 0);
        recv(client_socket, password, sizeof(password), 0);

        if (authenticate_user(username, password)) {
            printf("Xác thực thành công cho người dùng: %s\n", username);
            send(client_socket, "1", 1, 0); // Thông báo xác thực thành công cho client

            // Xử lý yêu cầu từ client
            handle_client(client_socket, username, password);
        } else {
            printf("Xác thực thất bại cho người dùng: %s\n", username);
            send(client_socket, "0", 1, 0); // Thông báo xác thực thất bại cho client
        }
    }

    close(server_socket);
    return 0;
}



// nhiệm vụ còn lại là khi ấn start. Máy tính sẽ đọc dữ liệu từ 1 file .... và gửi tới server,.. Server nhận được sẽ gửi lại thông báo. nếu ấn vào số 3 thì có thể thoát tắt chương trình và thoát khỏi submenu, chuyển về main menu