#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>

std::queue<int> task_queue;
std::mutex queue_mutex;
std::condition_variable cv;
bool stop_pool = false;
const int NUM_THREADS = 10;

std::string parse_request(int client_fd) {
    char buffer[4096];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) return "";

    std::string request(buffer, bytes_read);
    size_t pos = request.find('\n');
    if (pos == std::string::npos) return "";
    std::string first_line = request.substr(0, pos);

    size_t method_end = first_line.find(' ', 0);
    if (method_end == std::string::npos || first_line.substr(0, 3) != "GET") {
        return "400 Bad Request";
    }
    size_t path_end = first_line.find(' ', method_end + 1);
    if (path_end == std::string::npos) return "400 Bad Request";
    
    std::string path = first_line.substr(method_end + 1, path_end - method_end - 1);
    if (path.empty() || path == "/") path = "/index.html";

    return path;
}

std::string get_file_content(const std::string& path) {
    // Basic security check - reject paths with ".." or absolute paths
    if (path.find("..") != std::string::npos || path[0] != '/') {
        return "";
    }
    
    std::string fs_path = "./www" + path;
    
    std::ifstream file(fs_path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void send_response(int client_fd, const std::string& content, const std::string& content_type = "text/html") {
    std::string status = content.empty() ? "404 Not Found" : "200 OK";
    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    if (!content.empty()) {
        response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    }
    response += "Connection: close\r\n\r\n";
    response += content;

    write(client_fd, response.c_str(), response.length());
}

void worker_thread() {
    while (true) {
        int client_fd;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [&] { return !task_queue.empty() || stop_pool; });
            if (stop_pool && task_queue.empty()) return;
            client_fd = task_queue.front();
            task_queue.pop();
        }

        // Parse HTTP request
        std::string path = parse_request(client_fd);
        std::string content_type = "text/html";  // Default

        if (path == "400 Bad Request") {
            send_response(client_fd, "<h1>400 Bad Request</h1>", "text/html");
        } else {
            // Determine content type from path extension
            if (path.find(".css") != std::string::npos) {
                content_type = "text/css";
            } else if (path.find(".png") != std::string::npos) {
                content_type = "image/png";
            } else if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) {
                content_type = "image/jpeg";
            } else if (path.find(".html") != std::string::npos) {
                content_type = "text/html";
            }

            std::string content = get_file_content(path);
            send_response(client_fd, content, content_type);
        }
        close(client_fd);
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) port = std::stoi(argv[1]);

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // Bind to address and port
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // Listen for connections
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    // Create worker threads
    std::vector<std::thread> workers;
    for (int i = 0; i < NUM_THREADS; ++i) {
        workers.emplace_back(worker_thread);
    }

    std::cout << "HTTP server listening on port " << port << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            task_queue.push(client_fd);
        }
        cv.notify_one();
    }

    // Cleanup
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop_pool = true;
    }
    cv.notify_all();
    for (auto& w : workers) w.join();
    close(server_fd);
    return 0;
}
