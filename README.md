
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

## Run The Project 

#### To open the project:

Clone the project

```bash
  git clone https://github.com/YoadTamar/cnc_assignment4.git
```
#### To run the project:

Open terminal where you save the project files.

#### build the project:
```bash
make all
```
#### To run the first part:
```bash
sudo ./PartA <ip>
```
#### To run the second part:
```bash
sudo ./PartB <ip>
```
#### To clean the files
```bash
make clean
```
## prints examples:

#### part A example:
![partA_prints](https://user-images.githubusercontent.com/119599940/210381863-273b331e-ed84-4287-a90d-771ad68fc9a2.png)

#### part B example:
![partB_prints](https://user-images.githubusercontent.com/119599940/210381982-456d747e-f566-4a3b-9a3d-57fdc65de14b.png)



## wireshark examples


#### For example - ping:

![ping_pic](https://user-images.githubusercontent.com/119599940/210381265-9a3c9d51-830a-4d25-a948-2111cb9cc27a.png)


#### For example - tcp:

![pic_tcp](https://user-images.githubusercontent.com/119599940/210381373-dadc8221-931d-4c2a-a6a7-47b918e61bef.png)

## 🛠 Skills
C Programing 

