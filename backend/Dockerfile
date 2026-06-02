# Use an official lightweight GCC/C++ image
FROM gcc:latest

# Install SQLite development headers AND Python3 inside the container
RUN apt-get update && apt-get install -y libsqlite3-dev cmake python3

# Set the working directory inside the container
WORKDIR /app

# Copy the backend source files into the container
COPY . .

# Create a build directory, run CMake, and compile the binary
RUN mkdir -p build && cd build && cmake .. && make

# Expose the port your C++ server listens on
EXPOSE 8080

# Run the Python generator and pipe it directly into your C++ binary automatically
CMD ["sh", "-c", "python3 -u scripts/generate_traffic.py | ./build/log_analyzer"]