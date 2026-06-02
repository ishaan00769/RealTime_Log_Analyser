# Use an official Node.js image
FROM node:20-alpine

# Set the working directory inside the container
WORKDIR /app

# Copy package files first to leverage Docker caching for node_modules
COPY package*.json ./

# Install the frontend dependencies inside the container
RUN npm install

# Copy the rest of your React files
COPY . .

# Expose the Vite development server port (usually 5173)
EXPOSE 5173

# Start the Vite development server, allowing external connections (--host)
CMD ["npm", "run", "dev", "--", "--host"]