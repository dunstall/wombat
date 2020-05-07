# System Integration & Performance Testing

## Running
First start the server:
```
docker build . -t wombatbroker -f build/Dockerfile
docker run -it wombatbroker
```

Then run system tests:
```
docker build . -t wombattests -f build/Dockerfile.test
docker run -it wombattests
```
