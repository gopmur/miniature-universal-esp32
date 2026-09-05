// #include "services/telnet.hpp"
// #include <cstring>
// #include "cc.h"
// #include "esp_err.h"
// #include "lwip/inet.h"
// #include "lwip/sockets.h"
// #include "service.hpp"

// TelnetService::TelnetService(int priority, const char* username, const char* password)
//     : Service(priority, "telnet") {
//   this->username = static_cast<char*>(malloc(strlen(username) + 1));
//   this->password = static_cast<char*>(malloc(strlen(password) + 1));
//   strcpy(this->password, password);
//   strcpy(this->username, username);
// }

// void TelnetService::main() {
//   int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//   struct sockaddr_in sock_address = {};
//   sock_address.sin_addr.s_addr = htonl(INADDR_ANY);
//   sock_address.sin_family = AF_INET;
//   sock_address.sin_port = htons(23);
//   ESP_ERROR_CHECK(bind(server_socket, (struct sockaddr*)&sock_address, sizeof(sock_address)));
//   listen(server_socket, 1);
//   char rx_buffer[256];
//   while (true) {
//     struct sockaddr_in client_addr;
//     socklen_t addr_len = sizeof(client_addr);
//     int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
//     if (client_socket < 0) {
//       continue;
//     }
//     auto username_prompt = "username: ";
//     auto password_prompt = "password: ";
//     auto invalid_credentials_message = "\ninvalid credentials";
//     auto prompt = "ESP32> ";
//     send(client_socket, username_prompt, strlen(username_prompt), 0);
//     int len = recv(client_socket, rx_buffer, sizeof(rx_buffer) - 1, 0);
//     rx_buffer[len] = 0;

//     bool username_is_correct = strcmp(rx_buffer, this->username) == 0;
//     send(client_socket, password_prompt, strlen(password_prompt), 0);
//     len = recv(client_socket, rx_buffer, sizeof(rx_buffer) - 1, 0);
//     rx_buffer[len] = 0;

//     bool password_is_correct = strcmp(rx_buffer, this->password) == 0;

//     if (!username_is_correct || !password_is_correct) {
//       send(client_socket, invalid_credentials_message, strlen(invalid_credentials_message), 0);
//       closesocket(client_socket);
//       continue;

//     }

//     send(client_socket, prompt, strlen(prompt), 0);

//     closesocket(client_socket);
//     continue;
//   }
// }