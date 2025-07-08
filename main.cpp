#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define BUFFER_SIZE 1024

std::string dir;

void *handleClient(void *arg) {
  std::cout << "thread created" << std::endl;
  int new_socket = *((int *)arg);
  std::string buffer(BUFFER_SIZE, '\0');
  ssize_t req = read(new_socket, (void *)&buffer[0], BUFFER_SIZE);
  std::cout << buffer << std::endl;
  if (req < 0) {
    std::cerr << "error receiving message" << std::endl;
    close(new_socket);
    return nullptr;
  }
  if (buffer.substr(5, 4) == "echo") {
    std::string echo(BUFFER_SIZE, '\0');
    std::string compression_header = "";
    std::string body = "";
    std::string response;
    int counter = 0;
    int count = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      if (buffer[i] == '\0') {
        break;
      }
      count++;
      std::string com = buffer.substr(i, 5);
      if (com == "echo/") {
        for (int j = i + 5; j < BUFFER_SIZE; j++) {
          std::string comp = buffer.substr(j, 9);
          if (comp == " HTTP/1.1") {
            break;
          }
          body = body + buffer[j];
        }
      }
      std::string comp = buffer.substr(i, 15);
      if (comp == "Accept-Encoding") {
        for (int j = i + 17; j < BUFFER_SIZE; j++) {
          if (buffer[j] == '\r') {
            break;
          }
          compression_header = compression_header + buffer[j];
          counter++;
        }
      }
    }
    std::cout << "echo body is: " << body << std::endl;
    std::cout << "compression header: " << compression_header << std::endl;
    std::string content_len = std::to_string(body.length());
    if (counter) {
      if (compression_header == "invalid-encoding") {
        response =
            "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
            content_len + "\r\n\r\n" + body;
      } else {
        response = "HTTP/1.1 200 OK\r\nContent-Type: "
                   "text/plain\r\nContent-Encoding: gzip \r\nContent-Length: " +
                   content_len + "\r\n\r\n" + body;
      }
      // following the given compression schemes and giving response
    } else {
      response =
          "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
          content_len + "\r\n\r\n" + body;
    }
    // sending the response
    int bytes_sent =
        send(new_socket, (void *)&response[0], response.length(), 0);
    return nullptr;
  }
  if (buffer.substr(5, 10) == "user-agent") {
    std::string user_ag(BUFFER_SIZE, '\0');
    int count = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      if (buffer[i] == '\0') {
        break;
      }
      count++;
    }
    int user_ag_len = 0;
    for (int i = 60; i < count - 4; i++) {
      user_ag[i - 60] = buffer[i];
      user_ag_len++;
    }
    std::string user_ag_len_str = std::to_string(user_ag_len);
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: "
                           "text/plain\r\nContent-Length: \r\n\r\n";
    response.insert(response.begin() + 59, user_ag_len_str.begin(),
                    user_ag_len_str.end());
    response.append(user_ag);
    send(new_socket, response.c_str(), response.length(), 0);
    return nullptr;
  } else if (buffer.substr(5, 5) == "files") {
    std::string filename = "";
    std::string response;
    for (int i = 11; i < BUFFER_SIZE; i++) {
      if (buffer[i] == ' ') {
        break;
      }
      filename = filename + buffer[i];
    }
    std::ifstream file(dir + filename);
    std::string content;
    std::vector<std::string> total_content;
    if (file.good()) {
      while (getline(file, content)) {
        total_content.push_back(content);
      }
      int lines = total_content.size();
      std::string con = "";
      for (int i = 0; i < lines; i++) {
        con = con + total_content[i];
      }
      response = "HTTP/1.1 200 OK\r\nContent-Type: "
                 "application/octet-stream\r\nContent-Length: " +
                 std::to_string(con.length()) + "\r\n\r\n" + con;
    } else {
      response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }
    int bytes_sent =
        send(new_socket, (void *)&response[0], response.length(), 0);
    return nullptr;
  } else if (buffer.substr(0, 12) == "POST /files/") {
    int count = 0;
    std::string fileName;
    std::string content_len = "";
    int content_len_int;
    std::string content;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      if (buffer[i] == '\0') {
        break;
      }
      std::string comp = buffer.substr(i, 5);
      if (comp == "HTTP/") {
        fileName = buffer.substr(12, i - 12 - 1);
      }
      std::string comp2 = buffer.substr(i, 14);
      if (comp2 == "Content-Length") {
        int counter = 0;
        while (buffer[i + 16 + counter] != '\r') {
          content_len = content_len + buffer[i + 16 + counter];
          counter++;
        }
      }
      count++;
    }
    content_len_int = std::stoi(content_len);
    content = buffer.substr(count - content_len_int, content_len_int);
    std::ofstream file;
    file.open(dir + fileName);
    file << content;
    file.close();
    std::string response = "HTTP/1.1 201 Created\r\n\r\n";
    int bytes_sent =
        send(new_socket, (void *)&response[0], response.length(), 0);
    return nullptr;
  }
  std::string response = "HTTP/1.1 200 OK\r\n\r\n";
  response = (buffer.starts_with("GET / HTTP/1.1\r\n"))
                 ? response
                 : "HTTP/1.1 404 Not Found\r\n\r\n";
  int bytes_sent = send(new_socket, (void *)&response[0], response.length(), 0);
  return nullptr;
}

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  if (argc == 3 && strcmp(argv[1], "--directory") == 0) {
    dir = argv[2];
  }
  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.

  std::cout << "Logs from your program will appear here!\n";

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;

  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  while (1) {
    std::cout << "Waiting for a client to connect...\n";

    int client_addr_len = sizeof(client_addr);
    int *new_socket = new int;
    *new_socket = accept(server_fd, (struct sockaddr *)&client_addr,
                         (socklen_t *)&client_addr_len); // client_socket
    std::cout << "Client connected\n";
    pthread_t thread1;
    pthread_create(&thread1, NULL, &handleClient, (void *)new_socket);
    pthread_detach(thread1);
  }
  // Send a 200 response after get request
  close(server_fd);
  return EXIT_SUCCESS;
}
