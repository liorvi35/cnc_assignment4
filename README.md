
# Communication and Computing Course Assigment 4


## Authors

- [@Yoad Tamar](https://github.com/YoadTamar)
- [@Lior Vinman](https://github.com/liorvi35)

## General Description
This is assignment 4 in "Communication and Computing" Course course at Ariel University.
We were asked to implement a ping program.
- in the first part we were ask to implement a simple ping: <br>
  The user will run the program (ping.c) with the ip. The program will send an ICMP ECHO REQUEST to the host, and when receiving ICMP-ECHOREPLY, the program will send the next ICMP ECHO REQUEST (no need to stop). 
  This code can't handle timeout, and it will just wait till it gets back reply.
- in the second part we were ask to implement a "better" ping:
   The user will run the program (better_ping.c) with the ip. It will do the same thing as before, but now we will handle the timeout.
   The better ping will run a second program that hold a timer (TCP connection on port 3000) to ensure that if we don’t receive an ICMP-ECHO-REPLY after sending an ICMP-REQUEST for 10 seconds, It will exit and print <br> “server < ip > cannot be reached.”


