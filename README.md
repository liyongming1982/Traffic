## peartraffic

Currently it rewrites only `sendto` and `recvfrom`.

## Usage

```
$ make
```

## Example
 
Server
$ LD_PRELOAD=./peartraffic.so  ncat -l -p 5000

Client
$ LD_PRELOAD=./peartraffic.so  ncat server 5000
