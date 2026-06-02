# Use an official lightweight GCC/C++ image
FROM gcc:latest

# Install SQLite development headers inside the container
RUN apt-get update && apt-get install -y libsqlite3-dev cmake

# Set the working directory inside the container
WORKDIR /app

# Copy the backend source files into the container
COPY . .

# Create a build directory, run CMake, and compile the binary
RUN mkdir -p build && cd build && cmake .. && make

# Expose the port your C++ server listens on (e.g., 8080)
EXPOSE 8080

# Command to run your compiled binary
CMD ["./build/log_analyzer"]