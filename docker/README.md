### Docker

Directories containing each Dockerfile for individual, stand-alone demos.

The Dockerfile(s) need to be built from parent directory due to relative path includes:  

```
docker build -t <name> -f <docker/demo1/Dockerfile> .
```
