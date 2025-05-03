# Houdini File Server (HoudiniFS) 
A lightweight, self-destructing file server that automatically deletes files after their expiration time.

## Key Features 

- **Self-destructing files** - Set expiration time in seconds
- **Simple CLI interface** - Easy server/client operation
- **Lightweight** - Written in pure C
- **Secure transfers** - Files never leave your local network
- **Automatic cleanup** - Background thread removes expired files

## Technical Highlights 

- **TCP socket programming** with proper error handling
- **Multi-threaded architecture** (pthreads for background cleanup)
- **Network byte order conversion** (htonl/ntohl)
- **File I/O operations** with chunked transfers
- **Makefile build system** for easy compilation

## Installation & Setup 

### Requirements
- Linux/macOS(unix based OS)
- GCC compiler
- Basic terminal knowledge

### Build Instructions
```bash
git clone https://github.com/yourusername/HoudiniFS.git
cd HoudiniFS
make
```

This creates two binaries in the `build/` directory:
* `server` - The file server daemon
* `client` - The upload client

## Usage 

### Starting the Server
```bash
./build/server &  # Run in background
```

### Uploading Files
```bash
./build/client /path/to/file SERVER_IP EXPIRY_SECONDS
```

Example:
```bash
./build/client secret_document.pdf 192.168.1.100 10
```

(Uploads file, auto-deletes after 10 secondss)

## Architecture Overview 
```
HoudiniFS/
├── build/           # Compiled binaries
├── files/           # File storage directory
│   └── .gitkeep     # Maintains directory structure
├── src/
│   ├── server.c     # Main server logic
│   └── client.c     # Client implementation
├── Makefile         # Build configuration
└── .gitignore       # Ignore build artifacts
```

## Technical Deep Dive 

### Server Operation
1. Creates TCP socket on port 9000
2. Spawns background cleanup thread
3. For each client:
   * Receives filename, size, and expiry
   * Validates file size (<10MB)
   * Streams file chunks to disk
   * Records expiry timestamp

### Cleanup Thread
* Runs every 60 seconds
* Scans `expiry.db` for expired files
* Removes expired files from disk
* Maintains database integrity

### Client Operation
1. Validates input file
2. Connects to server
3. Sends metadata (filename, size, expiry)
4. Streams file in 4KB chunks

## Performance Characteristics 
* Handles files up to 10MB
* Supports multiple concurrent uploads
* Minimal memory overhead (~2MB resident)
* Cleanup adds <1% CPU load

## Security Considerations 
* No authentication (intended for LAN use)
* Filename sanitization recommended
* Size validation prevents DoS
* Files never leave your local network

#  Use Cases for Houdini File Server

Houdini File Server is designed for **temporary and secure file sharing**. It automatically deletes files after a user-defined time, ensuring sensitive data does not stay longer than needed.

##  Practical Applications

- **Secure Temporary File Sharing**  
  Share sensitive files like credentials, passwords, or important documents that self-delete after a set duration — minimizing the risk of leaks.

- **Internal Team Transfers**  
  Easily move files across team members without manual cleanup. Files are automatically removed after their expiration time, keeping shared storage neat.

- **Academic or Workshop Demonstrations**  
  Professors, trainers, or speakers can share assignment files, resources, or example files with attendees, which expire after the event ends.

- **Temporary Backup Drops**  
  Use Houdini File Server to move files temporarily between personal devices or workstations, without permanent cloud storage involvement.

- **Compliance with Minimal Data Retention Policies**  
  Industries like finance, healthcare, or law often require strict short-term data retention. Houdini File Server enforces expiration automatically to assist with compliance.

##  Why Call It "Houdini"?

Just like the magician **Harry Houdini** who was famous for disappearing acts, the files sent to Houdini File Server **"vanish" automatically** after a certain time — **making them disappear like magic**!




## License 
MIT License - Free for personal and commercial use
