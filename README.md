# HTTP-Server
# Lab 4.1: HTTP Server - Operating Systems Course

This project builds a C++ HTTP server incrementally across 4 parts, progressing from a simple single-threaded echo server to a multi-threaded HTTP web server that serves static files.

## Compilation Instructions

### Part 1: Simple Echo Server
```bash
g++ -o server echo_server.cpp -std=c++17
g++ -o client echo_client.cpp -std=c++17
```

### Part 2: Multi-Threaded Echo Server
```bash
g++ -o server multi_threaded_server.cpp -lpthread -std=c++17
g++ -o client echo_client.cpp -std=c++17
```

### Part 3: Thread-Pool Echo Server
```bash
g++ -o server thread_pool_server.cpp -lpthread -std=c++17
g++ -o client echo_client.cpp -std=c++17
```

### Part 4: HTTP Web Server
```bash
g++ -o server http_server.cpp -lpthread -std=c++17
```

## Running and Testing

### Part 1: Simple Echo Server 

**Terminal 1 - Start Server**
```bash
./server 8080
```

**Terminal 2 - Test with Client**
```bash
# Single test
./client localhost 8080 "Hello, server!"
```

**Terminal 3 - Test Queuing**
```bash
# Run 3 clients simultaneously
./client localhost 8080 "Client-1" &
./client localhost 8080 "Client-2" &
./client localhost 8080 "Client-3" &
```

### Part 2: Multi-Threaded Echo Server

**Terminal 1 - Start Server**
```bash
./server 8080
```

**Terminal 2 - Test Concurrent Connections**
```bash
# Launch 10 clients simultaneously
for i in {1..10}; do
    ./client localhost 8080 "Message-$i" &
done
```

### Part 3: Thread-Pool Echo Server

**Terminal 1 - Start Server**
```bash
./server 8080
```

**Terminal 2 - Test with More Clients than Pool Size**
```bash
# Launch 20 clients
for i in {1..20}; do
    ./client localhost 8080 "Message-$i" &
done
```

**Terminal 2 - Performance Comparison**
```bash
# Measure time taken
time for i in {1..20}; do ./client localhost 8080 "Test-$i" & done;
```

### Part 4: HTTP Web Server

**Terminal 1 - Start Server**
```bash
./server 8080
```

**Terminal 2 - Test with curl**

```bash
# 1. Root path (serves index.html)
curl -v http://localhost:8080/

# 2. Explicit index.html
curl -v http://localhost:8080/index.html

# 3. CSS file
curl -v http://localhost:8080/style.css

# 4. Image file (check Content-Type: image/png)
curl -v http://localhost:8080/logo.png

# 5. Subpage
curl -v http://localhost:8080/subpage.html

# 6. Non-existent file (should return 404)
curl -v http://localhost:8080/missing.txt

# 7. Favicon (404 if not exists)
curl -v http://localhost:8080/favicon.ico

# 8. Security test - path with .. (should reject)
curl -v http://localhost:8080/..

# 9. Query parameter (ignore for now)
curl -v http://localhost:8080/?test=1

# 10. Long non-existent path (404)
curl -v http://localhost:8080/very/long/path/to/file.html
```

**Test with Web Browser**
```bash
# Open the port in your browser
```

**Test Concurrent HTTP Requests:**
```bash
# Launch 10 concurrent requests
for i in {1..10}; do
    curl http://localhost:8080/ > /dev/null 2>&1 &
done
wait
echo "All requests completed"
```
