# yusim
Using Disksims with two sub-processes to implement hybrid storage system
- In this version, we designed a simulator with three stages for simulation but ignored 'simulate' stage. The two sim(hdd and ssd) receive 'one' requests and serve it. After that, the sim returns a empty request(no other info.) to user with response time. That is. Always, the sim only serves one request.
