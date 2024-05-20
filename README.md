# Webserv

A simple web server written in C++98.

## Features

- YAML configuration (with custom parser)
- Module-like system for handling routes
- Support for multiple server binding (like nginx)
- HTTP/1.1 compliant
- Support for HEAD, GET, DELETE, POST, PUT methods
- CGI scripts
- File uploads
- Static file serving (with directory indexing and listing)
- HTTP Redirects

### YAML Parser

The YAML parser is a custom implementation that supports the following features:
- Scalars, Sequences, Maps and Flows
- Comments
- Error handling (although not very informative)
- Nested structures

## Build

```bash
make
```

## Usage

```bash
./webserv [config_file]
```

## Configuration

The server is configured using a YAML file.
Follows a nginx-like structure.
The following is an example configuration (and the default loaded when no arg is provided):

```yaml
servers:
  - listen: 8080
    server_names: localhost
    default: true
    routes:
      - uri: /
        settings:
          methods: [GET, HEAD]
        modules:
          - type: static
            settings:
              root: ~
              directory_listing: true
```
- `listen`: Port to listen on, can be a single port or a list of ports.
- `server_names`: List of server names that the server will respond to.
- `default`: If true, this server will be the default one (requests sent to a interface with no server conf match will be sent to this one instead).
- `routes`: List of routes.
 - `uri`: URI to match.
  - `settings`: Route settings.
    - `methods`: List of allowed methods. If empty (or not set), all methods are allowed.
  - `modules`: List of modules to be executed for this route. The order is important.
    - `type`: Module type.
    - `settings`: Module settings.

You can find more examples in the `bin/showcase` directory.
There are also general settings available in the [system](bin/.sys/) directory.
