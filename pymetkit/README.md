# pymetkit

This repository contains an Python interface to the MetKit library for parsing MARS requests. 

## Example

The function for parsing a MARS request is `metkit.parse_mars_request` which accepts a string or file-like object 
as inputs. A list of `metkit.mars.Request` instances are returned, which is a dictionary containing the keys and 
values in the MARS request and the attribute `verb` for the verb in the MARS request.

### From String
```
from metkit import parse_mars_request

request_str = "retrieve,class=od,date=20240124,time=12,param=129,step=12,target=test.grib"
requests = parse_mars_request(requests)

print(requests[0])
# verb: retrieve, request: {'class': ['od'], 'date': ['20240124'], 'time': ['1200'], 'param': ['129'], 'step': ['12'], 'target': ['test.grib'], 'domain': ['g'], 'expver': ['0001'], 'levelist': ['1000', '850', '700', '500', '400', '300'], 'levtype': ['pl'], 'stream': ['oper'], 'type': ['an']}
```

### From File 
If the MARS request is contained inside a file, e.g. test_requests.txt:
```
from metkit import parse_mars_request 

requests = parse_mars_request(open("test_requests.txt", "r"))
```
